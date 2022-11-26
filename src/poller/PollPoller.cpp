// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "poller/PollPoller.h"
#include "Channel.h"
#include "log/Logging.h"

#include <errno.h>

namespace muduo {

Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::poll(pollfds_.data(), pollfds_.size(), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        LOG_TRACE << " nothing happened";
    } else {
        // EINTR错误的产生：当阻塞于某个慢系统调用的一个进程捕获某个信号且相应信号处理函数返回时，
        // 该系统调用可能返回一个EINTR错误。
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_SYSERR << "PollPoller::poll()";
        }
    }
    return now;
}

void PollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (auto pfd = pollfds_.cbegin();
         numEvents && pfd != pollfds_.cend();
         ++pfd) {
        if (pfd->revents > 0) {
            --numEvents;
            auto ch = channels_.find(pfd->fd);
            auto channel = ch->second;
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void PollPoller::updateChannel(Channel* channel)
{
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    // 不存在
    if (channel->index() < 0) {
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size() - 1);
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    } else {
        int idx = channel->index();
        struct pollfd& pfd = pollfds_[idx];
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {
            // 忽略此buffer
            pfd.fd = -channel->fd() - 1;
        }
    }
}

void PollPoller::removeChannel(Channel* channel)
{
    LOG_TRACE << "fd = " << channel->fd();
    int idx = channel->index();
    const struct pollfd& pfd = pollfds_[idx];
    channels_.erase(channel->fd());

    if (static_cast<size_t>(idx) == pollfds_.size() - 1) {
        pollfds_.pop_back();
    } else {
        int channelAtEnd = pollfds_.back().fd;
        std::iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        // channels_中的key是fd,是一个>=0的数字。
        // 但有可能最后一个pollfd被屏蔽了，即fd被设置为-fd-1;
        // 需要进行 fd-1 取到正数fd。
        if (channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd - 1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
}

} // namespace muduo
