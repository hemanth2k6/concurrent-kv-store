#include "redis_lite/thread_pool.hpp"
#include <mutex>
ThreadPool::ThreadPool(size_t num_threads)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers_.emplace_back([this]
                              {
            while(true){
                std::function<void()>task;
                {
                    std::unique_lock lock(mutex_);
                    condition_.wait(lock,[this]{return stop_||!tasks_.empty();});
                    if(stop_&&tasks_.empty()) return ;
                    task=std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            } });
    }
}
ThreadPool::~ThreadPool()
{
    shutdown();
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::unique_lock lock(mutex_);
        if (stop_)
            return;
        tasks_.push(std::move(task));
    }
    condition_.notify_one();
}

void ThreadPool::shutdown()
{
    {
        std::unique_lock lock(mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (auto &worker : workers_)
    {
        if (worker.joinable())
            worker.join();
    }
}