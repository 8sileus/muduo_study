// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "HttpRequest.h"

namespace muduo {

bool HttpRequest::setMethod(const char* start, const char* end)
{
    std::string m(start, end);
    if (m == "GET") {
        method_ = kGet;
    } else if (m == "POST") {
        method_ = kPost;
    } else if (m == "HEAD") {
        method_ = kHead;
    } else if (m == "PUT") {
        method_ = kPut;
    } else if (m == "DELETE") {
        method_ = kDelete;
    } else {
        method_ = kInvalid;
    }
    return method_ != kInvalid;
}

const char* HttpRequest::methodString() const
{
    const char* result = "UNKNOWN";
    switch (method_) {
    case kGet:
        result = "GET";
        break;
    case kPost:
        result = "POST";
        break;
    case kHead:
        result = "HEAD";
        break;
    case kPut:
        result = "PUT";
        break;
    case kDelete:
        result = "DELETE";
        break;
    default:
        break;
    }
    return result;
}

void HttpRequest::addHeader(const char* start, const char* colon, const char* end)
{
    std::string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon)) {
        ++colon;
    }
    std::string value(colon, end);
    while (!value.empty() && isspace(value[value.size() - 1])) {
        value.resize(value.size() - 1);
    }
    headers_[field] = value;
}

std::string HttpRequest::getHeader(const std::string& field) const
{
    std::string result;
    auto it = headers_.find(field);
    if (it != headers_.end()) {
        result = it->second;
    }
    return result;
}

void HttpRequest::swap(HttpRequest& other)
{
    std::swap(method_, other.method_);
    std::swap(version_, other.version_);
    path_.swap(other.path_);
    query_.swap(other.query_);
    receiveTime_.swap(other.receiveTime_);
    headers_.swap(other.headers_);
}

} // namespace muduo
