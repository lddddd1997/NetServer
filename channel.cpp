#include <channel.h>
#include <iostream>
#include <sys/epoll.h>

Channel::Channel() :
    fd_(-1)
{
    
}

Channel::~Channel()
{

}

void Channel::SetFd(int fd)
{
    fd_ = fd;
}

int Channel::GetFd() const
{
    return fd_;
}

void Channel::SetEvents(uint32_t events)
{
    events_ = events;
}

uint32_t Channel::GetEvents() const
{
    return events_;
}

void Channel::SetReadHandle(const EventCallback& cb)
{
    read_callback_ = cb;
}

void Channel::SetWriteHandle(const EventCallback& cb)
{
    write_callback_ = cb;
}

void Channel::SetCloseHandle(const EventCallback& cb)
{
    close_callback_ = cb;
}

void Channel::SetErrorHandle(const EventCallback& cb)
{
    error_callback_ = cb;
}

void Channel::HandleEvents()
{
    if(events_ & EPOLLRDHUP) // 对端正常关闭（close()，或ctrl+c），触发EPOLLIN和EPOLLRDHUP，故将其放第一
    {
        std::cout << "Event EPOLLRDHUP" << std::endl;
        if(close_callback_)
        {
            close_callback_();
        }
        else
        {
            std::cout << "Lack of close_callback_" << std::endl;
        }
    }
    else if(events_ & EPOLLIN) // 可读
    {
        std::cout << "Event EPOLLIN" << std::endl;
        if(read_callback_)
        {
            read_callback_();
        }
        else
        {
            std::cout << "Lack of read_callback_" << std::endl;
        }
    }
    else if(events_ & EPOLLOUT) // 可写
    {
        if(write_callback_)
        {
            write_callback_();
        }
        else
        {
            std::cout << "Lack of write_callback_" << std::endl;
        }
    }
    else if(events_ & EPOLLPRI) // 带外数据
    {
        
    }
    else
    {
        if(error_callback_)
        {
            error_callback_();
        }
        else
        {
            std::cout << "Lack of error_callback_" << std::endl;
        }
    }
}
