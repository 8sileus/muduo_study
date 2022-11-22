#include "InetAddress.h"
#include "Endian.h"
#include "SocketAPI.h"
#include "log/Logging.h"

#include <netdb.h>
#include <netinet/in.h>

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

//      struct sockaddr_storage {
//          sa_family_t ss_family; /* Address family */
//          __ss_aligntype __ss_align; /* Force desired alignment.  */
//          char __ss_padding[_SS_PADSIZE];
//      };

namespace muduo {

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
    if (ipv6) {
        ::memset(&addr6_, 0, sizeof(addr6_));
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = sockets::HostToNetwork16(port);
    } else {
        ::memset(&addr_, 0, sizeof(addr_));
        in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = sockets::HostToNetwork32(ip);
        addr_.sin_port = sockets::HostToNetwork16(port);
    }
}

InetAddress::InetAddress(std::string ip, uint16_t port, bool ipv6)
{
    if (ipv6 || strchr(ip.c_str(), ':')) {
        ::memset(&addr6_, 0, sizeof(addr6_));
        sockets::FromIpPort(ip.c_str(), port, &addr6_);
    } else {
        ::memset(&addr_, 0, sizeof(addr_));
        sockets::FromIpPort(ip.c_str(), port, &addr_);
    }
}

std::string InetAddress::toIp() const
{
    char buf[64] = "";
    sockets::ToIp(buf, sizeof(buf), getSockAddr());
    return buf;
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = "";
    sockets::ToIpPort(buf, sizeof(buf), getSockAddr());
    return buf;
}

uint16_t InetAddress::port() const
{
    return sockets::NetworkToHost16(portNetEndian());
}

static thread_local char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(std::string hostname, InetAddress* out)
{
    struct hostent hent;
    struct hostent* he = nullptr;
    int herrno = 0;
    ::memset(&hent, 0, sizeof(hent));

    int ret = ::gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof(t_resolveBuffer), &he, &herrno);
    if (ret == 0 && he != nullptr) {
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    } else {
        if (ret) {
            LOG_SYSERR << "InetAddress::resolve";
        }
        return false;
    }
}

} // namespace muduo
