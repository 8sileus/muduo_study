// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_TCPCLIENT_H__
#define __MUDUO_TCPCLIENT_H__

#include "Connector.h"
#include "Noncopyable.h"
#include "TcpConnection.h"

#include <mutex>

namespace muduo {

class TcpClient : Noncopyable {
public:
    TcpClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& name);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_;
    }

    EventLoop* getLoop() const { return loop_; }
    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }
    const std::string& name() const { return name_; }

    // 都不是线程安全
    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
    void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnection::Ptr& conn);

private:
    EventLoop* loop_;
    TcpConnection::Ptr connection_;
    Connector::Ptr connector_;
    const std::string name_;
    bool retry_;
    bool connect_;
    int nextConnId_;

    mutable std::mutex mutex_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

} // namespace muduo

#endif //__MUDUO_TCPCLIENT_H__