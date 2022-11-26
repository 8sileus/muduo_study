// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_SOCKETSAPI_H__
#define __MUDUO_SOCKETSAPI_H__

#include <arpa/inet.h>

namespace muduo {
namespace sockets {

    /// 创建一个非阻塞套接字
    int CreateNonblockingOrDie(sa_family_t family);
    int Connect(int sockfd, const struct sockaddr* addr);
    void Bind(int sockfd, const struct sockaddr* addr);
    void Listen(int sockfd);
    int Accept(int sockfd, struct sockaddr_in6* addr);
    ssize_t Read(int sockfd, void* buf, size_t count);
    ssize_t Readv(int sockfd, const struct iovec* iov, int iovcnt);
    ssize_t Write(int sockfd, const void* buf, size_t count);
    void Close(int sockfd);
    void ShutdownWrite(int sockfd);

    void ToIpPort(char* buf, size_t size, const struct sockaddr* addr);
    void ToIp(char* buf, size_t size, const struct sockaddr* addr);
    void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
    void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);

    int GetSocketError(int sockfd);

    const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
    const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
    struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
    const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
    const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

    struct sockaddr_in6 GetLocalAddr(int sockfd);
    struct sockaddr_in6 GetPeerAddr(int sockfd);
    bool IsSelfConnect(int sockfd);

} // namespace sockets
} // namespace muduo

#endif //__MUDUO_SOCKETSAPI_H__