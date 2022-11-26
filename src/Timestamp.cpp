// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Timestamp.h"

#include <stdio.h> //snprintf
#include <sys/time.h>

namespace muduo {

std::string Timestamp::toFormattedString(bool showMicroseconds) const
{
    char buffer[64] = { 0 };
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    struct tm tm_time;
    localtime_r(&seconds, &tm_time);

    if (showMicroseconds) {
        int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
        snprintf(buffer, sizeof(buffer), "%4d%02d%02d %02d:%02d:%02d.%06d",
            tm_time.tm_year + 1900,
            tm_time.tm_mon + 1,
            tm_time.tm_mday,
            tm_time.tm_hour,
            tm_time.tm_min,
            tm_time.tm_sec,
            microseconds);
    } else {
        snprintf(buffer, sizeof(buffer), "%4d%02d%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900,
            tm_time.tm_mon + 1,
            tm_time.tm_mday,
            tm_time.tm_hour,
            tm_time.tm_min,
            tm_time.tm_sec);
    }
    return buffer;
}

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return Timestamp(tv.tv_sec * kMicroSecondsPerSecond + tv.tv_usec);
}

} // namespace muduo
