// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "log/AsyncLogging.h"
#include "Timestamp.h"
#include "log/LogFile.h"

#include <stdio.h>

namespace muduo {

const int AsyncLogging::s_numBufferMaxSize = 16;

AsyncLogging::AsyncLogging(
    const std::string& basename,
    off_t rollSize,
    int flushInterval)
    : flushInterval_(flushInterval)
    , running_(false)
    , basename_(basename)
    , rollSize_(rollSize)
    , thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging")
    , promise_()
    , mutex_()
    , cond_()
    , currentBuffer_(new Buffer)
    , fullBuffers_()
    , emptyBuffers_()
    , numBuffer_(5)
{
    currentBuffer_->bzero();
    for (int i = 0; i < 4; ++i) {
        emptyBuffers_.emplace_back(new Buffer);
    }
}

AsyncLogging::~AsyncLogging()
{
    if (running_) {
        stop();
    }
}

void AsyncLogging::append(const char* logline, int len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (currentBuffer_->avail() > len) {
        currentBuffer_->append(logline, len);
    } else {
        fullBuffers_.push_back(std::move(currentBuffer_));
        if (!emptyBuffers_.empty()) {
            currentBuffer_ = std::move(emptyBuffers_.front());
            emptyBuffers_.pop_front();
        } else {
            currentBuffer_.reset(new Buffer);
            ++numBuffer_;
        }
        currentBuffer_->append(logline, len);
        cond_.notify_one();
    }
}

void AsyncLogging::start()
{
    running_ = true;
    thread_.start();
    promise_.get_future().wait();
}

void AsyncLogging::stop()
{
    running_ = false;
    cond_.notify_one();
    thread_.join();
}

void AsyncLogging::threadFunc()
{
    promise_.set_value();
    LogFile output(basename_, rollSize_, false);
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (fullBuffers_.empty()) {
                cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
            }
            fullBuffers_.push_back(std::move(currentBuffer_));
            if (emptyBuffers_.empty()) {
                currentBuffer_.reset(new Buffer);
                ++numBuffer_;
            } else {
                currentBuffer_ = std::move(emptyBuffers_.front());
                emptyBuffers_.pop_front();
            }
        }
        //??????buffer??????????????????????????????buffer?????????????????????
        if (numBuffer_ > 25) {
            char buf[256];
            ::snprintf(buf, sizeof(buf), "Dropped log messages at %s, %zd larger buffers\n",
                Timestamp::now().toFormattedString(false).c_str(), fullBuffers_.size() - 2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            fullBuffers_.resize(2);
            numBuffer_ = 2;
        }

        for (const auto& buffer : fullBuffers_) {
            output.append(buffer->data(), buffer->length());
        }
        emptyBuffers_.splice(emptyBuffers_.end(), fullBuffers_);
        output.flush();
    }
    output.flush();
}

} // namespace muduo
