/**
* @file     Timestamp.cpp
* @brief    timestamp
* @author   lddddd (https://github.com/lddddd1997)
*/
#include "Timestamp.h"

Timestamp::Timestamp() :
    micro_seconds_since_epoch_(0)
{

}

Timestamp::Timestamp(int64_t msse) :
    micro_seconds_since_epoch_(msse)
{

}

Timestamp::~Timestamp()
{

}

Timestamp Timestamp::Now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr); // UTC时间，不是系统调用，不会陷入内核
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * MICRO_SECONDS_PER_SECOND + tv.tv_usec);
}

std::string Timestamp::ToFormattedString(bool show_micro_seconds)
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch_ / MICRO_SECONDS_PER_SECOND);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if(show_micro_seconds)
    {
        int microseconds = static_cast<int>(micro_seconds_since_epoch_ % MICRO_SECONDS_PER_SECOND);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                microseconds);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}
