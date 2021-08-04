/**
* @file     TimerQueue.h
* @brief    timer queue
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef TIMER_QUEUE_H_
#define TIMER_QUEUE_H_

#include <set>
#include <vector>
#include <memory>
#include "EventLoop.h"
#include "Timer.h"

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
    
    void AddTimer(Timestamp when, double interval, TimerCallback callback);
private:
    EventLoop *loop_;
    Channel timer_channel_;
    TimerSet timer_set_;
    
    void AddTimerInLoop(TimerSPtr sp_timer);
    bool InsertToSet(TimerSPtr sp_timer);
    ExpiredList ExpiredTimer(Timestamp now);
    void ExpiredReadHandler();
    
};

#endif
