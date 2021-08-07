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
    std::cout << "Clean up the business thread pool " << std::endl;
}

void AsyncLogging::Append(const char *log_line, int len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if(current_buffer_->Avail() > len)
    {
        current_buffer_->Append(log_line, len);
    }
    else
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
        condition_.notify_one();
    }
}

void AsyncLogging::RunInThread()
{
    assert(running_ == true);
    LogFile log_output(base_name_, roll_size_, false);
    BufferUPtr new_buffer1(new Buffer);
    BufferUPtr new_buffer2(new Buffer);
    new_buffer1->Bzero();
    new_buffer2->Bzero();
    BufferList buffers_to_write;
    buffers_to_write.reserve(16);
    while(running_)
    {
        assert(buffers_to_write.empty());
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if(buffers_.empty())
            {
                condition_.wait_for(lock, std::chrono::seconds(flush_interval_)); // 超时返回std::cv_status::timeout
            }
            buffers_.push_back(std::move(current_buffer_));
            current_buffer_ = std::move(new_buffer1);
            buffers_to_write.swap(buffers_);
            if(spare_buffer_ == nullptr)
            {
                spare_buffer_ = std::move(new_buffer2);
            }
        }

        if(buffers_to_write.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
               Timestamp::Now().ToFormattedString().c_str(),
               buffers_to_write.size() - 2);
            fputs(buf, stderr);
            log_output.AppendUnlocked(buf, static_cast<int>(strlen(buf)));
            buffers_to_write.erase(buffers_to_write.begin() + 2, buffers_to_write.end());
        }

        for(const BufferUPtr& buffer : buffers_to_write)
        {
            log_output.AppendUnlocked(buffer->Data(), buffer->Length());
        }
        if(buffers_to_write.size() > 2)
        {
            buffers_to_write.resize(2);
        }

        if(new_buffer1 == nullptr)
        {
            assert(!buffers_to_write.empty());
            new_buffer1 = std::move(buffers_to_write.back());
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
        log_output.Flush();
    }
    log_output.Flush();
}
