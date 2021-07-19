/**
* @file     thread_pool.cpp
* @brief    thread pool
* @author   lddddd (lddddd1997@qq.com)
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
    typedef std::function<void()> Task;

    ThreadPool(int thread_num);
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ~ThreadPool();
    

    void Start();
    void Stop();
    void CommitTask(Task task); // 向线程池提交任务
    void ExtendPool(int thread_num); // 扩充线程池
    int IdleThreadCount(); // 获取线程总数量
    int ThreadCount(); // 获取空闲线程数量
    int TasksCount(); // 获取任务队列数量

private:
    std::atomic<int> thread_num_; // 线程池的线程总数量（定义为原子类型，多线程中无需加锁，不会被CPU调度打断）
    std::atomic<int> idle_thread_num_; // 空闲线程数量
    std::atomic<bool> running_; // 线程池是否在工作
    std::vector<std::unique_ptr<std::thread>> thread_lists_; // 线程列表
    std::queue<Task> task_queue_; // 任务队列
    void RunInThread(int thread_num); // 线程执行函数

    std::mutex mutex_; //任务队列互斥量
    std::condition_variable condition_; //任务队列同步的条件变量
};

#endif
