// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_ENDIAN_H__
#define __MUDUO_ENDIAN_H__

#include <endian.h>
#include <stdint.h>

namespace muduo {
namespace sockets {

    inline uint64_t HostToNetwork64(uint64_t host64)
    {
        return htobe64(host64);
    }

    inline uint32_t HostToNetwork32(uint32_t host32)
    {
        return htobe32(host32);
    }

    inline uint16_t HostToNetwork16(uint16_t host16)
    {
        return htobe16(host16);
    }

    inline uint64_t NetworkToHost64(uint64_t net64)
    {
        return be64toh(net64);
    }

    inline uint32_t NetworkToHost32(uint32_t net32)
    {
        return be32toh(net32);
    }

    inline uint16_t NetworkToHost16(uint16_t net16)
    {
        return be16toh(net16);
    }

} // namespace sockets
} // namespace muduo

#endif //__MUDUO_ENDIAN_H__