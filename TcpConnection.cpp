/**
* @file     TcpConnection.cpp
* @brief    tcp connection
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <TcpConnection.h>
#include <unistd.h>
#include <Utilities.h>
#include <iostream>
#include <assert.h>

TcpConnection::TcpConnection(EventLoop *loop, int fd,
                  const struct sockaddr_in& local_addr, const struct sockaddr_in& peer_addr) :
    disconnected_(true),
    loop_(loop),
    channel_(new Channel()),
    local_addr_(local_addr),
    peer_addr_(peer_addr)
{
    channel_->SetFd(fd);
    channel_->SetEvents(EPOLLRDHUP | EPOLLIN | EPOLLET);
    channel_->SetReadHandle(std::bind(&TcpConnection::HandleRead, this));
    channel_->SetWriteHandle(std::bind(&TcpConnection::HandleWrite, this));
    channel_->SetCloseHandle(std::bind(&TcpConnection::HandleClose, this));
    channel_->SetErrorHandle(std::bind(&TcpConnection::HandleError, this));
}

TcpConnection::~TcpConnection() // TcpConnection的shared_ptr对象引用计数为0
{
    // loop_->RemoveChannelFromEpoller(channel_.get()); // 从该loop的epoller中删除channel事件，此时必定处于该loop_线程
    loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, loop_, channel_.get())); // CommitTaskToLoop内部判断，如果处于当前loop_线程，则直接执行任务，否则投递到loop_线程的任务队列等待执行
    close(channel_->Fd());
    assert(disconnected_); // 确认是否已经关闭
}

void TcpConnection::ConnectEstablished()
{
    // https://blog.csdn.net/u011344601/article/details/51997886?utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-4.control&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-4.control
    // loop_->CommitChannelToEpoller(channel_.get()); // base_loop线程执行，可能导致loop_的epoller的线程安全问题（loop_线程epoll_wait时，base_loop线程调用loop_的epoll_ctl）
    loop_->CommitTaskToLoop(std::bind(&EventLoop::CommitChannelToEpoller, loop_, channel_.get())); // 故加入任务队列，由loop_线程执行添加，此时loop_必定不会处于epoll_wait
    disconnected_ = false;
}

void TcpConnection::Send(const std::string& str)
{
    buffer_out_ += str;
    loop_->CommitTaskToLoop(std::bind(&TcpConnection::SendInLoop, shared_from_this()));
}

void TcpConnection::SendInLoop()
{
    if(disconnected_)
    {
        return ;
    }
    int nwrite = Utilities::Writen(channel_->Fd(), buffer_out_);
    if(nwrite > 0)
    {
        uint32_t events = channel_->Events();
        if(buffer_out_.size() == 0) // 数据发完了
        {
            // channel_->SetEvents(events & (~EPOLLOUT)); // 取消EPOLLOUT事件触发
            // loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, channel_.get()));
            write_complete_callback_(shared_from_this());

            // TODO:半关闭，如果对端关闭后服务端继续写会发送SIGPIPE，导致服务端退出？

        }
        else // 系统缓冲区满了，并且数据没发完，设置EPOLLOUT事件触发
        {
            if(events & EPOLLOUT) // 已经设置了EPOLLOUT事件触发
            {
                return ;
            }
            channel_->SetEvents(events | EPOLLOUT);
            loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, channel_.get()));
        }
    }
    else if(nwrite == 0)
    {
        HandleClose();
    }
    else
    {
        HandleError();
    }
}

void TcpConnection::Shutdown()
{
    loop_->CommitTaskToLoop(std::bind(&TcpConnection::ShutdownInLoop, shared_from_this()));
}

void TcpConnection::ShutdownInLoop()
{
    if(disconnected_)
    {
        return ;
    }
    std::cout << "shutdown" << std::endl;
    close_callback_(shared_from_this());
    // loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, loop_, channel_.get()));
    loop_->CommitTaskToLoop(std::bind(connection_cleanup_, shared_from_this())); // 上层Server层清理（从tcp连接表中删除，shared_ptr计数--）
    disconnected_ = true;
}

void TcpConnection::HandleRead()
{
    if(disconnected_)
    {
        return ;
    }
    ssize_t nread = Utilities::Readn(channel_->Fd(), buffer_in_);
    if(nread > 0)
    {
        message_callback_(shared_from_this(), buffer_in_);
        // buffer_in_.clear();
    }
    else if(nread == 0) // 客户端关闭socket，FIN，设置了EPOLLRDHUP事件，不会发生该情况
    {
        std::cout << "nread = 0" << std::endl;
        HandleClose();
    }
    else
    {
        HandleError();
    }
}

void TcpConnection::HandleWrite() // 触发EPOLLOUT
{
    if(disconnected_)
    {
        return ;
    }
    ssize_t nwrite = Utilities::Writen(channel_->Fd(), buffer_out_);
    if(nwrite > 0)
    {
        uint32_t events = channel_->Events();
        if(buffer_out_.size() == 0) // 数据发完了
        {
            channel_->SetEvents(events & (~EPOLLOUT)); // 取消EPOLLOUT事件触发
            loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, channel_.get()));
            write_complete_callback_(shared_from_this());

            // TODO:半关闭，如果对端关闭后服务端继续写会发送SIGPIPE，导致服务端退出？

        }
        else // 系统缓冲区满了，并且数据没发完，设置EPOLLOUT事件触发
        {
            if(events & EPOLLOUT) // 已经设置了EPOLLOUT事件触发
            {
                return ;
            }
            channel_->SetEvents(events | EPOLLOUT);
            loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, channel_.get()));
        }
    }
    else if(nwrite == 0)
    {
        HandleClose();
    }
    else
    {
        HandleError();
    }
}

void TcpConnection::HandleClose()
{
    if(disconnected_)
    {
        return ;
    }
    // loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, loop_, channel_.get())); // 析构函数会调用
    loop_->CommitTaskToLoop(std::bind(connection_cleanup_, shared_from_this()));
    disconnected_ = true;
}

void TcpConnection::HandleError()
{
    if(disconnected_)
    {
        return ;
    }
    // loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, loop_, channel_.get())); // 析构函数会调用
    loop_->CommitTaskToLoop(std::bind(connection_cleanup_, shared_from_this()));
    disconnected_ = true;
}
