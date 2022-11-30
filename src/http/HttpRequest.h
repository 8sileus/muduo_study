// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_HTTP_HTTPREQUEST_H__
#define __MUDUO_HTTP_HTTPREQUEST_H__

#include "Timestamp.h"

#include <assert.h>
#include <stdio.h>

#include <map>
#include <string>

namespace muduo {

class HttpRequest {
public:
    enum Method {
        kInvalid,
        kGet,
        kPost,
        kHead,
        kPut,
        kDelete
    };
    enum Version {
        kUnknown,
        kHttp10,
        kHttp11
    };

public:
    HttpRequest()
        : method_(kInvalid)
        , version_(kUnknown)
    {
    }
    void swap(HttpRequest& other);

    void setVersion(Version version) { version_ = version; }
    Version getVersion() const { return version_; }
    bool setMethod(const char* start, const char* end);
    const char* methodString() const;
    void addHeader(const char* start, const char* colon, const char* end);
    std::string getHeader(const std::string& field) const;

    void setPath(const char* start, const char* end) { path_.assign(start, end); }
    const std::string& path() const { return path_; }
    void setQuery(const char* start, const char* end) { query_.assign(start, end); }
    const std::string& query() const { return query_; }
    void setReceiveTime(Timestamp t) { receiveTime_ = t; }
    Timestamp receiveTime() const { return receiveTime_; }
    const std::map<std::string, std::string> headers() const { return headers_; }

private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    Timestamp receiveTime_;
    std::map<std::string, std::string> headers_;
};

} // namespace muduo

#endif //__MUDUO_HTTP_HTTPREQUEST_H__