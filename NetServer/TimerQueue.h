/**
* @file     TimerQueue.h
* @brief    timer集合管理
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef TIMER_QUEUE_H_
#define TIMER_QUEUE_H_

#include <set>
#include <vector>
#include <memory>
#include "Timer.h"
#include "Channel.h"

class EventLoop;

class TimerQueue
{
public:
    using TimerCallback = std::function<void()>;
    using TimerSPtr = std::shared_ptr<Timer>;
    using TimerPair = std::pair<Timestamp, TimerSPtr>;
    using TimerSet = std::set<TimerPair>;
    using ExpiredList = std::vector<TimerPair>;

    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();
    
    void AddTimer(Timestamp when, double interval, const TimerCallback& callback); // 添加定时器，（线程安全的，内部通过CommitTaskToLoop添加）
private:
    EventLoop *loop_;
    Channel timer_channel_; // 定时channel
    TimerSet timer_set_; // 定时器集合
    
    void AddTimerInLoop(const TimerSPtr& sp_timer); // AddTimer会在所处loop线程中调用
    struct timespec HowMuchTimeFromNow(Timestamp when); // 现在到when的时间差
    void ResetTimerFd(Timestamp expiration); // 重置timerfd的定时触发时间
    bool InsertTimerToSet(const TimerSPtr& sp_timer); // 将定时器插入到集合中，返回该定时器的到期时间是否早于集合中的最早到期时间
    ExpiredList GetExpiredTimers(Timestamp now); // 获取已经到期的定时器
    void ResetTimersIfRepeat(const ExpiredList& expired, Timestamp now); // 重新将重复定时器插入集合中
    void ExpiredReadHandler(); // 到期定时器事件回调
    
};

#endif
