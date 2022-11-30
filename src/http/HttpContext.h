// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_HTTP_HTTPCONTEXT_H__
#define __MUDUO_HTTP_HTTPCONTEXT_H__

#include "http/HttpRequest.h"

namespace muduo {

class Buffer;

class HttpContext {
public:
    /// @brief http解析状态
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

public:
    HttpContext()
        : state_(kExpectRequestLine)
    {
    }

    bool parseRequest(Buffer* buf, Timestamp receiveTime);
    bool gotAll() const { return state_ == kGotAll; }
    void reset()
    {
        state_ = kExpectRequestLine;
        HttpRequest t;
        request_.swap(t);
    }
    const HttpRequest& request() const { return request_; }
    HttpRequest& request() { return request_; }

private:
    bool processRequestLine(const char* begin, const char* end);

private:
    HttpRequestParseState state_;
    HttpRequest request_;
};

} // namespace muduo

#endif //__MUDUO_HTTP_HTTPCONTEXT_H__