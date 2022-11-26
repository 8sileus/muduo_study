// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "log/LogFile.h"
#include "log/FileUtil.h"

#include "unistd.h"

namespace muduo {

LogFile::LogFile(const std::string& basename,
    off_t rollSize,
    bool threadSafe,
    int flushInterval,
    int checkEveryN)
    : basename_(basename)
    , rollSize_(rollSize)
    , flushInterval_(flushInterval)
    , checkEveryN_(checkEveryN)
    , count_(0)
    , mutex_(threadSafe ? new std::mutex : nullptr)
    , startOfPeriod_(0)
    , lastRoll_(0)
    , lastFlush_(0)
{
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len)
{
    if (mutex_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        append_unlocked(logline, len);
    } else {
        append_unlocked(logline, len);
    }
}

void LogFile::flush()
{
    if (mutex_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        file_->flush();
    } else {
        file_->flush();
    }
}

void LogFile::append_unlocked(const char* logline, int len)
{
    file_->append(logline, len);

    if (file_->writtenBytes() > rollSize_) {
        rollFile();
    } else {
        ++count_;
        if (count_ >= checkEveryN_) {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod_ != startOfPeriod_) { //如果roll间隔超过一天
                rollFile();
            } else if (now - lastFlush_ > flushInterval_) { //如果超过刷新间隔
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

bool LogFile::rollFile()
{
    time_t now = 0;
    std::string filename = getLogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new file_util::AppendFile(filename));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now)
{
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = ::time(NULL);
    localtime_r(now, &tm);
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S", &tm);
    filename += timebuf;

    char pidbuf[32];
    snprintf(pidbuf, sizeof(pidbuf), ".%d", ::getpid());
    filename += pidbuf;
    filename += ".log";
    return filename;
}

} // namespace muduo
