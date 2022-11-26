// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_POLLER_H__
#define __MUDUO_POLLER_H__

#include <unordered_map>
#include <vector>

#include "EventLoop.h"
#include "Noncopyable.h"
#include "Timestamp.h"

namespace muduo {

class Channel;

class Poller : Noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller();

    //供IO使用的统一接口
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    virtual bool hasChannel(Channel* channel) const;

public:
    /// @brief EventLoop通过此接口获取默认的Poller;
    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    // key:sockfd value:Channel
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;
};

} // namespace muduo

#endif //__MUDUO_POLLER_H__