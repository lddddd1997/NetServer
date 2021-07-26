/**
* @file     Channel.cpp
* @brief    事件的封装Channel
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <Channel.h>
#include <iostream>
#include <sys/epoll.h>

Channel::Channel() :
    fd_(-1),
    events_(0),
    revents_(0)
{
    
}

Channel::~Channel()
{

}

void Channel::HandleEvents()
{
    if(revents_ & EPOLLRDHUP) // 对端正常关闭（close()，或ctrl+c），触发EPOLLIN和EPOLLRDHUP，故将其放第一
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
    else if(revents_ & EPOLLIN) // 可读
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
    else if(revents_ & EPOLLOUT) // 可写
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
    else if(revents_ & EPOLLPRI) // 带外数据
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
