/**
* @file     Timer.cpp
* @brief    timer事件
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef TIMER_H_
#define TIMER_H_

#include <functional>
#include "Timestamp.h"

class Timer
{
public:
    using TimerCallback = std::function<void()>;
    Timer(Timestamp when, double interval, TimerCallback callback);
    ~Timer();
    
    void ResetExpirationIfRepeat(Timestamp now); // 重置重复定时器的到期时间
    Timestamp Expiration() const
    {
        return expiration_;
    }
    bool Repeat() const
    {
        return repeat_;
    }
    void ExecuteCallback() // 运行回调函数
    {
        timer_callback_();
    }

private:
    Timestamp expiration_; // 到期时间
    double interval_; // 周期
    bool repeat_; // 是否循环，interval_>0.0则循环，反之不循环
    TimerCallback timer_callback_; // 到期任务回调
};

#endif
