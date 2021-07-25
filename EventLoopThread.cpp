/**
* @file     EventLoopThread.cpp
* @brief    event loop thread
* @author   lddddd (https://github.com/lddddd1997)
*/

#include <EventLoopThread.h>
#include <iostream>

EventLoopThread::EventLoopThread() :
    loop_(nullptr)
{

}

EventLoopThread::~EventLoopThread()
{
    std::cout << "Clean up the EventLoopThread id: " << std::this_thread::get_id() << std::endl;
    loop_->Quit();
    thread_.join();
}

void EventLoopThread::Start()
{
    thread_ = std::thread(&EventLoopThread::RunInThread, this);
}

void EventLoopThread::RunInThread()
{
    std::thread::id tid = std::this_thread::get_id();
    EventLoop loop;
    loop_ = &loop;
    try
    {
        loop_->Loop();
    }
    catch(const std::exception& ex)
    {
        std::cerr << "exception caught int EventLoopThread::RunInThread at: " << tid << std::endl;
        std::cerr << "reason: " << ex.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
