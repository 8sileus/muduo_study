#ifndef __MUDUO_LOGFILE_H__
#define __MUDUO_LOGFILE_H__

#include "Noncopyable.h"

#include <memory>
#include <mutex>
#include <string>

namespace muduo {
namespace file_util {
    class AppendFile;
} // namespace file_util

class LogFile : Noncopyable {
public:
    LogFile(const std::string& basename,
        off_t rollSize,
        bool threadSafe = true,
        int flushInterval = 3,
        int checkEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();
    bool rollFile();

private:
    static const int kRollPerSeconds_ = 60 * 60 * 24;

private:
    void append_unlocked(const char* logline, int len);
    static std::string getLogFileName(const std::string& basename, time_t* now);

private:
    const std::string basename_;
    const off_t rollSize_;
    const int flushInterval_; //刷新间隔
    const int checkEveryN_;
    int count_;
    std::unique_ptr<std::mutex> mutex_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    std::unique_ptr<file_util::AppendFile> file_;
};

} // namespace muduo

#endif //__MUDUO_LOGFILE_H__