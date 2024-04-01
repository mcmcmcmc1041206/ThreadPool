#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <functional>
#include <future>
#include "mutex.h"
#include <stdexcept>
#include <Condition.h>

class ThreadPool
{
public:
    using tasks_thread = std::function<void()>;
    ThreadPool(size_t);
    template<class T,class... Args>
    auto enqueue(T&& f,Args&&... args)
        ->std::future<class std::result_of<T(Args...)>::type>;
    ~ThreadPool();

private:
    std::vector<std::thread> workers;
    std::queue<tasks_thread> tasks;
    bool stop_;
    MutexLock mutex_;
    Condition cond_;
};

ThreadPool::ThreadPool(size_t threads)
            :stop_(false)
            ,mutex_()
            ,cond_(mutex_)
{
    for(size_t i = 0 ;i < threads;i++)
    {
        workers.emplace_back(
            [this]
            {
                while(true)
                {
                    tasks_thread task;
                    {
                        MutexLockGuard mutexlock(mutex_);
                        while(this->stop_ || !this->tasks.empty())
                        {
                            cond_.wait();
                        }
                        if(this->stop_&&this->tasks.empty())
                            return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            }
        );
    }
    std::cout<<"We have created "<<threads<<"thread now!"<<std::endl;
}

template<class T,class... Args>
auto ThreadPool::enqueue(T&& f,Args&&... args)
        ->std::future<class std::result_of<T(Args...)>::type>
{
    using return_type = class std::result_of<T(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        MutexLockGuard mutexlock(mutex_);
        if(stop_)
        {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks.emplace([task](){(*task)();});
    }
    cond_.notify_one();
    return res;
}


inline ThreadPool::~ThreadPool()
{
    {
        MutexLockGuard mutexlock(mutex_);
        stop_ = true;
    }
    cond_.notifyAll();
    for(std::thread &worker: workers)
        worker.join();
}