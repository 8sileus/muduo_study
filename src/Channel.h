// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_CHANNEL_H__
#define __MUDUO_CHANNEL_H__

#include "Noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>
#include <string>

namespace muduo {

class EventLoop;

class Channel : Noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    /// @brief 供EventLoop调用
    void handleEvent(Timestamp receiveTime);

    //设置
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    void tie(const std::shared_ptr<void>& obj);
    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revents) { revents_ = revents; }

    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    // for Poller
    int index() { return index_; }
    void set_index(int index) { index_ = index; }

    // for debug
    std::string reventsToString() const;
    std::string eventsToString() const;

    void doNotLogHup() { logHup_ = false; }
    EventLoop* ownerLoop() { return loop_; }
    void remove();

private:
    static std::string eventsToString(int fd, int ev);
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

private:
    EventLoop* loop_; //
    const int fd_; //文件描述符
    int events_; //注册的事件
    int revents_; //发生的事件
    int index_; // EPoll中作为状态，Poll中作为下标
    bool logHup_; //是否记录fd关闭信息

    std::weak_ptr<void> tie_; //观察当前channel的存在状态
    bool tied_; // 是否被绑定过
    bool eventHandling_;
    bool addedToLoop_; //该Channel是否被加入loop
    ReadEventCallback readCallback_; //读回调函数
    EventCallback writeCallback_; //写回调函数
    EventCallback closeCallback_; //关闭回调函数
    EventCallback errorCallback_; //错误回调函数
};

} // namespace muduo

#endif //__MUDUO_CHANNEL_H__