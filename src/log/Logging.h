// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

//  日志工作流程 :
//  1.  调用宏产生匿名Logging对象.              |   LOG_*
//  2.  Logging里的Impl类初始化. Tmpl构造函数.  |   << 日期,时间,线程id,等级,出错信息
//  3.  用户传入的信息.                         |   << something
//  4.  匿名Logging析构,在析构函数中            |   << 文件名,行号.
//      把LogStream类中的Buffer里的数据发给函数g_logoutput,
#ifndef __MUDUO_LOGGING_H__
#define __MUDUO_LOGGING_H__

#include "Timestamp.h"
#include "log/LogStream.h"

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"

namespace muduo {
class Logger {
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS
    };

    class SourceFile {
    public:
        template <int N>
        SourceFile(const char (&arr)[N])
            : data_(arr)
            , size_(N - 1)
        {
            // C 库函数 char *strrchr(const char *str, int c)
            // 在参数 str 所指向的字符串中搜索最后一次出现字符 c（一个无符号字符）的位置
            const char* slash = ::strrchr(data_, '/');
            if (slash) {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char* filename)
            : data_(filename)
        {
            const char* slash = ::strrchr(data_, '/');
            if (slash) {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }
        const char* data_;
        int size_;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    LogStream& stream() { return impl_.stream_; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    using OutputFunc = void (*)(const char* msg, int len);
    using FlushFunc = void (*)();
    static void setOutputFunc(OutputFunc);
    static void setFlushFunc(FlushFunc);
    // static void setTimeZone(const TimeZone& tz);

private:
    class Impl {
    public:
        using LogLevel = Logger::LogLevel;
        Impl(LogLevel level, int old_error, const SourceFile& file, int line);
        void formatTime();
        void finish();

        static const char* s_colors[NUM_LOG_LEVELS];

        Timestamp time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    };
    Impl impl_;
    
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}

// LOG_*的宏，创建一个临时匿名Logger对象，临时匿名很重要，因为临时匿名对象是一使用完就马上销毁，调用析构函数。
// 而C++对于栈中的具名对象，先创建的后销毁。这就使得后创建的Logger对象先于先创建的Logger对象销毁。
// 即先调用析构函数将日志输出，这就会使得日志内容反序（具体说是一个由{}包括的块中反序）。
// 使用临时匿名Logger对象的效果就是：LOG_*这行代码不仅仅包含日志内容，还会马上把日志输出。

#define LOG_TRACE                                          \
    if (muduo::Logger::logLevel() <= muduo::Logger::TRACE) \
    muduo::Logger(__FILE__, __LINE__, muduo::Logger::TRACE, __func__).stream()
#define LOG_DEBUG                                          \
    if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG) \
    muduo::Logger(__FILE__, __LINE__, muduo::Logger::DEBUG, __func__).stream()
#define LOG_INFO                                          \
    if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
    muduo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN muduo::Logger(__FILE__, __LINE__, muduo::Logger::WARN).stream()
#define LOG_ERROR muduo::Logger(__FILE__, __LINE__, muduo::Logger::ERROR).stream()
#define LOG_FATAL muduo::Logger(__FILE__, __LINE__, muduo::Logger::FATAL).stream()
#define LOG_SYSERR muduo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL muduo::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int saveErrno);

#define CHECK_NOTNULL(val) \
    ::muduo::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr)
{
    if (ptr == nullptr) {
        Logger(file, line, Logger::FATAL).stream() << names;
    }
    return ptr;
}
} // namespace muduo

#endif //__MUDUO_LOGGING_H__