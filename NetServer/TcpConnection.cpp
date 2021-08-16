/**
* @file     TcpConnection.cpp
* @brief    tcp客户端连接
* @author   lddddd (https://github.com/lddddd1997)
* @par      bug fixed:
*           2021.07.31，通过在handler内定义TcpConnectionSPtr prolong = shared_from_this()来延长本对象生命周期至handler调用结束
*           2021.08.13，通过在channel中增加连接的weak_ptr来解决绑定this指针可能造成的线程安全问题
*           2021.08.14，调用CloseHandler前设置不再监听该连接channel事件，否则有可能在调用析构函数时还会触发事件
*/
#include <unistd.h>
#include <iostream>
#include <assert.h>
#include "Socket.h"
#include "Utilities.h"
#include "TcpConnection.h"
#include "Logger.h"

TcpConnection::TcpConnection(EventLoop *loop, int fd,
                  const struct sockaddr_in& local_addr, const struct sockaddr_in& peer_addr) :
    disconnected_(true),
    loop_(loop),
    connection_channel_("client"),
    local_addr_(local_addr),
    peer_addr_(peer_addr)
{
    connection_channel_.SetFd(fd);
    connection_channel_.SetEvents(/*EPOLLRDHUP | */EPOLLIN/* | EPOLLET*/);
    connection_channel_.SetReadHandler(std::bind(&TcpConnection::ReadHandler, this)); // 构造函数中不能使用shared_from_this()
    connection_channel_.SetWriteHandler(std::bind(&TcpConnection::WriteHandler, this)); 
    connection_channel_.SetCloseHandler(std::bind(&TcpConnection::CloseHandler, this)); // 传this，存在线程安全（调用cleanup后，引用计数为0，回收本对象，导致Handler后面的程序无法运行）
    connection_channel_.SetErrorHandler(std::bind(&TcpConnection::ErrorHandler, this));
}

TcpConnection::~TcpConnection() // TcpConnection的shared_ptr对象引用计数为0
{
    // loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, loop_, &connection_channel_)); // 无需清除，close后epoll会自动删除，，man文档Q6
    
    // LOG_INFO << "Remove connection " << inet_ntoa(LocalAddress().sin_addr)
    //         << ":" << ntohs(LocalAddress().sin_port)
    //         << " from " << inet_ntoa(PeerAddress().sin_addr)
    //         << ":" << ntohs(PeerAddress().sin_port);
    close(connection_channel_.Fd()); // 关闭该连接
    assert(disconnected_);
}

void TcpConnection::ConnectEstablished() // basic_loop线程接收新连接后，初始化
{
    // connection_channel_.SetReadHandler(std::bind(&TcpConnection::ReadHandler, shared_from_this())); // fix不可取，bind绑定本对象至channel的function，会一直维持生命周期至channel释放，导致无法关闭连接
    // connection_channel_.SetWriteHandler(std::bind(&TcpConnection::WriteHandler, shared_from_this()));
    // connection_channel_.SetCloseHandler(std::bind(&TcpConnection::CloseHandler, shared_from_this()));
    // connection_channel_.SetErrorHandler(std::bind(&TcpConnection::ErrorHandler, shared_from_this()));
    // connection_channel_.SetCloseHandler(std::bind(&TcpConnection::CloseHandler, shared_from_this()));

    // https://blog.csdn.net/u011344601/article/details/51997886?utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-4.control&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-4.control
    // loop_->CommitChannelToEpoller(&connection_channel_); // basic_loop线程执行，可能导致loop_的epoller的线程安全问题（loop_线程epoll_wait时，basic_loop线程调用loop_的epoll_ctl）
    connection_channel_.Tie(shared_from_this()); // fix bug:由于channel回调函数绑定的是this指针,可能存在回调时连接已关闭的情况
    loop_->CommitTaskToLoop(std::bind(&EventLoop::CommitChannelToEpoller, loop_, &connection_channel_)); // 故加入任务队列，由loop_线程执行添加，此时loop_必定不会处于epoll_wait
    disconnected_ = false;
    if(commit_callback_)
    {
        commit_callback_(shared_from_this()); // 更新Server层的时间轮
    }
}

void TcpConnection::Send(const std::string& str)
{
    buffer_out_ += str;
    if(loop_->IsInLoopThread())
    {
        SendInLoop();
        return;
    }
    loop_->CommitTaskToLoop(std::bind(&TcpConnection::SendInLoop, shared_from_this())); // 投递到loop_线程，解决线程安全问题
}

void TcpConnection::SendInLoop()
{
    if(disconnected_)
    {
        return ;
    }
    int nwrite = Utilities::Writen(connection_channel_.Fd(), buffer_out_);
    if(nwrite > 0)
    {
        uint32_t events = connection_channel_.Events();
        if(buffer_out_.size() == 0) // 数据发完了
        {
            // connection_channel_.SetEvents(events & (~EPOLLOUT)); // 取消EPOLLOUT事件触发（在触发EPOLLOUT执行WriteHandler并且写完后再取消）
            // loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, &connection_channel_));
            write_complete_callback_(shared_from_this());
        }
        else // 系统缓冲区满了，并且数据没发完，设置EPOLLOUT事件触发
        {
            if(events & EPOLLOUT) // 已经设置了EPOLLOUT事件触发
            {
                return ;
            }
            connection_channel_.SetEvents(events | EPOLLOUT);
            loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, &connection_channel_));
        }
    }
    else if(nwrite == 0)
    {
        // CloseHandler();
    }
    else
    {
        LOG_ERROR << "TcpConnection::SendInLoop";
        ErrorHandler();
    }
}

void TcpConnection::Shutdown() // 主动关闭连接
{
    if(loop_->IsInLoopThread())
    {
        ShutdownInLoop();
        return;
    }
    loop_->CommitTaskToLoop(std::bind(&TcpConnection::ShutdownInLoop, shared_from_this())); // 投递到loop_线程，解决线程安全问题
}

void TcpConnection::ShutdownInLoop()
{
    CloseHandler();
}

void TcpConnection::ReadHandler()
{
    if(disconnected_)
    {
        return ;
    }
    ssize_t nread = Utilities::Readn(connection_channel_.Fd(), buffer_in_);
    if(nread > 0)
    {
        if(update_callback_)
        {
            update_callback_(shared_from_this()); // 更新Server层的时间轮
        }
        message_callback_(shared_from_this(), buffer_in_); // Fix bug:因为有可能在收到消息后shutdown，导致引用计数为0，故后面不能再使用shared_from_this，否则会抛出异常崩溃
        // buffer_in_.clear();
    }
    else if(nread == 0) // 客户端关闭socket，FIN
    {
        CloseHandler();
    }
    else
    {
        LOG_ERROR << "TcpConnection::ReadHandler";
        ErrorHandler();
    }
}

void TcpConnection::WriteHandler() // 触发EPOLLOUT
{
    if(disconnected_)
    {
        return ;
    }
    ssize_t nwrite = Utilities::Writen(connection_channel_.Fd(), buffer_out_);
    if(nwrite > 0)
    {
        uint32_t events = connection_channel_.Events();
        if(buffer_out_.size() == 0) // 数据发完了
        {
            connection_channel_.SetEvents(events & (~EPOLLOUT)); // 取消EPOLLOUT事件触发
            loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, &connection_channel_));
            write_complete_callback_(shared_from_this());
        }
        else // 系统缓冲区满了，并且数据没发完，设置EPOLLOUT事件触发
        {
            if(events & EPOLLOUT) // 已经设置了EPOLLOUT事件触发
            {
                return ;
            }
            connection_channel_.SetEvents(events | EPOLLOUT);
            loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, &connection_channel_));
        }
    }
    else if(nwrite == 0)
    {
        // CloseHandler();
    }
    else
    {
        LOG_ERROR << "TcpConnection::WriteHandler";
        ErrorHandler();
    }
}

void TcpConnection::CloseHandler()
{
    if(disconnected_)
    {
        return ;
    }
    disconnected_ = true;
    connection_channel_.SetEvents(0);
    loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, &connection_channel_)); // fix bug:设置不再监听该channel，否则有可能在调用析构函数时还会触发事件
    loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, loop_, &connection_channel_));
    TcpConnectionSPtr prolong = shared_from_this(); // 延长本对象的生命周期至该函数调用结束
    loop_->CommitTaskToLoop(std::bind(connection_cleanup_, prolong)); // 交给TcpServer，从connections_map_中删除
    close_callback_(prolong);
}

void TcpConnection::ErrorHandler()
{
    int err = Socket::GetSocketError(connection_channel_.Fd());
    LOG_ERROR << '[' << LocalAddressToString() << "] -SO_ERROR = " << err << ", " << strerror(err);
}
