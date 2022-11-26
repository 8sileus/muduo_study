// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketAPI.h"
#include "TcpConnection.h"
#include "log/Logging.h"

#include <errno.h>

namespace muduo {

void defaultConnectionCallback(const TcpConnection::Ptr& conn)
{
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
              << conn->peerAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void defaultMessageCallback(const TcpConnection::Ptr&, Buffer* buf, Timestamp receiveTime)
{
    buf->retrieveAll();
}

TcpConnection::TcpConnection(
    EventLoop* loop,
    const std::string& name,
    int sockfd,
    const InetAddress& localAddr,
    const InetAddress& peerAddr)
    : loop_(loop)
    , name_(name)
    , state_(kConnecting)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_(64 * 1024 * 1024)
{
    //注册回调
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this << " fd=" << channel_->fd();
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this << " fd=" << channel_->fd()
              << " state=" << stateToString();
}

bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const
{
    return socket_->getTcpInfo(tcpi);
}

std::string TcpConnection::getTcpInfoString() const
{
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof(buf));
    return buf;
}

void TcpConnection::send(const void* data, size_t size)
{
    send(std::string_view(static_cast<const char*>(data), size));
}

void TcpConnection::send(Buffer* buffer)
{
    send(std::string_view(buffer->retrieveAllAsString()));
}

void TcpConnection::send(const std::string_view& message)
{
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message.data(), message.size());
        } else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message.data(), message.size()));
        }
    }
}

// 发送数据 应用写的快 而内核发送数据慢 需要把待发送数据写入缓冲区，而且设置了水位回调
void TcpConnection::sendInLoop(const void* data, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    // 如果之前已经被设置为shutdown 就不用再执行了
    if (state_ == kDisconnected) {
        LOG_WARN << "disconnected, give up writing";
        return;
    }
    // IF1:之前已经注册了EPOLLOUT事件 意味着:现在写缓冲区已满
    //      则把数据添加到outBuffer中
    // IF2:之前没有注册EPOLLOUT事件 意味着：写缓冲区空闲
    //      直接调用write即可
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = sockets::Write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                // 发送完毕就给channel取消epollout事件
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            // EWOULDBLOCK = EAGAIN 用于非阻塞模式，表示没有数据的正常返回
            // EPIPE 连接已经关闭
            if (errno != EWOULDBLOCK) {
                LOG_SYSERR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }
    // IF2中如果write没有把数据全部发送出去，则把剩余数据写入outBuffer_,
    // 并给channel注册EPOLLOUT事件,当发送缓冲区有空余空间后会通知channel，
    // 然后调用channel注册的writeCallback_函数，而channel的writeCallback_.
    // 实际上就是TcpConnection设置的handleWrite
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        // 如果旧数据长度<高水位线,同时加上这次的数据后>=高水位线。
        // 这就表明之前没有向loop注册过highWaterMarkCallback_回调函数。
        // 那么这次就向loop注册
        // 反之，则意味着之前已经注册过highWaterMarkCallback_回调函数。
        // 那么就不用再注册了，直接向outBuffer_添加数据即可。
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    //说明outBuffer_已经把数据发完了
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceClose()
{
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

// void TcpConnection::forceCloseWithDelay(double seconds)
// {
//     if (state_ == kConnected || state_ == kDisconnecting) {
//         setState(kDisconnecting);
//         loop_->runAfter(seconds, std::bind(&TcpConnection::forceClose, shared_from_this()));
//     }
// }

void TcpConnection::forceCloseInLoop()
{
    if (state_ == kConnected || state_ == kDisconnecting) {
        handleClose();
    }
}

const char* TcpConnection::stateToString() const
{
    switch (state_) {
    case kDisconnected:
        return "kDisconnected";
    case kConnecting:
        return "kConnecting";
    case kConnected:
        return "kConnected";
    case kDisconnecting:
        return "kDisconnecting";
    default:
        return "unknown state";
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
    if (!reading_ || !channel_->isReading()) {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::stopRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop()
{
    if (reading_ || channel_->isReading()) {
        channel_->disableReading();
        reading_ = false;
    }
}

void TcpConnection::connectEstablished()
{
    LOG_TRACE << "connectEstablished";

    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading(); //向poller注册可读事件

    // 执行连接回调
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    LOG_TRACE << "connectDestroyed";

    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        // 执行连接回调
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) { //有数据到来则执行messageCallback回调
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) { //客户端已经断开
        handleClose();
    } else { //出错
        errno = savedErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting()) {
        ssize_t n = sockets::Write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
    } else {
        LOG_TRACE << "Connection fd = " << channel_->fd() << " is down, no more writing";
    }
}

void TcpConnection::handleClose()
{
    LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();

    setState(kDisconnected);
    channel_->disableAll();
    TcpConnection::Ptr ptr(shared_from_this());
    connectionCallback_(ptr);
    closeCallback_(ptr);
}

void TcpConnection::handleError()
{
    int err = sockets::GetSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

} // namespace muduo
