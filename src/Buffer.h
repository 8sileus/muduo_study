// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_BUFFER_H__
#define __MUDUO_BUFFER_H__

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcod

#include "Endian.h"
#include "SocketAPI.h"
#include "StringPiece.h"

#include <algorithm>
#include <string>
#include <vector>

namespace muduo {

class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {
    }
    void swap(Buffer& rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    /// @brief 返回已读的长度
    size_t prependableBytes() const
    {
        return readerIndex_;
    }
    /// @brief 返回可读的长度
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    /// @brief 返回可写的长度
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    /// @brief 返回可读部分的起始地址
    const char* peek() const { return begin() + readerIndex_; }

    /// @brief 查找\\r\\n
    const char* findCRLF() const
    {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }
    const char* findCRLF(const char* start) const
    {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }
    /// @brief 找到结束符
    const char* findEOL() const
    {
        const void* eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }
    const char* findEOL(const char* start) const
    {
        const void* eol = memchr(start, '\n', beginWrite() - start);
        return static_cast<const char*>(eol);
    }

    // 取走可读数据
    void retrieve(size_t len);
    void retrieveUntil(const char* end) { retrieve(end - peek()); }
    void retrieveInt64() { retrieve(sizeof(int64_t)); }
    void retrieveInt32() { retrieve(sizeof(int32_t)); }
    void retrieveInt16() { retrieve(sizeof(int16_t)); }
    void retrieveInt8() { retrieve(sizeof(int8_t)); }
    void retrieveAll() { readerIndex_ = writerIndex_ = kCheapPrepend; }
    std::string retrieveAllAsString() { return retrieveAsString(readableBytes()); }
    std::string retrieveAsString(size_t len);
    StringPiece toStringPiece() const { return StringPiece(peek(), static_cast<int>(readableBytes())); }

    void append(const StringPiece& str) { append(str.data(), str.size()); }
    void append(const void* data, size_t len) { append(static_cast<const char*>(data), len); }
    void append(const char* data, size_t len);

    /// @brief 确保缓冲区长度足够写
    void ensureWritableBytes(size_t len);

    /// @brief 返回可写部分的首地址
    char* beginWrite() { return begin() + writerIndex_; }
    const char* beginWrite() const { return begin() + writerIndex_; }
    /// @brief 增加可读数据
    void hasWritten(size_t len) { writerIndex_ += len; }
    /// @brief 撤回已写数据
    void unwrite(size_t len) { writerIndex_ -= len; }

    // 在后面添加数据
    void appendInt64(int64_t x);
    void appendInt32(int32_t x);
    void appendInt16(int16_t x);
    void appendInt8(int8_t x);
    // 查看数据，不移动readInex_;
    int64_t peekInt64() const;
    int32_t peekInt32() const;
    int16_t peekInt16() const;
    int8_t peekInt8() const;
    // 读数据
    int64_t readInt64();
    int32_t readInt32();
    int16_t readInt16();
    int8_t readInt8();
    // 在前面添加数据
    void prependInt64(int64_t x);
    void prependInt32(int32_t x);
    void prependInt16(int16_t x);
    void prependInt8(int8_t x);
    void prepend(const void* data, size_t len);

    /// @brief 缩小容量
    void shrink(size_t reserve);
    /// @brief 返回容量
    size_t capacity() const { return buffer_.capacity(); }
    /// @brief 从fd中读取数据
    ssize_t readFd(int fd, int* saveErrno);

private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }
    void makeSpace(size_t len);

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[];
};

} // namespace muduo

#endif //__MUDUO_BUFFER_H__