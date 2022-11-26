// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Buffer.h"
#include "SocketAPI.h"

#include <errno.h>
#include <string.h>
#include <sys/uio.h>

namespace muduo {

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

void Buffer::retrieve(size_t len)
{
    if (len < readableBytes()) {
        readerIndex_ += len;
    } else {
        retrieveAll();
    }
}

std::string Buffer::retrieveAsString(size_t len)
{
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

void Buffer::append(const char* data, size_t len)
{
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::ensureWritableBytes(size_t len)
{
    if (writableBytes() < len) {
        makeSpace(len);
    }
}

void Buffer::appendInt64(int64_t x)
{
    int64_t be64 = sockets::HostToNetwork64(x);
    append(&be64, sizeof(be64));
}

void Buffer::appendInt32(int32_t x)
{
    int32_t be32 = sockets::HostToNetwork32(x);
    append(&be32, sizeof(be32));
}

void Buffer::appendInt16(int16_t x)
{
    int16_t be16 = sockets::HostToNetwork16(x);
    append(&be16, sizeof(be16));
}

void Buffer::appendInt8(int8_t x)
{
    append(&x, sizeof(x));
}

int64_t Buffer::peekInt64() const
{
    int64_t be64 = 0;
    ::memcpy(&be64, peek(), sizeof(be64));
    return sockets::NetworkToHost64(be64);
}

int32_t Buffer::peekInt32() const
{
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof(be32));
    return sockets::NetworkToHost32(be32);
}

int16_t Buffer::peekInt16() const
{
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof(be16));
    return sockets::NetworkToHost16(be16);
}

int8_t Buffer::peekInt8() const
{
    int8_t x = *peek();
    return x;
}

int64_t Buffer::readInt64()
{
    int64_t result = peekInt64();
    retrieveInt64();
    return result;
}

int32_t Buffer::readInt32()
{
    int32_t result = peekInt32();
    retrieveInt32();
    return result;
}

int16_t Buffer::readInt16()
{
    int16_t result = peekInt16();
    retrieveInt16();
    return result;
}

int8_t Buffer::readInt8()
{
    int8_t result = peekInt8();
    retrieveInt8();
    return result;
}

void Buffer::prependInt64(int64_t x)
{
    int64_t be64 = sockets::HostToNetwork64(x);
    prepend(&be64, sizeof(be64));
}

void Buffer::prependInt32(int32_t x)
{
    int32_t be32 = sockets::HostToNetwork32(x);
    prepend(&be32, sizeof(be32));
}

void Buffer::prependInt16(int16_t x)
{
    int16_t be16 = sockets::HostToNetwork16(x);
    prepend(&be16, sizeof(be16));
}

void Buffer::prependInt8(int8_t x)
{
    prepend(&x, sizeof(x));
}

void Buffer::prepend(const void* data, size_t len)
{
    readerIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, begin() + readerIndex_);
}

void Buffer::shrink(size_t reserve)
{
    Buffer other;
    other.ensureWritableBytes(readableBytes() + reserve);
    other.append(toStringPiece());
    swap(other);
}

ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf) ? 2 : 1);
    const ssize_t n = sockets::Readv(fd, vec, iovcnt);
    if (n < 0) {
        *saveErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

void Buffer::makeSpace(size_t len)
{
    //已读空间 + 可写空间 < 待写数据
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
        //扩容
        buffer_.resize(writerIndex_ + len);
    } else {
        //前移
        size_t readable = readableBytes();
        std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend + readable;
    }
}

} // namespace muduo
