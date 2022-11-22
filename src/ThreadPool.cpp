#include "ThreadPool.h"

#include <stdio.h>

namespace muduo {
ThreadPool::ThreadPool(const std::string& name)
    : name_(name)
    , maxQueueSize_(0)
    , running_(false)
{
}

ThreadPool::~ThreadPool()
{
    if (running_) {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    running_ = true;
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        char id[32];
        ::snprintf(id, sizeof(id), "%d", i + 1);
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this), name_ + id));
        threads_[i]->start();
    }
    if (numThreads == 0 && threadInitCallBack_) {
        threadInitCallBack_();
    }
}

void ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        running_ = false;
        notEmpty_.notify_all();
        notFull_.notify_all();
    }
    for (auto& thread : threads_) {
        thread->join();
    }
}

size_t ThreadPool::queueSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(Task task)
{
    //如果是单线程则直接执行
    if (threads_.empty()) {
        task();
    } else {
        std::unique_lock<std::mutex> lock(mutex_);
        while (isFull() && running_) {
            notFull_.wait(lock);
        }
        if (running_ == false) {
            return;
        }
        queue_.push_back(std::move(task));
        notEmpty_.notify_one();
    }
}

ThreadPool::Task ThreadPool::take()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty() && running_) {
        notEmpty_.wait(lock);
    }
    Task task;
    if (!queue_.empty()) {
        task = std::move(queue_.front());
        queue_.pop_front();
        if (maxQueueSize_ > 0) {
            notFull_.notify_one();
        }
    }
    return task;
}

void ThreadPool::runInThread()
{
    if (threadInitCallBack_) {
        threadInitCallBack_();
    }
    while (running_) {
        Task task(take());
        if (task) {
            task();
        }
    }
}

} // namespace muduo
