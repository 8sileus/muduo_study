// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_HTTP_HTTPSERVER_H__
#define __MUDUO_HTTP_HTTPSERVER_H__

#include "TcpServer.h"

namespace muduo {

class HttpRequest;
class HttpResponse;

class HttpServer : Noncopyable {
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;
    HttpServer(EventLoop* loop,
        const InetAddress& listenAddr,
        const std::string& name,
        TcpServer::Option = TcpServer::kNoReusePort);

    void start();

    EventLoop* getLoop() const { return server_.getLoop(); }
    void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }
    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);

private:
    TcpServer server_;
    HttpCallback httpCallback_;
};

} // namespace muduo

#endif //__MUDUO_HTTP_HTTPSERVER_H__