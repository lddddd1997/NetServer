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

    void Start() // 启动异步日志
    {
        running_ = true;
        log_thread_ = std::thread(&AsyncLogging::RunInThread, this);
    }
    void Stop() // 停止
    {
        running_ = false;
        condition_.notify_all();
        if(log_thread_.joinable())
        {
            log_thread_.join();
        }
    }
    void Append(const char *log_line, int len); // 往缓冲区添加日志
    
private:
    const int flush_interval_; // 条件变量超时时间
    std::atomic<bool> running_; // 后端消费线程运行状态
    const std::string base_name_; // 日志文件前缀名
    const off_t roll_size_; // 日志文件滚动大小，很准确，因为当buffer超过LARGE_BUFFER或flush_interval_超时才会判断是否需要创建文件
    std::thread log_thread_; // 后端线程
    BufferUPtr current_buffer_; // 当前多个生产者正在写入的缓冲区
    BufferUPtr spare_buffer_; // 备用缓冲区
    BufferList buffers_; // 存放已写的缓冲区

    std::mutex mutex_; // 用于保护写入缓冲区
    std::condition_variable condition_; // 通知缓冲区写满

    void RunInThread();
};

#endif
