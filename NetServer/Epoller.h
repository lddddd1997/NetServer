/**
* @file     Epoller.h
* @brief    多路IO复用接口epoll的封装
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef EPOLLER_H_
#define EPOLLER_H_

#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include "Channel.h"

class Epoller
{
public:
    using ChannelPtrList = std::vector<Channel*>;
    using EventList = std::vector<struct epoll_event>;
    using ChannelHashMap = std::unordered_map<int, Channel*>; // 使用unordered_map，效率比map高

    Epoller();
    ~Epoller();

    void EpollWait(int timeout_ms, ChannelPtrList &active_channel_list); // epoll_wait封装，超时时间timeout_ms
    void CommitChannelToEpoller(Channel *channel); // EPOLL_CTL_ADD，提交channel事件到epoll树中
    void RemoveChannelFromEpoller(Channel *channel); // EPOLL_CTL_DEL，从epoll树中移除channel事件
    void UpdateChannelInEpoller(Channel *channel); // EPOLL_CTL_MOD，更新epoll树中的channel事件

private:
    int epollfd_; // epoll文件描述符
    EventList event_list_; // events数组，epoll_wait的传出参数
    ChannelHashMap channel_map_; // 事件集合
    static const int EVENTLISTSIZE = 16; // 初始events数组大小
    void FilleActiveChannels(int nfds, ChannelPtrList &active_channel_list); // 填充活跃事件
};

#endif
