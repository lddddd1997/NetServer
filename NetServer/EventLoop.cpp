/**
* @file     EventLoop.cpp
* @brief    事件循环封装
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <unistd.h>
#include <sys/eventfd.h>
#include <iostream>
#include "EventLoop.h"
#include "Logger.h"

EventLoop::EventLoop(const std::string& name) :
    looping_(false),
    quit_(true),
    thread_name_(name),
    thread_id_(std::this_thread::get_id()),
    wakeup_channel_("wakeup"),
    timer_queue_(this)
{
    wakeupfd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    // std::cout << "EventLoop " << wakeupfd_ << std::endl;
    if(wakeupfd_ == -1)
    {
        perror("EventLoop::EventLoop");
        exit(EXIT_FAILURE);
    }
    wakeup_channel_.SetFd(wakeupfd_);
    wakeup_channel_.SetEvents(EPOLLIN/* | EPOLLET*/);
    wakeup_channel_.SetReadHandler(std::bind(&EventLoop::WakeupReadHandler, this));
    CommitChannelToEpoller(&wakeup_channel_); // 将事件通知加入epoller中
}

EventLoop::~EventLoop()
{
    std::cout << "EventLoop::~EventLoop" << std::endl;
    close(wakeupfd_);
}

void EventLoop::Looping()
{
    looping_ = true;
    quit_ = false;
    std::cout << "The " << thread_name_ << " thread is running, thread id = " << thread_id_ << std::endl;
    while(!quit_)
    {
        active_channel_list_.clear();
        epoller_.EpollWait(EPOLL_TIMEOUT, active_channel_list_);
        for(Channel *channel : active_channel_list_)
        {
            current_active_channel_ = channel;
            current_active_channel_->EventsHandlingWithGuard(); // 执行活跃事件的回调
        }
        current_active_channel_ = nullptr;
        ExecutePendingTasks(); // 执行任务
    }
    looping_ = false;
}

void EventLoop::CommitTaskToLoop(const Task& task)
{
    // std::cout << "commit id: " << std::this_thread::get_id() << " loop id: " << ThreadId() << std::endl;
    if(IsInLoopThread()) // 如果在当前线程，则直接执行任务，减少IO次数
    {
        task();
        return ;
    }
    {
        std::lock_guard <std::mutex> lock(mutex_);                    
        task_list_.push_back(task); 
    }
    // std::cout << "WakeUp  " << wakeupfd_ << std::endl;
    Wakeup(); // 跨线程唤醒，唤醒IO线程
}

void EventLoop::RunAt(Timestamp when, const TimerCallback& callback)
{
    timer_queue_.AddTimer(when, 0.0, callback);
}

void EventLoop::RunAfter(double delay_s, const TimerCallback& callback)
{
    RunAt(Timestamp::Now() + delay_s, callback);
}

void EventLoop::RunEvery(double interval_s, const TimerCallback& callback)
{
    timer_queue_.AddTimer(Timestamp::Now() + interval_s, interval_s, callback);
}

void EventLoop::Wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupfd_, &one, sizeof(one));
    if(n != sizeof(one))
    {
        // perror("EventLoop::Wakeup");
        LOG_ERROR << "EventLoop::Wakeup";
    }
}

void EventLoop::WakeupReadHandler()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupfd_, &one, sizeof(one));
    if(n != sizeof(one))
    {
        // perror("EventLoop::WakeupReadHandler");
        LOG_ERROR << "EventLoop::WakeupReadHandler";
    }
}

void EventLoop::ExecutePendingTasks()
{
    TaskList task_list;
    {
        std::lock_guard <std::mutex> lock(mutex_);
        task_list.swap(task_list_); // 交换后再操作，减少锁的作用范围
    }
    // std::cout << "task_list size: " << task_list.size() << std::endl;
    for(const Task& task : task_list)
    {
        task();
    }
}

