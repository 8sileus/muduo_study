// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "log/Logging.h"
#include "CurrentThread.h"
#include "Timestamp.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

//颜色宏定义

namespace muduo {

thread_local char t_errnobuf[512];
thread_local char t_time[64];
thread_local time_t t_lastSecond = 0;

const char* Logger::Impl::s_colors[NUM_LOG_LEVELS] = {
    GREEN, // DEBUG 绿色
    BLUE, // DEBUG 蓝色
    WHITE, // INFO 白色
    YELLOW, // WARN 黄色
    RED, // WARN 红色
    LIGHT_GRAY, // FATAL 灰色
};

const char* strerror_tl(int saveErrno)
{
    return ::strerror_r(saveErrno, t_errnobuf, sizeof(t_errnobuf));
}

Logger::LogLevel initLogLevel()
{
    if (::getenv("MYMUDUO_LOG_TRACE")) {
        return Logger::TRACE;
    } else if (::getenv("MYMUDUO_LOG_DEBUG")) {
        return Logger::DEBUG;
    } else {
        return Logger::INFO;
    }
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL "
};

//辅助类 用于已知长度的字符串
class T {
public:
    T(const char* str, unsigned len)
        : str_(str)
        , len_(len)
    {
    }

    const char* str_;
    const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v)
{
    s.append(v.str_, v.len_);
    return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
    s.append(v.data_, v.size_);
    return s;
}

void defaultOutput(const char* msg, int len)
{
    fwrite(msg, 1, len, stdout);
}

void defaultFlush()
{
    fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

Logger::Impl::Impl(LogLevel level, int saveErrno, const SourceFile& file, int line)
    : time_(Timestamp::now())
    , stream_()
    , level_(level)
    , line_(line)
    , basename_(file)
{
    formatTime();
    current_thread::tid();
    stream_ << T(current_thread::tidString(), current_thread::tidStringLength());
    stream_ << s_colors[level];
    stream_ << T(LogLevelName[level], 6);
    if (saveErrno != 0) {
        stream_ << strerror_tl(saveErrno) << " (errno=" << saveErrno << ")";
    }
}

void Logger::Impl::formatTime()
{
    int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
    if (seconds != t_lastSecond) {
        t_lastSecond = seconds;
        struct tm tm_time;
        ::localtime_r(&seconds, &tm_time);
        ::snprintf(t_time, sizeof(t_time), "[%4d%02d%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900,
            tm_time.tm_mon + 1,
            tm_time.tm_mday,
            tm_time.tm_hour,
            tm_time.tm_min,
            tm_time.tm_sec);
    }
    Fmt us(" +%06d] ", microseconds);
    stream_ << T(t_time, 18) << T(us.data(), 10);
}

void Logger::Impl::finish()
{
    stream_ << NONE << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
    : impl_(INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line)
{
}
Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : impl_(level, 0, file, line)
{
    impl_.stream_ << func << ' ';
}
Logger::Logger(SourceFile file, int line, bool toAbort)
    : impl_(toAbort ? FATAL : ERROR, errno, file, line)
{
}

Logger::~Logger()
{
    impl_.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (impl_.level_ == FATAL) {
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutputFunc(OutputFunc out)
{
    g_output = out;
}

void Logger::setFlushFunc(FlushFunc flush)
{
    g_flush = flush;
}
} // namespace muduo
