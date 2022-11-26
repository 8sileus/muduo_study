// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "SocketAPI.h"
#include "Endian.h"
#include "log/Logging.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h> // snprintf
#include <sys/socket.h>
#include <sys/uio.h> // readv
#include <unistd.h>

namespace {

using SA = struct sockaddr;

void SetNonBlockAndCloseOnExec(int sockfd)
{
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK | FD_CLOEXEC;
    ::fcntl(sockfd, F_SETFD, flags);
}

} // namespace

namespace muduo {
namespace sockets {
    const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr)
    {
        return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
    }

    struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr)
    {
        return static_cast<struct sockaddr*>(static_cast<void*>(addr));
    }

    const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
    {
        return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
    }

    const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr)
    {
        return static_cast<const struct sockaddr_in*>(static_cast<const void*>(addr));
    }

    const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr)
    {
        return static_cast<const struct sockaddr_in6*>(static_cast<const void*>(addr));
    }

    int CreateNonblockingOrDie(sa_family_t family)
    {
#if VALGRIND
        int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0) {
            LOG_FATAL("sockets::CreateNonblockingOrDie");
        }
        SetNonBlockAndCloseOnExec(sockfd);
#else
        int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if (sockfd < 0) {
            LOG_SYSFATAL << "sockets::CreateNonblockingOrDie";
        }
#endif
        return sockfd;
    }

    void Bind(int sockfd, const struct sockaddr* addr)
    {
        int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
        if (ret < 0) {
            LOG_SYSFATAL << "sockets::Bind";
        }
    }

    void Listen(int sockfd)
    {
        int ret = ::listen(sockfd, SOMAXCONN);
        if (ret < 0) {
            LOG_SYSFATAL << "sockets::listenOrDie";
        }
    }

    int Accept(int sockfd, struct sockaddr_in6* addr)
    {
        socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
#if VALGRIND || defined(NO_ACCEPT4)
        int connfd = ::accept(sockaddr_cast(addr), &addrlen);
        SetNonBlockAndCloseOnExec(connfd);
#else
        int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif

        if (connfd < 0) {
            int savedErrno = errno;
            LOG_SYSERR << "Socket::accept";
            switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << savedErrno;
                break;
            }
        }
        return connfd;
    }

    int Connect(int sockfd, const struct sockaddr* addr)
    {
        return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    }

    ssize_t Read(int sockfd, void* buf, size_t count)
    {
        return ::read(sockfd, buf, count);
    }

    ssize_t Readv(int sockfd, const struct iovec* iov, int iovcnt)
    {
        return ::readv(sockfd, iov, iovcnt);
    }

    ssize_t Write(int sockfd, const void* buf, size_t count)
    {
        return ::write(sockfd, buf, count);
    }

    void Close(int sockfd)
    {
        if (::close(sockfd) < 0) {
            LOG_SYSERR << "sockets::close";
        }
    }

    void ShutdownWrite(int sockfd)
    {
        if (::shutdown(sockfd, SHUT_WR) < 0) {
            LOG_SYSERR << "sockets::shutdownWrite";
        }
    }

    void ToIpPort(char* buf, size_t size, const struct sockaddr* addr)
    {
        if (addr->sa_family == AF_INET6) {
            buf[0] = '[';
            ToIp(buf + 1, size - 1, addr);
            size_t end = ::strlen(buf);
            const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
            uint16_t port = sockets::NetworkToHost16(addr6->sin6_port);
            ::snprintf(buf + end, size - end, "]:%u", port);
        } else {
            ToIp(buf, size, addr);
            size_t end = ::strlen(buf);
            const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
            uint16_t port = sockets::NetworkToHost16(addr4->sin_port);
            ::snprintf(buf + end, size - end, ":%u", port);
        }
    }
    void ToIp(char* buf, size_t size, const struct sockaddr* addr)
    {
        if (addr->sa_family == AF_INET6) {
            const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
            ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
        } else {
            const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
            ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
        }
    }

    void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
    {
        addr->sin_family = AF_INET;
        addr->sin_port = sockets::HostToNetwork16(port);
        if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
            LOG_SYSERR << "sockets::FromIpPort";
        }
    }

    void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr)
    {
        addr->sin6_family = AF_INET6;
        addr->sin6_port = sockets::HostToNetwork16(port);
        if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
            LOG_SYSERR << "sockets::FromIpPort";
        }
    }

    int GetSocketError(int sockfd)
    {
        int optval;
        socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
            return errno;
        } else {
            return optval;
        }
    }

    struct sockaddr_in6 GetLocalAddr(int sockfd)
    {
        struct sockaddr_in6 localAddr;
        ::memset(&localAddr, 0, sizeof(localAddr));
        socklen_t addrlen = static_cast<socklen_t>(sizeof(localAddr));
        // getsockname函数用于获取与某个套接字关联的本地协议地址
        if (::getsockname(sockfd, sockaddr_cast(&localAddr), &addrlen) < 0) {
            LOG_SYSERR << "sockets::GetLocalAddr";
        }
        return localAddr;
    }

    struct sockaddr_in6 GetPeerAddr(int sockfd)
    {
        // getpeername函数用于获取与某个套接字关联的外地协议地址
        struct sockaddr_in6 peerAddr;
        memset(&peerAddr, 0, sizeof(peerAddr));
        socklen_t addrlen = static_cast<socklen_t>(sizeof(peerAddr));
        if (::getpeername(sockfd, sockaddr_cast(&peerAddr), &addrlen) < 0) {
            LOG_SYSERR << "sockets::GetPeerAddr";
        }
        return peerAddr;
    }

    bool IsSelfConnect(int sockfd)
    {
        struct sockaddr_in6 localaddr = GetLocalAddr(sockfd);
        struct sockaddr_in6 peeraddr = GetPeerAddr(sockfd);
        if (localaddr.sin6_family == AF_INET) {
            const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
            const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
            return laddr4->sin_port == raddr4->sin_port
                && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
        } else if (localaddr.sin6_family == AF_INET6) {
            return localaddr.sin6_port == peeraddr.sin6_port
                && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof(localaddr.sin6_addr)) == 0;
        } else {
            return false;
        }
    }

} // namespace sockets
} // namespace muduo
