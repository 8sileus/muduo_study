// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_EVENTLOOP_H__
#define __MUDUO_EVENTLOOP_H__

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>

#include "Callbacks.h"
#include "CurrentThread.h"
#include "Noncopyable.h"
#include "Timer.h"
#include "Timestamp.h"

namespace muduo {
class Channel;
class Poller;
class TimerQueue;

class EventLoop : Noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    /// @brief 开始事件循环
    void loop();
    /// @brief 退出事件循环
    void quit();
    //返回poll返回的时间
    Timestamp pollReturnTime() const { return pollReturnTime_; }

    /// @brief 在loop中执行cb。
    void runInLoop(Functor cb);
    /// @brief 把cb让如待执行队列，并唤醒loop线程线程执行。
    void queueInLoop(Functor cb);
    /// @brief 回调函数队列大小
    size_t queueSize() const;

    /// @brief 绝对时间触发timer
    TimerId runAt(Timestamp time, TimerCallback cb);
    /// @brief 相对时间触发timer
    TimerId runAfter(double delay, TimerCallback cb);
    /// @brief 重复触发timer
    TimerId runEvery(double interval, TimerCallback cb);
    /// @brief 删除指定timer
    void cancel(TimerId timerId);

    //唤醒loop所在线程
    void wakeup();
    //调用poller的方法
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    bool isInLoopThread() const { return threadId_ == current_thread::tid(); }
    bool eventHandling() const { return eventHandling_; }

public:
    //返回当前线程的EventLoop
    static EventLoop* getEventLoopOfCurrentThread();

private:
    void handleRead(); // for wakeup
    void doPendingFunctors(); //执行上层回调
    void printActiveChannels() const; // for DEBUG

private:
    using ChannelList = std::vector<Channel*>;

    std::atomic<bool> looping_;
    std::atomic<bool> quit_;
    bool eventHandling_; //标志当前loop是否在执行Channel的handleEvent
    /// @brief 标识当前loop是否有需要执行的回调操作
    bool callingPendingFunctors_;

    /// @brief 记录loop所在线程
    const pid_t threadId_;
    /// @brief poller返回的发生事件的时间点
    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    /// @brief 当main loop获取一个新用户的channel，通过轮询算法，选择一个subloopp，通过该成员唤醒subloop，处理channel
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    /// @brief poller返回的活跃事件列表
    ChannelList activeChannels_;

    /// @brief 存储loop要执行的回调函数
    std::vector<Functor> pendingFunctors_;
    /// @brief pendingFunctors_的锁
    mutable std::mutex mutex_;
};
} // namespace muduo

#endif //__MUDUO_EVENTLOOP_H__
