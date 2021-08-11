/**
* @file     TcpConnection.cpp
* @brief    tcp客户端连接
* @author   lddddd (https://github.com/lddddd1997)
* @par      bug fixed:
*           2021.07.31，通过在handler内定义TcpConnectionSPtr prolong = shared_from_this()来延长本对象生命周期至handler调用结束
*/
#include <unistd.h>
#include <iostream>
#include <assert.h>
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
    connection_channel_.SetEvents(EPOLLRDHUP | EPOLLIN | EPOLLET);
    connection_channel_.SetReadHandler(std::bind(&TcpConnection::ReadHandler, this)); // 构造函数中不能使用shared_from_this()
    connection_channel_.SetWriteHandler(std::bind(&TcpConnection::WriteHandler, this)); 
    connection_channel_.SetCloseHandler(std::bind(&TcpConnection::CloseHandler, this)); // 传this，存在线程安全（调用cleanup后，引用计数为0，回收本对象，导致Handler后面的程序无法运行）
    connection_channel_.SetErrorHandler(std::bind(&TcpConnection::ErrorHandler, this)); // fix：在这几个函数中定义智能指针，延长本对象生命周期至handler调用结束
}

TcpConnection::~TcpConnection() // TcpConnection的shared_ptr对象引用计数为0
{
    // std::cout << "TcpConnection::~TcpConnection" << std::endl;
    // loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, loop_, &connection_channel_)); // 无需清除，close后epoll会自动删除，，man文档Q6
    LOG_INFO << "Disconnected " << inet_ntoa(this->LocalAddress().sin_addr)
            << ":" << ntohs(this->LocalAddress().sin_port)
            << " from " << inet_ntoa(this->PeerAddress().sin_addr)
            << ":" << ntohs(this->PeerAddress().sin_port);
    close(connection_channel_.Fd()); // 关闭该连接
    assert(disconnected_); // 确认是否已经关闭
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
    loop_->CommitTaskToLoop(std::bind(&EventLoop::CommitChannelToEpoller, loop_, &connection_channel_)); // 故加入任务队列，由loop_线程执行添加，此时loop_必定不会处于epoll_wait
    disconnected_ = false;
}

void TcpConnection::Send(const std::string& str)
{
    buffer_out_ += str;
    loop_->CommitTaskToLoop(std::bind(&TcpConnection::SendInLoop, shared_from_this())); // 投递到loop_线程，解决线程安全问题
}

void TcpConnection::SendInLoop()
{
    if(disconnected_)
    {
        return ;
    }
    int nwrite = Utilities::Writen(connection_channel_.Fd(), buffer_out_);
    // std::cout << "nwrite = " << nwrite << std::endl;
    if(nwrite > 0)
    {
        uint32_t events = connection_channel_.Events();
        if(buffer_out_.size() == 0) // 数据发完了
        {
            // connection_channel_.SetEvents(events & (~EPOLLOUT)); // 取消EPOLLOUT事件触发（在触发EPOLLOUT执行WriteHandler并且写完后再取消）
            // loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, &connection_channel_));
            write_complete_callback_(shared_from_this());

            // TODO:半关闭，如果对端关闭后服务端继续写会发送SIGPIPE，导致服务端退出？

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
        CloseHandler();
    }
    else
    {
        ErrorHandler();
    }
}

void TcpConnection::Shutdown() // 主动关闭连接
{
    loop_->CommitTaskToLoop(std::bind(&TcpConnection::ShutdownInLoop, shared_from_this())); // 投递到loop_线程，解决线程安全问题
}

void TcpConnection::ShutdownInLoop()
{
    if(disconnected_)
    {
        return ;
    }
    // std::cout << "shutdown" << std::endl;
    loop_->CommitTaskToLoop(std::bind(connection_cleanup_, shared_from_this())); // 上层Server层清理（从tcp连接表中删除，shared_ptr计数--）
    close_callback_(shared_from_this());
    disconnected_ = true;
}

void TcpConnection::ReadHandler()
{
    // std::cout << "shared_from_this().use_count " << shared_from_this().use_count() << std::endl;
    if(disconnected_)
    {
        return ;
    }
    ssize_t nread = Utilities::Readn(connection_channel_.Fd(), buffer_in_);
    // std::cout << "nread = " << nread << std::endl;
    if(nread > 0)
    {
        message_callback_(shared_from_this(), buffer_in_);
        update_callback_(shared_from_this()); // 更新Server层的时间轮
        // buffer_in_.clear();
    }
    else if(nread == 0) // 客户端关闭socket，FIN，设置了优先处理EPOLLRDHUP事件，不会发生该情况
    {
        std::cout << "nread = 0" << std::endl;
        CloseHandler();
    }
    else
    {
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
    // std::cout << "nwrite = " << nwrite << std::endl;
    if(nwrite > 0)
    {
        uint32_t events = connection_channel_.Events();
        if(buffer_out_.size() == 0) // 数据发完了
        {
            connection_channel_.SetEvents(events & (~EPOLLOUT)); // 取消EPOLLOUT事件触发
            loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, &connection_channel_));
            write_complete_callback_(shared_from_this());

            // TODO:半关闭，如果对端关闭后服务端继续写会发送SIGPIPE，导致服务端退出？

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
        CloseHandler();
    }
    else
    {
        ErrorHandler();
    }
}

void TcpConnection::CloseHandler()
{
    if(disconnected_)
    {
        return ;
    }
    TcpConnectionSPtr prolong = shared_from_this(); // 延长本对象的生命周期至该函数调用结束
    loop_->CommitTaskToLoop(std::bind(connection_cleanup_, shared_from_this())); // 交给TcpServer，从connections_map_中删除
    close_callback_(shared_from_this());
    disconnected_ = true;
}

void TcpConnection::ErrorHandler()
{
    if(disconnected_)
    {
        return ;
    }
    TcpConnectionSPtr prolong = shared_from_this(); // 延长本对象的生命周期至该函数调用结束
    loop_->CommitTaskToLoop(std::bind(connection_cleanup_, shared_from_this())); // 交给TcpServer，从connections_map_中删除
    error_callback_(shared_from_this());
    disconnected_ = true;
}
