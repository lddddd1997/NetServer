/**
* @file     EventLoopThread.h
* @brief    事件循环IO线程one loop per thread
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef EVENT_LOOP_THREAD_H_
#define EVENT_LOOP_THREAD_H_

#include <thread>
#include <EventLoop.h>

class EventLoopThread
{
public:
    EventLoopThread();
    ~EventLoopThread();

    void Start(); // 启动线程
    EventLoop* Loop() // 获取当前线程的loop
    {
        return loop_;
    }

private:
    std::thread thread_; // 线程
    std::thread::id thread_id_; // 线程id

    EventLoop *loop_; // 线程运行的loop循环
    void RunInThread(); // 线程执行的函数
};

#endif
