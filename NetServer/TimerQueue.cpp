/**
* @file     TimerQueue.cpp
* @brief    timer集合管理
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <sys/timerfd.h>
#include <unistd.h>
#include <assert.h>
#include <cstring>
#include <iostream>
#include "TimerQueue.h"
#include "EventLoop.h"
#include "Logger.h"

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
    timer_channel_.SetEvents(EPOLLIN/* | EPOLLET*/);
    timer_channel_.SetReadHandler(std::bind(&TimerQueue::ExpiredReadHandler, this));
    loop_->CommitTaskToLoop(std::bind(&EventLoop::CommitChannelToEpoller, loop_, &timer_channel_)); // 将事件通知加入epoller中
}

TimerQueue::~TimerQueue()
{
    close(timer_channel_.Fd());
}

void TimerQueue::AddTimer(Timestamp when, double interval, const TimerCallback& callback)
{
    TimerSPtr sp_timer = std::make_shared<Timer>(when, interval, callback);
    loop_->CommitTaskToLoop(std::bind(&TimerQueue::AddTimerInLoop, this, sp_timer)); // bind的参数完美转发，会增加sp_timer的引用计数
}

void TimerQueue::AddTimerInLoop(const TimerSPtr& sp_timer)
{
    bool earliest_changed = InsertTimerToSet(sp_timer);
    if(earliest_changed) // 如果添加的定时器到期时间比集合中最早的到期时间早
    {
        ResetTimerFd(sp_timer->Expiration());
    }
}

struct timespec TimerQueue::HowMuchTimeFromNow(Timestamp when)
{
    int64_t micro_seconds = when.MicroSecondsSinceEpoch() - Timestamp::Now().MicroSecondsSinceEpoch();
    if(micro_seconds < 100)
    {
        micro_seconds = 100;
    }
    struct timespec ts;
    bzero(&ts, sizeof(ts));
    ts.tv_sec = static_cast<time_t>(micro_seconds / Timestamp::MICRO_SECONDS_PER_SECOND);
    ts.tv_nsec = static_cast<long>((micro_seconds % Timestamp::MICRO_SECONDS_PER_SECOND) * 1000);
    return ts;
}

void TimerQueue::ResetTimerFd(Timestamp expiration)
{
    struct itimerspec new_val;
    struct itimerspec old_val;
    bzero(&new_val, sizeof(new_val));
    bzero(&old_val, sizeof(old_val));
    new_val.it_value = HowMuchTimeFromNow(expiration);
    if(timerfd_settime(timer_channel_.Fd(), 0, &new_val, &old_val) < 0)
    {
        // perror("TimerQueue::ResetTimerFd");
        LOG_ERROR << "TimerQueue::ResetTimerFd";
        // exit(EXIT_FAILURE);
    }
}

bool TimerQueue::InsertTimerToSet(const TimerSPtr& sp_timer)
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

TimerQueue::ExpiredList TimerQueue::GetExpiredTimers(Timestamp now)
{
    ExpiredList expired;
    TimerSet::iterator expired_end = timer_set_.lower_bound(TimerPair(now, nullptr)); // 找出过期的定时器
    std::copy(timer_set_.begin(), expired_end, back_inserter(expired)); // 添加到expired
    timer_set_.erase(timer_set_.begin(), expired_end); // 从set集合清理掉，second引用计数--

    return expired;
}

void TimerQueue::ResetTimersIfRepeat(const ExpiredList& expired, Timestamp now)
{
    for(const TimerPair& it : expired)
    {
        if(it.second->Repeat())
        {
            assert(it.first == it.second->Expiration());
            it.second->ResetExpirationIfRepeat(now); // 重置定时器的到期时间
            InsertTimerToSet(it.second); // 加入定时器集合
        }
        // std::cout << "ResetTimersIfRepeat:" << it.second.use_count() << std::endl;
    }
    if(!timer_set_.empty())
    {
        Timestamp next_expired = timer_set_.begin()->second->Expiration(); // 取出最早到期的定时器
        if(next_expired.Valid())
        {
            ResetTimerFd(next_expired); // 重置timerfd触发为最早到期的时间
        }
    }
}

void TimerQueue::ExpiredReadHandler()
{
    Timestamp now(Timestamp::Now());
    uint64_t howmany;
    ssize_t n = ::read(timer_channel_.Fd(), &howmany, sizeof(howmany));
    if(n != sizeof(howmany))
    {
        // perror("TimerQueue::ExpiredReadHandler");
        LOG_ERROR << "TimerQueue::ExpiredReadHandler";
    }

    ExpiredList expired = GetExpiredTimers(now);
    for(const TimerPair& it : expired)
    {
        it.second->ExecuteCallback(); // 处理到期任务
    }
    ResetTimersIfRepeat(expired, now);
}
