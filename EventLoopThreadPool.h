/**
* @file     EventLoopThreadPool.h
* @brief    event loop thread pool
* @author   lddddd (https://github.com/lddddd1997)
*/

#ifndef EVENT_LOOP_THREAD_POOL_H_
#define EVENT_LOOP_THREAD_POOL_H_

#include <vector>
#include "EventLoop.h"
#include "EventLoopThread.h"

class EventLoopThreadPool
{
public:
    using ThreadList = std::vector<std::unique_ptr<EventLoopThread>>;

    EventLoopThreadPool(EventLoop *base_loop, int thread_num);
    ~EventLoopThreadPool();
    void Start();
    EventLoop* GetNextLoop();
private:
    int thread_num_;
    int index_;
    EventLoop *base_loop_;
    ThreadList thread_list_;
};




#endif
