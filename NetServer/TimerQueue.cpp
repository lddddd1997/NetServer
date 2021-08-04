/**
* @file     TimerQueue.cpp
* @brief    timer queue
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <sys/timerfd.h>
#include <unistd.h>
#include <assert.h>
#include "TimerQueue.h"

TimerQueue::TimerQueue(EventLoop *loop) :
    loop_(loop),
    timer_channel_("timer queue")
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd == -1)
    {
        perror("TimerQueue::TimerQueue");
        exit(EXIT_FAILURE);
    }
    timer_channel_.SetFd(timerfd);
    timer_channel_.SetEvents(EPOLLIN | EPOLLET);
    timer_channel_.SetReadHandler(std::bind(&TimerQueue::ExpiredReadHandler, this));
    loop_->CommitTaskToLoop(std::bind(&EventLoop::CommitChannelToEpoller, loop_, &timer_channel_)); // 将事件通知加入epoller中
}

TimerQueue::~TimerQueue()
{
    close(timer_channel_.Fd());
}

void TimerQueue::AddTimer(Timestamp when, double interval, TimerCallback callback)
{
    TimerSPtr sp_timer = std::make_shared<Timer>(when, interval, callback);
    loop_->CommitTaskToLoop(std::bind(&TimerQueue::AddTimerInLoop, this, sp_timer));
}

void TimerQueue::AddTimerInLoop(TimerSPtr sp_timer) // 引用传递不增加use_count，故不能定义为const TimerSPtr& sp_timer
{
    bool earliest_changed = InsertToSet(sp_timer);
}

bool TimerQueue::InsertToSet(TimerSPtr sp_timer)
{
    bool earliest_changed = false;
    Timestamp when = sp_timer->Expiration();
    TimerSet::iterator it = timer_set_.begin();
    if(it == timer_set_.end() || when < it->first)
    {
        earliest_changed = true;
    }

    std::pair<TimerSet::iterator, bool> result = timer_set_.insert(TimerPair(when, sp_timer));
    assert(result.second);
    return earliest_changed;
}

TimerQueue::ExpiredList TimerQueue::ExpiredTimer(Timestamp now)
{
    return TimerQueue::ExpiredList();
}

void TimerQueue::ExpiredReadHandler()
{

}
