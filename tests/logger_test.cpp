#include "../NetServer/Logger.h"
#include "../NetServer/AsyncLogging.h"
#include <iostream>
#include <thread>
#include <unistd.h>
using namespace std;
// g++ logger_test.cpp ../NetServer/Logger.cpp ../NetServer/Timestamp.cpp ../NetServer/LogStream.cpp -pthread
const int MICRO_SECONDS_PER_SECOND = 1000000;

int64_t Now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return seconds * MICRO_SECONDS_PER_SECOND + tv.tv_usec;
}

void bench()
{
    int64_t t1 = Now();
    for(int i = 0; i < 100000; i++)
        LOG_DEBUG << "123456789 abcdefghijklmnopqrstuvwxyz";
    int64_t delta_t = Now() - t1;
    cout << delta_t << " us" << endl;
    cout << "can wirte " << 1000000 / (delta_t / 1000000.0) << " Bytes per second" << endl;
}

int main()
{
    Logger().EnableAsyncLogging("async_log", 8 * 1024 * 1024);
    bench();
    
    // Logger().DisableAsyncLogging();
    // LOG_DEBUG << "123456789 abcdefghijklmnopqrstuvwxyz";
    // LOG_DEBUG << "123456789 abcdefghijklmnopqrstuvwxyz";
    sleep(5);
    return 0;
}

