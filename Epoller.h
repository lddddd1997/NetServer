/**
* @file     Epoller.h
* @brief    epoller
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef EPOLLER_H_
#define EPOLLER_H_

#include <Channel.h>
#include <vector>
#include <map>
#include <sys/epoll.h>

class Epoller
{
public:
    using ChannelList = std::vector<Channel*>;

    Epoller();
    ~Epoller();

    void EpollWait(int timeout_ms, ChannelList &active_channel_list);
    void CommitChannel(Channel *channel);
    void RemoveChannel(Channel *channel);
    void UpdateChannel(Channel *channel);

private:
    int epollfd_;
    std::vector<struct epoll_event> event_list_;
    std::map<int, Channel*> channel_map_;
    static const int EVENTLISTSIZE = 16;
    void FilleActiveChannels(int nfds, ChannelList &active_channel_list);
};

#endif
