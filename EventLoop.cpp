/**
* @file     EventLoop.cpp
* @brief    event loop
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <EventLoop.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <iostream>

EventLoop::EventLoop() :
    looping_(false),
    quit_(true),
    tid_(std::this_thread::get_id())
{
    wakeupfd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(wakeupfd_ == -1)
    {
        perror("EventLoop::EventLoop");
        exit(EXIT_FAILURE);
    }
    wakeup_channel_.SetFd(wakeupfd_);
    wakeup_channel_.SetEvents(EPOLLIN | EPOLLET);
    wakeup_channel_.SetReadHandle(std::bind(&EventLoop::HandleRead, this));
    CommitChannelToEpoller(&wakeup_channel_);
}

EventLoop::~EventLoop()
{
    close(wakeupfd_);
}

void EventLoop::Loop()
{
    looping_ = true;
    quit_ = false;
    std::cout << "Start looping..." << std::endl;

    while(!quit_)
    {
        active_channel_list_.clear();
        epoller_.EpollWait(5000, active_channel_list_);
        for(Channel* channel : active_channel_list_)
        {
            current_active_channel_ = channel;
            current_active_channel_->HandleEvents();
        }
        current_active_channel_ = nullptr;
        ExecutePendingTasks();
    }
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

void EventLoop::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupfd_, &one, sizeof(one));
    if(n != sizeof(one))
    {
        perror("EventLoop::Wakeup");
    }
}

void EventLoop::CommitTaskToLoop(const Task& task)
{
        {
            std::lock_guard <std::mutex> lock(mutex_);                    
            task_list_.push_back(task); 
        }
        //std::cout << "WakeUp" << std::endl;
        Wakeup(); //跨线程唤醒，worker线程唤醒IO线程
}

void EventLoop::ExecutePendingTasks()
{
    TaskList task_list;
    {
        std::lock_guard <std::mutex> lock(mutex_);
        task_list.swap(task_list_);
    }
    for(const Task& task : task_list)
    {
        task();
    }

}

