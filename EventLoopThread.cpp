/**
* @file     EventLoopThread.cpp
* @brief    事件循环IO线程one loop per thread
* @author   lddddd (https://github.com/lddddd1997)
*/

#include <EventLoopThread.h>
#include <iostream>

EventLoopThread::EventLoopThread() :
    thread_id_(-1),
    loop_(nullptr)
{

}

EventLoopThread::~EventLoopThread()
{
    // std::cout << "Clean up the IO thread id: " << thread_id_ << std::endl;
    loop_->Quit();
    thread_.join(); // 清理IO线程
}

void EventLoopThread::Start()
{
    thread_ = std::thread(&EventLoopThread::RunInThread, this); // thread& operator=(thread&& __t) noexcept
}

void EventLoopThread::RunInThread()
{
    thread_id_ = std::this_thread::get_id();
    std::cout << "The IO thread is running, thread id = " << thread_id_ << std::endl;
    EventLoop loop;
    loop_ = &loop;
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
