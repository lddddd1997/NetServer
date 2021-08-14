/**
* @file     Epoller.cpp
* @brief    多路IO复用接口epoll的封装
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <unistd.h>
#include <iostream>
#include "Epoller.h"
#include "Logger.h"

Epoller::Epoller() :
    event_list_(EVENT_LIST_SIZE)
{
    epollfd_ = epoll_create1(EPOLL_CLOEXEC); // epoll_create
    if(epollfd_ == -1)
    {
        perror("Epoller::Epoller");
        exit(EXIT_FAILURE);
    }
}

Epoller::~Epoller()
{
    close(epollfd_);
}

void Epoller::EpollWait(int timeout_ms, ChannelPtrList& active_channel_list)
{
    int nfds = epoll_wait(epollfd_, &*event_list_.begin(), static_cast<int>(event_list_.size()), timeout_ms);
    if(nfds > 0)
    {
        FilleActiveChannels(nfds, active_channel_list); // 填充活跃事件到传出参数active_channel_list
        if(nfds == static_cast<int>(event_list_.size())) // 扩充events数组
        {
            event_list_.resize(event_list_.size() * 2);
        }
    }
    else if(nfds == 0)
    {

    }
    else
    {
        perror("Epoller::EpollWait");
        // exit(EXIT_FAILURE);
    }

}

void Epoller::CommitChannelToEpoller(Channel *channel)
{
    int fd = channel->Fd();
    struct epoll_event evt;
    evt.events = channel->Events();
    evt.data.ptr = channel; // 浅拷贝
    channel_map_[fd] = channel;
    if(epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &evt) == -1)
    {
        perror("Epoller::CommitChannel");
        exit(EXIT_FAILURE);
    }
}

void Epoller::RemoveChannelFromEpoller(Channel *channel)
{
    int fd = channel->Fd();
    struct epoll_event evt;
    evt.events = channel->Events();
    evt.data.ptr = channel;
    channel_map_.erase(fd);
    if(epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &evt) == -1)
    {
        perror("Epoller::RemoveChannel");
        exit(EXIT_FAILURE);
    }
}

void Epoller::UpdateChannelInEpoller(Channel *channel)
{
    int fd = channel->Fd();
    struct epoll_event evt;
    evt.events = channel->Events();
    evt.data.ptr = channel;

    if(epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &evt) == -1)
    {
        perror("Epoller::UpdateChannel");
        exit(EXIT_FAILURE);
    }
}

void Epoller::FilleActiveChannels(int nfds, ChannelPtrList& active_channel_list)
{
    for(int i = 0; i < nfds; i++)
    {
        Channel *channel = static_cast<Channel*>(event_list_[i].data.ptr);
        ChannelHashMap::const_iterator it = channel_map_.find(channel->Fd());
        if(it != channel_map_.end())
        {
            channel->SetRevents(event_list_[i].events); // 设置epoll的触发事件
            active_channel_list.push_back(channel);
        }
        else
        {

        }
    }
}