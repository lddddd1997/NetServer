/**
* @file     EventLoop.h
* @brief    event loop
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef EVEVENT_LOOP_H_
#define EVEVENT_LOOP_H_

#include <thread>
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <Epoller.h>

class EventLoop
{
public:
    using Task = std::function<void()>;
    using TaskList = std::vector<Task>;
    using ChannelList = std::vector<Channel*>;

    EventLoop();
    ~EventLoop();

    void Loop();
    void Quit()
    {
        quit_ = true;
    }

    void Wakeup();
    std::thread::id ThreadId() const
    {
        return tid_;
    }

    void CommitChannelToEpoller(Channel *channel)
    {
        epoller_.CommitChannelToEpoller(channel);
    }
    void RemoveChannelFromEpoller(Channel *channel)
    {
        epoller_.RemoveChannelFromEpoller(channel);
    }

    void UpdateChannelInEpoller(Channel *channel)
    {
        epoller_.UpdateChannelInEpoller(channel);
    }

    void HandleRead();
    void CommitTaskToLoop(const Task& task);
    void ExecutePendingTasks();

private:
    std::atomic<bool> looping_;
    std::atomic<bool> quit_;
    int wakeupfd_;
    std::thread::id tid_;
    Epoller epoller_;
    ChannelList active_channel_list_;
    Channel *current_active_channel_;
    TaskList task_list_;
    Channel wakeup_channel_;
    std::mutex mutex_; 
};

#endif
