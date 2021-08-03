/**
* @file     Channel.cpp
* @brief    事件的封装Channel
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <Channel.h>
#include <iostream>
#include <sys/epoll.h>
#include <thread>

Channel::Channel(const std::string& name) :
    fd_(-1),
    events_(0),
    revents_(0),
    name_(name)
{
    
}

Channel::~Channel()
{

}

void Channel::EventsHandling()
{
    // std::cout << "threadid = " << std::this_thread::get_id() << " Event " << name_ << std::endl;
    if(revents_ & EPOLLRDHUP) // 对端正常关闭（close，或ctrl+c，或SHUT_WR，SHUT_RDWR），触发EPOLLIN和EPOLLRDHUP，故优先处理
    {
        // std::cout << "Event EPOLLRDHUP" << std::endl;
        if(close_handler_)
        {
            close_handler_();
        }
        else
        {
            std::cout << "Lack of close_handler_" << std::endl;
        }
        // return ; // 对端关闭后，不处理其它事件
    }
    if(revents_ & EPOLLERR)
    {
        std::cout << "Event EPOLLERR" << std::endl;
        if(error_handler_)
        {
            error_handler_();
        }
        else
        {
            std::cout << "Lack of error_handler_" << std::endl;
        }
        return ;
    }
    if(revents_ & (EPOLLIN | EPOLLPRI)) // 可读或带外数据
    {
        // std::cout << "Event EPOLLIN" << std::endl;
        if(read_handler_)
        {
            read_handler_();
        }
        else
        {
            std::cout << "Lack of read_handler_" << std::endl;
        }
    }
    if(revents_ & EPOLLOUT) // 可写
    {
        std::cout << "Event EPOLLOUT" << std::endl;
        if(write_handler_)
        {
            write_handler_();
        }
        else
        {
            std::cout << "Lack of write_handler_" << std::endl;
        }
    }
}
