/**
* @file     Timestamp.h
* @brief    timestamp
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <stdint.h>
#include <string>
#include <sys/time.h>
#include <iostream>
class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t msse);
    Timestamp(const Timestamp& timestamp) = default;
    ~Timestamp();

    static Timestamp Now(); // 获取当前的时间戳
    std::string ToFormattedString(bool show_micro_seconds = true); const
    bool Valid() const
    {
        return MicroSecondsSinceEpoch() > 0;
    }
    int64_t MicroSecondsSinceEpoch() const
    {
        return micro_seconds_since_epoch_;
    }
    time_t SecondsSinceEpoch() const
    {
        return static_cast<time_t>(micro_seconds_since_epoch_ / MICRO_SECONDS_PER_SECOND);
    }
    void Swap(Timestamp& that)
    {
        std::swap(micro_seconds_since_epoch_, that.micro_seconds_since_epoch_); // 可以直接访问that对象的私有属性，因为C++的限定符是限定类的，不是限定对象的
    }

    bool operator<(const Timestamp& rhs) const
    {
        return this->MicroSecondsSinceEpoch() < rhs.MicroSecondsSinceEpoch();
    }
    Timestamp operator+(const Timestamp& rhs) const
    {
        return Timestamp(this->MicroSecondsSinceEpoch() + rhs.MicroSecondsSinceEpoch());
    }
    Timestamp operator+(double seconds) const
    {
        return Timestamp(this->MicroSecondsSinceEpoch() + static_cast<int64_t>(seconds * MICRO_SECONDS_PER_SECOND));
    }
    bool operator==(const Timestamp& rhs) const
    {
        return this->MicroSecondsSinceEpoch() == rhs.MicroSecondsSinceEpoch();
    }

    static const int MICRO_SECONDS_PER_SECOND = 1000000;

private:
    int64_t micro_seconds_since_epoch_; // since 1970-01-01 00:00:00的UTC时间
};

#endif
