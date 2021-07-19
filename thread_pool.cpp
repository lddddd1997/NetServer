/**
* @file     thread_pool.cpp
* @brief    thread pool
* @author   lddddd (lddddd1997@qq.com)
*/
#include <thread_pool.h>
#include <iostream>

ThreadPool::ThreadPool(int thread_num) :
    thread_num_(thread_num),
    idle_thread_num_(thread_num),
    running_(false),
    mutex_(),
    condition_()
{

}

ThreadPool::~ThreadPool()
{
    Stop();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "Clean up the ThreadPool " << std::endl;
    }
    for(int i = 0; i < thread_num_; i++)
    {
        // thread_lists_[i]->detach();
        thread_lists_[i]->join();
    }
}

void ThreadPool::Start()
{
    running_ = true;
    thread_lists_.reserve(thread_num_);
    for(int i = 0; i < thread_num_; i++)
    {
        thread_lists_.emplace_back(new std::thread(std::bind(&ThreadPool::RunInThread, this, i)));
    }
}

void ThreadPool::Stop()
{
    running_ = false;
    condition_.notify_all();
}

void ThreadPool::CommitTask(Task task)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        task_queue_.push(task);
    }
    condition_.notify_one();
}

void ThreadPool::ExtendPool(int thread_num)
{
    // TODO：线程池扩充
}

int ThreadPool::IdleThreadCount()
{
    return idle_thread_num_;
}

int ThreadPool::ThreadCount()
{
    return thread_num_;
}

int ThreadPool::TasksCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return task_queue_.size();
}

void ThreadPool::RunInThread(int thread_num)
{
    std::thread::id tid = std::this_thread::get_id();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "The " << thread_num << "th thread is running, thread id = " << tid << std::endl;
    }
    Task task;
    while(running_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while(task_queue_.empty() && running_)
            {
                condition_.wait(lock);
            }
            if(!running_)
            {
                break;
            }
            task = task_queue_.front();
            task_queue_.pop();
        }
        if(task) // class function: explicit operator bool() const
        {
            try
            {
                idle_thread_num_--; // 原子类型变量，无需加锁
                task();
                idle_thread_num_++;
            }
            catch(const std::exception& ex)
            {
                std::cerr << "exception caught int ThreadPool::RunInThread at: " << tid << std::endl;
                std::cerr << "reason: " << ex.what() << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}

/*#include <thread_pool.h>
#include <iostream>
#include <unistd.h>
using namespace std;

ThreadPool Pool(10);
void test1()
{
    sleep(1);
}

void test2()
{
    for(int i = 0; i < 10; i++)
    {
        Pool.CommitTask(test1);
        usleep(rand() % 1000000);
    }
}

int main()
{
    
    Pool.Start();
    sleep(1);
    Pool.CommitTask(test2);
    for(int i = 0; i < 10; i++)
    {
        cout << Pool.IdleThreadCount() << " " << Pool.ThreadCount() << " " << Pool.TasksCount() << endl;
        sleep(1);
    }
    return 0;
}*/
