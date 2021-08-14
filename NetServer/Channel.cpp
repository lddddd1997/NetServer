/**
* @file     Channel.cpp
* @brief    事件的封装Channel
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <iostream>
#include <sys/epoll.h>
#include <thread>
#include "Channel.h"

Channel::Channel(const std::string& name) :
    fd_(-1),
    events_(0),
    revents_(0),
    name_(name),
    tied_(false)
{
    
}

Channel::~Channel()
{

}

void Channel::EventsHandlingWithGuard()
{
    std::shared_ptr<void> guard;
    if(tied_)
    {
        guard = tie_.lock();
        if(guard == nullptr)
        {
            return ;
        }
    }
    // if(revents_ & EPOLLRDHUP) // 对端正常关闭（close，或ctrl+c，或SHUT_WR，SHUT_RDWR），触发EPOLLIN和EPOLLRDHUP，故优先处理
    // {
    //     // std::cout << "Event EPOLLRDHUP" << std::endl;
    //     if(close_handler_)
    //     {
    //         close_handler_();
    //     }
    //     else
    //     {
    //         std::cout << "Lack of close_handler_" << std::endl;
    //     }
    //     return ; // 对端关闭后，不处理其它事件，否则调用close_handler_清除tcp连接后还会处理写事件，造成非法访问内存，导致系统崩溃
    // }
    // if(revents_ & EPOLLERR) // 收到RST包会触发EPOLLIN | EPOLLERR | EPOLLHUP
    // {
    //     std::cout << name_ << " Event EPOLLERR " << fd_ << " " << revents_<< std::endl;
    //     if(error_handler_)
    //     {
    //         error_handler_();
    //     }
    //     else
    //     {
    //         std::cout << "Lack of error_handler_" << std::endl;
    //     }
    //     // return ;
    // }
    // if(revents_ & (EPOLLIN | EPOLLPRI)) // 可读或带外数据
    // {
    //     // std::cout << "Event EPOLLIN" << std::endl;
    //     if(read_handler_)
    //     {
    //         read_handler_();
    //     }
    //     else
    //     {
    //         std::cout << "Lack of read_handler_" << std::endl;
    //     }
    // }
    // if(revents_ & EPOLLOUT) // 可写
    // {
    //     std::cout << "Event EPOLLOUT" << std::endl;
    //     if(write_handler_)
    //     {
    //         write_handler_();
    //     }
    //     else
    //     {
    //         std::cout << "Lack of write_handler_" << std::endl;
    //     }
    // }
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) // 优先处理写事件
    {
        if(close_handler_)
        {
            close_handler_();
        }
        else
        {
            std::cout << "Lack of close_handler_" << std::endl;
        }
    }
    if(revents_ & EPOLLERR) // 收到RST包会触发EPOLLIN | EPOLLERR | EPOLLHUP
    {
        // std::cout << name_ << " Event EPOLLERR " << fd_ << std::endl;
        if(error_handler_)
        {
            error_handler_();
        }
        else
        {
            std::cout << "Lack of error_handler_" << std::endl;
        }
    }
    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) // 可读或带外数据
    {
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
