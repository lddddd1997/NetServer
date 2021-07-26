/**
* @file     EventLoopThreadPool.cpp
* @brief    event loop线程池
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <EventLoopThreadPool.h>
#include <iostream>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop, int thread_num) :
    thread_num_(thread_num),
    index_(0),
    base_loop_(base_loop)
{

}

EventLoopThreadPool::~EventLoopThreadPool()
{
    std::cout << "Clean up the IO thread pool " << std::endl;
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

EventLoop* EventLoopThreadPool::GetNextLoop() // round robin轮询调度
{
    EventLoop *loop = base_loop_; // 若IO线程数量为0，则分配主loop
    if(!thread_list_.empty())
    {
        loop = thread_list_[index_]->Loop();
        index_ = (index_ + 1) % thread_num_;
    }
    return loop;
}
