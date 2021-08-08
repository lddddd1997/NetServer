#include <sys/time.h>
#include <iostream>
#include <unistd.h>
using namespace std;

const int MICRO_SECONDS_PER_SECOND = 1000000;

int64_t Now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return seconds * MICRO_SECONDS_PER_SECOND + tv.tv_usec;
}

std::string ToFormattedString(bool show_micro_seconds)
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(Now() / MICRO_SECONDS_PER_SECOND);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if(show_micro_seconds)
    {
        int microseconds = static_cast<int>(Now() % MICRO_SECONDS_PER_SECOND);
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

void test_localtime()
{
    time_t now = time(nullptr);
    struct tm tm_now = *localtime(&now); // 内部锁机制，效率差
}

void test_gettimeofday()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
}

void test_gmtime_r()
{
    time_t now = time(nullptr);
    struct tm tm_time;
    gmtime_r(&now, &tm_time);
}

const int COUNT = 10000000;

int main()
{
    sleep(1);

    char time_buf[32];
    time_t now = time(nullptr);
    struct tm tm;
    gmtime_r(&now, &tm); // UTC时间
    strftime(time_buf, sizeof(time_buf), ".%Y%m%d-%H%M%S.", &tm);
    cout << "gmtime_r: " << time_buf << endl;
    struct tm tm_now = *localtime(&now); // 本地时间
    strftime(time_buf, sizeof(time_buf), ".%Y%m%d-%H%M%S.", &tm_now);
    cout << "localtime: " << time_buf << endl;
    cout << "gettimeofday: " << ToFormattedString(true) << endl;

    int64_t t1 = Now();
    for(int i = 0; i < COUNT; i++)
        test_localtime();
    cout << "test_localtime: " << Now() - t1 << endl;

    t1 = Now();
    for(int i = 0; i < COUNT; i++)
        test_gettimeofday();
    cout << "test_gettimeofday: " << Now() - t1 << endl;

    t1 = Now();
    for(int i = 0; i < COUNT; i++)
        test_gmtime_r();
    cout << "test_gmtime_r: " << Now() - t1 << endl;

    return 0;
}


