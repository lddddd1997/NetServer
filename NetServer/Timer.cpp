/**
* @file     Timer.cpp
* @brief    timer事件
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <iostream>
#include "Timer.h"

Timer::Timer(Timestamp when, double interval, TimerCallback callback) :
    expiration_(when),
    interval_(interval),
    repeat_(interval > 0.0),
    timer_callback_(std::move(callback))
{

}

Timer::~Timer()
{
    std::cout << "Timer::~Timer" << std::endl;
}

void Timer::ResetExpirationIfRepeat(Timestamp now)
{
    if(repeat_)
    {
        expiration_ = now + interval_;
    }
    else
    {
        expiration_ = Timestamp();
    }
}
