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
    , nextBuffer_(new Buffer)
    , buffers_()
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
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
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_) {
            currentBuffer_ = std::move(nextBuffer_);
        } else {
            currentBuffer_.reset(new Buffer);
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
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (buffers_.empty()) {
                cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (nextBuffer_ == nullptr) {
                nextBuffer_ = std::move(newBuffer2);
            }
        }
        //过多buffer需要写，则只保留两个buffer，其余的都丢弃
        if (buffersToWrite.size() > 25) {
            char buf[256];
            ::snprintf(buf, sizeof(buf), "Dropped log messages at %s, %zd larger buffers\n",
                Timestamp::now().toFormattedString(false).c_str(),
                buffersToWrite.size() - 2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }

        for (const auto& buffer : buffersToWrite) {
            output.append(buffer->data(), buffer->length());
        }

        if (buffersToWrite.size() > 2) {
            // 删除多余的buffer
            buffersToWrite.resize(2);
        }

        if (newBuffer1 == nullptr) {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (newBuffer2 == nullptr) {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}

} // namespace muduo
