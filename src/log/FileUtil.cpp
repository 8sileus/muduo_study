// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "FileUtil.h"
#include "log/Logging.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

namespace muduo {

file_util::AppendFile::AppendFile(std::string_view filename)
    // a:append w:write r:read e:O_CLOEXE
    : fp_(::fopen(filename.data(), "ae"))
    , writtenBytes_(0)
{
    ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

file_util::AppendFile::~AppendFile()
{
    ::fclose(fp_);
}

void file_util::AppendFile::append(const char* logline, size_t len)
{
    size_t written = 0;
    while (written != len) {
        size_t remain = len - written;
        size_t n = write(logline + written, remain);
        if (n != remain) {
            int err = ::ferror(fp_);
            if (err) {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
                break;
            }
        }
        written += n;
    }
    writtenBytes_ += written;
}

void file_util::AppendFile::flush()
{
    ::fflush(fp_);
}

size_t file_util::AppendFile::write(const char* logline, size_t len)
{
    return ::fwrite_unlocked(logline, 1, len, fp_);
}

file_util::ReadSmallFile::ReadSmallFile(std::string_view filename)
    : fd_(::open(filename.data(), O_RDONLY | O_CLOEXEC))
    , err_(0)
{
    buffer_[0] = '\0';
    if (fd_ < 0) {
        err_ = errno;
    }
}

file_util::ReadSmallFile::~ReadSmallFile()
{
    if (fd_ > 0) {
        ::close(fd_);
    }
}

template <typename String>
int file_util::ReadSmallFile::readToString(
    int maxSize,
    String* content,
    int64_t* fileSize,
    int64_t* modifyTime,
    int64_t* createTime)
{
    int err = err_;
    if (fd_ >= 0) {
        content->clear();
        if (fileSize) {
            struct stat statbuf;
            if (::fstat(fd_, &statbuf) == 0) {
                if (S_ISREG(statbuf.st_mode)) {
                    *fileSize = statbuf.st_size;
                    content->reserve(static_cast<int>(std::min(static_cast<int64_t>(maxSize), *fileSize)));
                } else if (S_ISDIR(statbuf.st_mode)) {
                    err = EISDIR;
                }
                if (modifyTime) {
                    *modifyTime = statbuf.st_mtime;
                }
                if (createTime) {
                    *createTime = statbuf.st_ctime;
                }
            } else {
                err = errno;
            }
        }

        while (content->size() < static_cast<size_t>(maxSize)) {
            size_t toRead = std::min(static_cast<size_t>(maxSize) - content->size(), sizeof(buffer_));
            ssize_t n = ::read(fd_, buffer_, toRead);
            if (n > 0) {
                content->append(buffer_, n);
            } else {
                if (n < 0) {
                    err = errno;
                }
                break;
            }
        }
    }
    return err;
}

int file_util::ReadSmallFile::readToBuffer(int* size)
{
    int err = err_;
    if (fd_ >= 0) {
        ssize_t n = ::pread(fd_, buffer_, sizeof(buffer_) - 1, 0);
        if (n >= 0) {
            if (size) {
                *size = static_cast<int>(n);
            }
            buffer_[n] = '\0';
        } else {
            err = errno;
        }
    }
    return err;
}

template int file_util::readFile(std::string_view filename,
    int maxSize,
    std::string* content,
    int64_t*, int64_t*, int64_t*);

template int file_util::ReadSmallFile::readToString(
    int maxSize,
    std::string* content,
    int64_t*, int64_t*, int64_t*);

} // namespace muduo
