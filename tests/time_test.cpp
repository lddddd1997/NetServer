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
    time_t seconds = static_cast<time_t>(Now() / MICRO_SECONDS_PER_SECOND);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
}

const int COUNT = 10000000;

int main()
{
    sleep(1);
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


