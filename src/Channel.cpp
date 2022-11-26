// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Channel.h"
#include "EventLoop.h"
#include "log/Logging.h"

#include <poll.h>

#include <sstream>

namespace muduo {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index_(-1)
    , logHup_(true)
    , tied_(false)
    , eventHandling_(false)
    , addedToLoop_(false)
{
}

Channel::~Channel() = default;

//防止channel被remove掉，channel还在执行回调
void Channel::tie(const std::shared_ptr<void>& obj)
{
    LOG_TRACE << "Channel::tie";
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove()
{
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_) {
        auto guard = tie_.lock();
        if (guard) {
            handleEventWithGuard(receiveTime);
        }
    } else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    eventHandling_ = true;
    LOG_TRACE << reventsToString();
    //  POLLIN 普通或优先级带数据可读
    //  POLLRDNORM 普通数据可读
    //  POLLRDBAND 优先级带数据可读
    //  OLLPRI 高优先级数据可读
    //  POLLOUT 普通数据可写
    //  POLLWRNORM 普通数据可写
    //  POLLWRBAND 优先级带数据可写
    //  POLLERR 发生错误
    //  POLLHUP 对方描述符挂起
    //  POLLNVAL 表示文件描述符的值是无效的

    //关闭
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (logHup_) {
            LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
        }

        if (closeCallback_) {
            closeCallback_();
        }
    }
    if (revents_ & POLLNVAL) {
        LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
    }
    //错误
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) {
            errorCallback_();
        }
    }
    //读
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_) {
            readCallback_(receiveTime);
        }
    }
    //写
    if (revents_ & POLLOUT) {
        if (writeCallback_) {
            writeCallback_();
        }
    }
    eventHandling_ = false;
}

std::string Channel::reventsToString() const
{
    return eventsToString(fd_, revents_);
}

std::string Channel::eventsToString() const
{
    return eventsToString(fd_, events_);
}

std::string Channel::eventsToString(int fd, int events)
{
    std::ostringstream oss;
    oss << fd << ": ";
    if (events & POLLIN)
        oss << "IN ";
    if (events & POLLPRI)
        oss << "PRI ";
    if (events & POLLOUT)
        oss << "OUT ";
    if (events & POLLHUP)
        oss << "HUP ";
    if (events & POLLRDHUP)
        oss << "RDHUP ";
    if (events & POLLERR)
        oss << "ERR ";
    if (events & POLLNVAL)
        oss << "NVAL ";
    return oss.str();
}

} // namespace muduo
