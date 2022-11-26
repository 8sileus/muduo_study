// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_TCPCONNECTION_H__
#define __MUDUO_TCPCONNECTION_H__

#include "Buffer.h"
#include "Callbacks.h"
#include "InetAddress.h"
#include "Noncopyable.h"
#include "StringPiece.h"

#include <memory>

struct tcp_info;

namespace muduo {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : Noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {

private:
    enum StateE {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

public:
    using Ptr = std::shared_ptr<TcpConnection>;

    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
        const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }
    bool isReading() const { return reading_; };

    /// @brief 获取tcp详细信息 如果正确返回true
    bool getTcpInfo(struct tcp_info*) const;
    std::string getTcpInfoString() const;
    /// @brief 发送数据
    // void send(std::string&& msg);
    void send(const void* message, int len);
    void send(const StringPiece& message);
    void send(Buffer* Message);
    /// @brief 关闭连接
    void shutdown();
    /// @brief 建立连接
    void connectEstablished();
    /// @brief 销毁连接
    void connectDestroyed();

    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);

    void startRead();
    void stopRead();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

private:
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(std::string&& message);
    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();
    void setState(StateE s) { state_ = s; }
    const char* stateToString() const;
    void startReadInLoop();
    void stopReadInLoop();

private:
    /// @brief 所属的subloop
    EventLoop* loop_;
    const std::string name_;
    StateE state_;
    bool reading_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    //各种回调
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    /// @brief 高水位标记
    size_t highWaterMark_;

    /// @brief 接受数据缓冲区
    Buffer inputBuffer_;
    /// @brief 发送数据缓冲区
    Buffer outputBuffer_;
};

} // namespace muduo

#endif //__MUDUO_TCPCONNECTION_H__