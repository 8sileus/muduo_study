// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_ASYNCLOGGING_H__
#define __MUDUO_ASYNCLOGGING_H__

#include "Thread.h"
#include "log/LogStream.h"

#include <atomic>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

namespace muduo {
class AsyncLogging : Noncopyable {
public:
    AsyncLogging(const std::string& basename, off_t rollSize, int flushInterval = 3);
    ~AsyncLogging();

    void append(const char* logline, int len);
    void start();
    void stop();

private:
    void threadFunc();

private:
    using Buffer = detail::FixedBuffer<detail::kLargeBuffer>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;

    const int flushInterval_;
    std::atomic<bool> running_;
    const std::string basename_;
    const off_t rollSize_;
    Thread thread_;
    std::promise<void> promise_;
    std::mutex mutex_;
    std::condition_variable cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
};

} // namespace muduo
#endif //__MUDUO_ASYNCLOGGING_H__