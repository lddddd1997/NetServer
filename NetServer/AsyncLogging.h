/**
* @file     AsyncLogging.h
* @brief    异步日志
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef ASYNC_LOGGING_H_
#define ASYNC_LOGGING_H_

#include <string>

class AsyncLogging
{
public:
    // using Buffer = 
    AsyncLogging(const std::string& base_name, int roll_size, int flush_interval = 3);
    ~AsyncLogging();

    void Start();
    void Stop();
    void Append(const char *log_line, int len);

private:


    void RunInThread();
};

#endif
