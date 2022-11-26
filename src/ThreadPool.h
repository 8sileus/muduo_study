// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_THREADPOOL_H__
#define __MUDUO_THREADPOOL_H__

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <vector>

#include "Noncopyable.h"
#include "Thread.h"

namespace muduo {

class ThreadPool : Noncopyable {
public:
    using Task = std::function<void()>;
    explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
    ~ThreadPool();

    void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
    void setThreadInitCallBack(const Task& cb) { threadInitCallBack_ = cb; }

    void start(int numThreads);
    void stop();

    const std::string& name() const { return name_; }
    size_t queueSize() const;

    void run(Task f);

private:
    //使用isFull前 外部已经加锁
    bool isFull() const { return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_; }
    Task take();

private:
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    std::string name_;
    Task threadInitCallBack_;
    std::vector<std::unique_ptr<Thread>> threads_;
    std::deque<Task> queue_;
    size_t maxQueueSize_;
    bool running_;
};
} // namespace muduo

#endif //__MUDUO_THREADPOOL_H__