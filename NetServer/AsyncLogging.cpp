/**
* @file     AsyncLogging.cpp
* @brief    异步日志
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <iostream>
#include <assert.h>
#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"

AsyncLogging::AsyncLogging(const std::string& base_name, int roll_size, int flush_interval) :
    flush_interval_(flush_interval),
    running_(false),
    base_name_(base_name),
    roll_size_(roll_size),
    current_buffer_(new Buffer),
    spare_buffer_(new Buffer)
{
    current_buffer_->Bzero();
    spare_buffer_->Bzero();
    buffers_.reserve(16);
}

AsyncLogging::~AsyncLogging()
{
    Stop();
    std::cout << "Clean up the async logging thread " << std::endl;
}

void AsyncLogging::Append(const char *log_line, int len)
{
    if(!running_)
    {
        std::cout << "Waiting for thread running..." << std::endl;
        return ;
    }
    
    std::lock_guard<std::mutex> lock(mutex_); // 存在多线程同时调用，需上锁
    if(current_buffer_->Avail() > len)
    {
        current_buffer_->Append(log_line, len);
    }
    else // 写满了
    {
        buffers_.push_back(std::move(current_buffer_)); // 右值引用，转移所有权后，current_buffer_ == nullptr
        if(spare_buffer_ != nullptr)
        {
            current_buffer_ = std::move(spare_buffer_);
        }
        else
        {
            current_buffer_.reset(new Buffer);
        }
        current_buffer_->Append(log_line, len);
        condition_.notify_one(); // 通知消费者，多生产者，单消费者模型
    }
}

void AsyncLogging::RunInThread()
{
    assert(running_ == true);
    LogFile log_output(base_name_, roll_size_);
    BufferUPtr new_buffer1(new Buffer);
    BufferUPtr new_buffer2(new Buffer);
    new_buffer1->Bzero();
    new_buffer2->Bzero();
    BufferList buffers_to_write;
    buffers_to_write.reserve(16);
    std::cout << "The async logging thread is running, thread id = " << std::this_thread::get_id() << std::endl;
    while(running_)
    {
        assert(buffers_to_write.empty());
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if(buffers_.empty())
            {
                condition_.wait_for(lock, std::chrono::seconds(flush_interval_)); // 超时返回std::cv_status::timeout
            }
            buffers_.push_back(std::move(current_buffer_)); // 当前写入缓冲区可能没满
            current_buffer_ = std::move(new_buffer1);
            buffers_to_write.swap(buffers_);
            if(spare_buffer_ == nullptr)
            {
                spare_buffer_ = std::move(new_buffer2);
            }
        }

        if(buffers_to_write.size() > 25) // 生产速度远高于消费速度，将丢掉多余日志
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
               Timestamp::Now().ToFormattedString().c_str(),
               buffers_to_write.size() - 2);
            fputs(buf, stderr);
            log_output.AppendUnlocked(buf, static_cast<int>(strlen(buf))); // 单线程处理，无需加锁
            buffers_to_write.erase(buffers_to_write.begin() + 2, buffers_to_write.end());
        }

        for(const BufferUPtr& buffer : buffers_to_write)
        {
            log_output.AppendUnlocked(buffer->Data(), buffer->Length()); // 后端将前端日志持久化
        }
        if(buffers_to_write.size() > 2)
        {
            buffers_to_write.resize(2);
        }

        if(new_buffer1 == nullptr)
        {
            assert(!buffers_to_write.empty());
            new_buffer1 = std::move(buffers_to_write.back()); // 归还缓冲区
            buffers_to_write.pop_back();
            new_buffer1->Reset();
        }
        if(new_buffer2 == nullptr)
        {
            assert(!buffers_to_write.empty());
            new_buffer2 = std::move(buffers_to_write.back());
            buffers_to_write.pop_back();
            new_buffer2->Reset();
        }
        buffers_to_write.clear();
        log_output.Flush(); // 刷新到文件
    }
    log_output.Flush();
}
