/**
* @file     EventLoopThread.h
* @brief    事件循环IO线程one loop per thread
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef EVENT_LOOP_THREAD_H_
#define EVENT_LOOP_THREAD_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include "EventLoop.h"

class EventLoopThread
{
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* StartLoop(); // 启动线程，并等待获取当前线程的loop

private:
    std::thread thread_; // 线程
    std::thread::id thread_id_; // 线程id

    EventLoop *loop_; // 线程运行的loop循环
    void RunInThread(); // 线程执行的函数

    std::mutex mutex_;
    std::condition_variable condition_; // 用于通知loop_在线程中创建成功
};

#endif
