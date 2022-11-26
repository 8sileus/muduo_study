// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_THREAD_H__
#define __MUDUO_THREAD_H__

#include "Noncopyable.h"

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

namespace muduo {

class Thread : Noncopyable {
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc func, const std::string& name = std::string());
    Thread(Thread&& other);
    // Thread& operator=(Thread&& other);
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    bool joinable() const { return !joined_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }

    static int numCreated() { return numCreated_.load(); }

private:
    void setDefaultName();

private:
    bool started_;
    bool joined_;
    std::unique_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;

    static std::atomic<int32_t> numCreated_;
};

} // namespace muduo

#endif //__MUDUO_THREAD_H__