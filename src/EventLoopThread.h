// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_EVENTLOOPTHREAD_H__
#define __MUDUO_EVENTLOOPTHREAD_H__

#include "Thread.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

namespace muduo {

class EventLoop;

class EventLoopThread : Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
        const std::string& name = std::string());
    ~EventLoopThread();

    EventLoopThread(EventLoopThread&&) = default;
    EventLoopThread& operator=(EventLoopThread&&) = default;

    EventLoop* startLoop();

private:
    void threadFunc();

private:
    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

} // namespace muduo

#endif //__MUDUO_EVENTLOOPTHREAD_H__