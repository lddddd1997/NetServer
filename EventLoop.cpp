/**
* @file     EventLoop.cpp
* @brief    事件循环封装
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <EventLoop.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <iostream>

EventLoop::EventLoop() :
    looping_(false),
    quit_(true),
    thread_id_(std::this_thread::get_id())
{
    wakeupfd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(wakeupfd_ == -1)
    {
        perror("EventLoop::EventLoop");
        exit(EXIT_FAILURE);
    }
    wakeup_channel_.SetFd(wakeupfd_);
    wakeup_channel_.SetEvents(EPOLLIN | EPOLLET);
    wakeup_channel_.SetReadHandle(std::bind(&EventLoop::WakeupHandleRead, this));
    CommitChannelToEpoller(&wakeup_channel_); // 将事件通知加入epoller中
}

EventLoop::~EventLoop()
{
    close(wakeupfd_);
}

void EventLoop::Looping()
{
    looping_ = true;
    quit_ = false;
    // std::cout << ThreadId() <<  " " << "start looping..." << std::endl;

    while(!quit_)
    {
        active_channel_list_.clear();
        epoller_.EpollWait(EPOLLTIMEOUT, active_channel_list_);
        for(Channel *channel : active_channel_list_)
        {
            current_active_channel_ = channel;
            current_active_channel_->HandleEvents(); // 执行活跃事件的回调
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
    //std::cout << "WakeUp" << std::endl;
    Wakeup(); // 跨线程唤醒，worker线程唤醒IO线程
}

void EventLoop::Wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupfd_, &one, sizeof(one));
    if(n != sizeof(one))
    {
        perror("EventLoop::Wakeup");
    }
}

void EventLoop::WakeupHandleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupfd_, &one, sizeof(one));
    if(n != sizeof(one))
    {
        perror("EventLoop::Wakeup");
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

