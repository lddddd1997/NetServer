/**
* @file     EventLoopThread.h
* @brief    event loop thread
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

    void Start();
    EventLoop* Loop()
    {
        return loop_;
    }

private:
    std::thread thread_;
    std::thread::id thread_id_;

    EventLoop *loop_;
    void RunInThread();
};

#endif
