/**
* @file     AsyncLogging.h
* @brief    异步日志
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef ASYNC_LOGGING_H_
#define ASYNC_LOGGING_H_

#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "LogStream.h"

class AsyncLogging
{
public:
    using Buffer = FixedBuffer<LogStream::LARGE_BUFFER>;
    using BufferUPtr = std::unique_ptr<Buffer>;
    using BufferList = std::vector<BufferUPtr>;
    AsyncLogging(const std::string& base_name, int roll_size, int flush_interval = 3);
    ~AsyncLogging();

    void Start()
    {
        running_ = true;
        log_thread_ = std::thread(&AsyncLogging::RunInThread, this);
    }
    void Stop()
    {
        running_ = false;
        condition_.notify_all();
        log_thread_.join();
    }
    void Append(const char *log_line, int len);
    
private:
    const int flush_interval_;
    std::atomic<bool> running_;
    const std::string base_name_;
    const off_t roll_size_;
    std::thread log_thread_;
    BufferUPtr current_buffer_;
    BufferUPtr spare_buffer_;
    BufferList buffers_;

    std::mutex mutex_;
    std::condition_variable condition_;

    void RunInThread();
};

#endif
