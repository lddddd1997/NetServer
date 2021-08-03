/**
* @file     EventLoopThread.cpp
* @brief    事件循环IO线程one loop per thread
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <iostream>
#include "EventLoopThread.h"

EventLoopThread::EventLoopThread() :
    thread_id_(-1),
    loop_(nullptr)
{

}

EventLoopThread::~EventLoopThread()
{
    // std::cout << "Clean up the IO thread id: " << thread_id_ << std::endl;
    if(loop_ != nullptr)
    {
        loop_->Quit();
        thread_.join(); // 清理IO线程
    }
}

EventLoop* EventLoopThread::StartLoop()
{
    thread_ = std::thread(&EventLoopThread::RunInThread, this); // thread& operator=(thread&& __t) noexcept
    EventLoop *loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr)
        {
            condition_.wait(lock); // 等待线程创建好后通知，否则会出现返回的loop_为nullptr
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::RunInThread()
{
    thread_id_ = std::this_thread::get_id();
    // std::cout << "The IO thread is running, thread id = " << thread_id_ << std::endl;
    EventLoop loop("IO");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        condition_.notify_one();
    }
    
    try
    {
        loop_->Looping();
    }
    catch(const std::exception& ex)
    {
        std::cerr << "exception caught int EventLoopThread::RunInThread at: " << thread_id_ << std::endl;
        std::cerr << "reason: " << ex.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
