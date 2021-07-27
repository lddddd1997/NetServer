/**
* @file     TcpConnection.cpp
* @brief    tcp connection
* @author   lddddd (https://github.com/lddddd1997)
*/

#include <TcpConnection.h>
#include <unistd.h>
#include <Utilities.h>
#include <iostream>

TcpConnection::TcpConnection(EventLoop *loop, int fd,
                  const struct sockaddr_in& local_addr, const struct sockaddr_in& peer_addr) :
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

TcpConnection::~TcpConnection()
{
    // loop_->RemoveChannelFromEpoller(channel_.get()); // 从该loop的epoller中删除channel事件，此时必定处于该loop_线程
    loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, loop_, channel_.get())); // CommitTaskToLoop内部判断，如果处于当前loop_线程，则直接执行任务
    close(channel_->Fd());
}

void TcpConnection::CommitChannelToLoop()
{
    // https://blog.csdn.net/u011344601/article/details/51997886?utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-4.control&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-4.control
    // loop_->CommitChannelToEpoller(channel_.get()); // base_loop线程执行，可能导致loop_的epoller的线程安全问题（loop_线程epoll_wait时，base_loop线程调用loop_的epoll_ctl）
    loop_->CommitTaskToLoop(std::bind(&EventLoop::CommitChannelToEpoller, loop_, channel_.get())); // 故加入任务队列，由loop_线程添加，此时loop_必定不会处于epoll_wait
}

void TcpConnection::HandleRead()
{
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

void TcpConnection::HandleWrite()
{
    ssize_t nwrite = Utilities::Writen(channel_->Fd(), buffer_out_);
    if(nwrite > 0)
    {
        uint32_t events = channel_->Events();
        if(buffer_out_.size() > 0) // 系统缓冲区满了，并且数据没发完，设置EPOLLOUT事件触发
        {
            channel_->SetEvents(events | EPOLLOUT);
            loop_->CommitTaskToLoop(std::bind(&EventLoop::UpdateChannelInEpoller, loop_, channel_.get()));
        }
        else // 数据发完了
        {

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

}

void TcpConnection::HandleError()
{



    // loop_->RemoveChannelFromEpoller(channel_.get()); // 移除loop_线程中epoller的该连接channel事件
    loop_->CommitTaskToLoop(std::bind(&EventLoop::RemoveChannelFromEpoller, loop_, channel_.get()));
}
