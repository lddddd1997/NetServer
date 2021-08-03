/**
* @file     Timestamp.h
* @brief    timestamp
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <stdint.h>
#include <sys/time.h>

class Timestamp
{
public:
    Timestamp();
    Timestamp(int64_t msse);
    ~Timestamp();

    static Timestamp Now();

    int MicroSecondsSinceEpoch() const
    {
        return micro_seconds_since_epoch_;
    }
    time_t SecondsSinceEpoch() const
    {
        return static_cast<time_t>(micro_seconds_since_epoch_ / MICROSECONDSPERSECOND);
    }
    static const int MICROSECONDSPERSECOND = 1000 * 1000;
private:
    int64_t micro_seconds_since_epoch_;
};

#endif
