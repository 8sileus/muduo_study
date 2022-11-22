#ifndef __MUDUO_FILEUTIL_H__
#define __MUDUO_FILEUTIL_H__

#include "Noncopyable.h"
#include "StringPiece.h"

namespace muduo {
namespace file_util {
    class ReadSmallFile : Noncopyable {
    public:
        ReadSmallFile(StringArg filename);
        ~ReadSmallFile();

        /**
         * @brief return errno
         */
        template <typename String>
        int readToString(
            int maxSize,
            String* content,
            int64_t* fileSize,
            int64_t* modifyTime,
            int64_t* createTime);

        /**
         * @brief return errno
         */
        int readToBuffer(int* size);
        const char* buffer() const { return buffer_; }

        static const int kBufferSize = 64 * 1024;

    private:
        int fd_;
        int err_;
        char buffer_[kBufferSize];
    };

    template <typename String>
    int readFile(
        StringArg filename,
        int maxSize,
        String* content,
        int64_t* fileSize = nullptr,
        int64_t* modifyTime = nullptr,
        int64_t* createTime = nullptr)
    {
        ReadSmallFile file(filename);
        return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
    }

    // not thread safe
    class AppendFile : Noncopyable {
    public:
        explicit AppendFile(StringArg filename);
        ~AppendFile();

        void append(const char* logline, size_t len);
        void flush();
        off_t writtenBytes() const { return writtenBytes_; }

    private:
        size_t write(const char* logline, size_t len);

    private:
        FILE* fp_;
        char buffer_[64 * 1024];
        off_t writtenBytes_;
    };
} // namespace file_util
} // namespace muduo

#endif //__MUDUO_FILEUTIL_H__
