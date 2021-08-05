/**
* @file     EventLoop.h
* @brief    事件循环封装
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#include <thread>
#include <vector>
#include <string>
#include <functional>
#include <atomic>
#include <mutex>
#include "Epoller.h"
#include "Timestamp.h"
#include "TimerQueue.h"

class EventLoop
{
public:
    using Task = std::function<void()>;
    using TaskList = std::vector<Task>;
    using ChannelPtrList = std::vector<Channel*>;
    using TimerCallback = std::function<void()>;

    EventLoop(const std::string& name);
    ~EventLoop();

    void Looping(); // 执行事件循环
    void Quit() // 退出事件循环
    {
        quit_ = true;
        // Wakeup(); // 唤醒loop中的epoll_wait，加快退出
    }
    std::thread::id ThreadId() const // 获取loop所在的线程ID
    {
        return thread_id_;
    }
    bool IsInLoopThread() const // 判断当前调用的线程是否处于loop线程
    {
        return thread_id_ == std::this_thread::get_id();
    }
    void CommitChannelToEpoller(Channel *channel) // 提交channel到epoller中
    {
        epoller_.CommitChannelToEpoller(channel);
    }
    void RemoveChannelFromEpoller(Channel *channel) // 从epoller中移除channel
    {
        epoller_.RemoveChannelFromEpoller(channel);
    }
    void UpdateChannelInEpoller(Channel *channel) // 更新epoller中的channel
    {
        epoller_.UpdateChannelInEpoller(channel);
    }

    void CommitTaskToLoop(const Task& task); // 提交任务至loop事件循环（线程安全的，内部会判断是否处于该线程）
    void RunAt(Timestamp when, const TimerCallback& callback); // when时间运行任务，（线程安全的，内部会调用CommitTaskToLoop）
    void RunAfter(double delay_s, const TimerCallback& callback); // delay秒后运行任务（线程安全的，内部会调用CommitTaskToLoop）
    void RunEvery(double interval_s, const TimerCallback& callback); // 每隔interval秒后运行任务（线程安全的，内部会调用CommitTaskToLoop）

private:
    std::atomic<bool> looping_; // loop运行状态
    std::atomic<bool> quit_; // 事件循环退出状态
    int wakeupfd_; // 事件通知文件描述符
    static const int EPOLL_TIMEOUT = 5000; // epoll_wait的超时时间
    std::string thread_name_; // 线程名
    std::thread::id thread_id_; // loop所在的线程ID
    Epoller epoller_;
    ChannelPtrList active_channel_list_; // 活跃事件列表
    Channel *current_active_channel_; // 当前正在处理的活跃事件
    TaskList task_list_; // 任务列表
    Channel wakeup_channel_; // 事件通知channel
    TimerQueue timer_queue_;
    std::mutex mutex_;  // 用于保护任务列表

    void Wakeup(); // 事件通知，唤醒loop
    void WakeupReadHandler(); // 唤醒后的读回调
    void ExecutePendingTasks(); // 执行任务
};

#endif
