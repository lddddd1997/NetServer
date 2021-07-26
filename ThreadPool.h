/**
* @file     ThreadPool.h
* @brief    thread pool
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>

class ThreadPool
{
public:
    using Task = std::function<void()>;
    using ThreadUPtrList = std::vector<std::unique_ptr<std::thread>>;

    ThreadPool(int thread_num);
    ThreadPool(const ThreadPool&) = delete;
    ~ThreadPool();
    ThreadPool& operator=(const ThreadPool&) = delete;

    void Start();
    void Stop()
    {
        running_ = false;
        condition_.notify_all();
    }

    void CommitTaskToPool(const Task& task); // 向线程池提交任务
    void ExpandPool(int thread_num); // 扩充线程池
    int IdleThreadsCount() const // 获取空闲线程数量
    {
        return idle_thread_num_;
    }

    int ThreadsCount() const // 获取线程总数量
    {
        return thread_num_;
    }
    
    int PendingTasksCount() const // 获取等待任务队列数量
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return task_queue_.size();
    }

private:
    std::atomic<int> thread_num_; // 线程池的线程总数量（定义为原子类型，多线程中无需加锁，不会被CPU调度打断）
    std::atomic<int> idle_thread_num_; // 空闲线程数量
    std::atomic<bool> running_; // 线程池是否在工作
    ThreadUPtrList thread_list_; // 线程列表
    std::queue<Task> task_queue_; // 任务队列
    void RunInThread(int thread_num); // 线程执行函数

    mutable std::mutex mutex_; //任务队列互斥量
    std::condition_variable condition_; //任务队列同步的条件变量
};

#endif
