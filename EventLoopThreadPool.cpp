/**
* @file     EventLoopThreadPool.cpp
* @brief    event loop thread pool
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <EventLoopThreadPool.h>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop, int thread_num) :
    thread_num_(thread_num),
    index_(0),
    base_loop_(base_loop)
{

}

EventLoopThreadPool::~EventLoopThreadPool()
{

}

void EventLoopThreadPool::Start()
{
    thread_list_.reserve(thread_num_);
    for(int i = 0; i < thread_num_; i++)
    {
        thread_list_.emplace_back(new EventLoopThread);
        thread_list_[i]->Start();
    }
}

EventLoop* EventLoopThreadPool::GetNextLoop()
{
    EventLoop* loop = base_loop_;
    if(!thread_list_.empty())
    {
        loop = thread_list_[index_]->Loop();
        index_ = (index_ + 1) % thread_num_;
    }
    return loop;
}
