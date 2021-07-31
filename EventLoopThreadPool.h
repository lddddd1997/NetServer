/**
* @file     EventLoopThreadPool.h
* @brief    event loop线程池
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
    using ThreadUPtrList = std::vector<std::unique_ptr<EventLoopThread>>;
    using EventLoopList = std::vector<EventLoop*>;

    EventLoopThreadPool(EventLoop *basic_loop, int thread_num);
    ~EventLoopThreadPool();

    void Start(); // 启动线程池
    EventLoop* GetNextLoop(); // round robin轮询分发loop
private:
    int thread_num_; // 线程数量
    int index_; // 轮询分发的索引
    EventLoop *basic_loop_; // 主loop
    ThreadUPtrList thread_list_; // 线程列表
    EventLoopList loop_list_;
};




#endif
