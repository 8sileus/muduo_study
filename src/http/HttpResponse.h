// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef __MUDUO_HTTP_HTTPRESPONSE_H__
#define __MUDUO_HTTP_HTTPRESPONSE_H__

#include <map>

namespace muduo {

class Buffer;
class HttpResponse {
public:
    enum HttpStatusCode {
        kUnknown,
        k200Ok = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

public:
    explicit HttpResponse(bool close)
        : statusCode_(kUnknown)
        , closeConnection_(close)
    {
    }

    void setStatusCode(HttpStatusCode code) { statusCode_ = code; }
    void setStatusMessage(const std::string& message) { statusMessage_ = message; }
    void setCloseConnection(bool on) { closeConnection_ = on; }
    bool closeConnection() const { return closeConnection_; }
    void setContentType(const std::string& contentType) { addHeader("Content-Type", contentType); }
    void addHeader(const std::string& key, const std::string& value) { headers_[key] = value; }
    void setBody(const std::string& body) { body_ = body; }

    void appendToBuffer(Buffer* output) const;

private:
    std::map<std::string, std::string> headers_;
    HttpStatusCode statusCode_;
    std::string statusMessage_;
    bool closeConnection_;
    std::string body_;
};

} // namespace muduo

#endif //__MUDUO_HTTP_HTTPRESPONSE_H__