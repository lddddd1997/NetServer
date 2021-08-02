/**
* @file     Timestamp.cpp
* @brief    timestamp
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <Timestamp.h>

Timestamp Timestamp::Now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * MICROSECONDSPERSECOND + tv.tv_usec);
}