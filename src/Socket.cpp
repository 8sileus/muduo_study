// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Socket.h"
#include "InetAddress.h"
#include "SocketAPI.h"
#include "log/Logging.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>

namespace muduo {

Socket::~Socket()
{
    sockets::Close(sockfd_);
}

bool Socket::getTcpInfo(struct tcp_info* tcpi) const
{
    socklen_t len = sizeof(*tcpi);
    ::memset(tcpi, 0, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::getTcpInfoString(char* buf, int len) const
{
    struct tcp_info tcpi;
    bool ok = getTcpInfo(&tcpi);
    if (ok) {
        snprintf(buf, len, "unrecovered=%u "
                           "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                           "lost=%u retrans=%u rtt=%u rttvar=%u "
                           "sshthresh=%u cwnd=%u total_retrans=%u",
            tcpi.tcpi_retransmits, // Number of unrecovered [RTO] timeouts
            tcpi.tcpi_rto, // Retransmit timeout in usec
            tcpi.tcpi_ato, // Predicted tick of soft clock in usec
            tcpi.tcpi_snd_mss,
            tcpi.tcpi_rcv_mss,
            tcpi.tcpi_lost, // Lost packets
            tcpi.tcpi_retrans, // Retransmitted packets out
            tcpi.tcpi_rtt, // Smoothed round trip time in usec
            tcpi.tcpi_rttvar, // Medium deviation
            tcpi.tcpi_snd_ssthresh,
            tcpi.tcpi_snd_cwnd,
            tcpi.tcpi_total_retrans); // Total retransmits for entire connection
    }
    return ok;
}

void Socket::bind(const InetAddress& addr)
{
    sockets::Bind(sockfd_, addr.getSockAddr());
}

void Socket::listen()
{
    sockets::Listen(sockfd_);
}

int Socket::accept(InetAddress* peeraddr)
{
    struct sockaddr_in6 addr;
    ::memset(&addr, 0, sizeof(addr));
    int connfd = sockets::Accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->setSockAddrInet6(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    sockets::ShutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
        &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
        &optval, static_cast<socklen_t>(sizeof(optval)));
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
        &optval, static_cast<socklen_t>(sizeof(optval)));
    if (ret < 0 && on) {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
#else
    if (on) {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
        &optval, static_cast<socklen_t>(sizeof(optval)));
}

} // namespace muduo
