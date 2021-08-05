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
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * MICRO_SECONDS_PER_SECOND + tv.tv_usec);
}
