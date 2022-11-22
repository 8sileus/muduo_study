#ifndef __MUDUO_LOGSTREAM_H__
#define __MUDUO_LOGSTREAM_H__

#include "Noncopyable.h"
#include "StringPiece.h"

namespace muduo {
namespace detail {

    const int kSmallBuffer = 4000;
    const int kLargeBuffer = 4000 * 1000;

    template <int SIZE>
    class FixedBuffer : Noncopyable {
    public:
        FixedBuffer()
            : cur_(data_)
        {
            setCookie(cookieStart);
        }

        ~FixedBuffer()
        {
            setCookie(cookieEnd);
        }

        void append(const char* buf, size_t len)
        {
            if (static_cast<size_t>(avail()) > len) {
                ::memcpy(cur_, buf, len);
                cur_ += len;
            }
        }

        const char* data() const { return data_; }
        int length() const { return static_cast<int>(cur_ - data_); }
        char* current() { return cur_; }
        int avail() const { return static_cast<int>(end() - cur_); }
        void add(size_t len) { cur_ += len; }
        void reset() { cur_ = data_; }
        void bzero() { ::memset(data_, 0, sizeof(data_)); }

        // for used by GDB
        const char* debugString();
        void setCookie(void (*cookie)()) { cookie_ = cookie; }
        // for used by unit test
        std::string toString() const { return std::string(data_, length()); }
        StringPiece toStringPiece() const { return StringPiece(data_, length()); }

    private:
        const char* end() const { return data_ + sizeof(data_); }
        static void cookieStart();
        static void cookieEnd();

    private:
        void (*cookie_)();
        char data_[SIZE];
        char* cur_;
    };
} // namespace detail
class LogStream : Noncopyable {
    using self = LogStream;

public:
    using Buffer = detail::FixedBuffer<detail::kSmallBuffer>; // Small Buffer Type

    self& operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }
    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);
    self& operator<<(const void*);
    self& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }
    self& operator<<(double);
    self& operator<<(char v)
    {
        buffer_.append(&v, 1);
        return *this;
    }
    self& operator<<(const char* str)
    {
        if (str) {
            buffer_.append(str, strlen(str));
        } else {
            buffer_.append("(null)", 6);
        }
        return *this;
    }
    self& operator<<(const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    self& operator<<(const std::string& v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

    self& operator<<(const StringPiece& v)
    {
        buffer_.append(v.data(), v.size());
        return *this;
    }

    self& operator<<(const Buffer& v)
    {
        *this << v.toStringPiece();
        return *this;
    }

    void append(const char* data, int len) { buffer_.append(data, len); }
    const Buffer& buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    void staticCheck();

    template <typename T>
    void formatInteger(T);

private:
    Buffer buffer_;

    //数字转为字符串的最大长度
    static const int kMaxNumericSize = 48;
};

class Fmt {
public:
    template <typename T>
    Fmt(const char* fmt, T val);

    const char* data() const { return buf_; }
    int length() const { return length_; }

private:
    char buf_[32];
    int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
    s.append(fmt.data(), fmt.length());
    return s;
}

// Format quantity n in SI units (k, M, G, T, P, E).
// The returned string is atmost 5 characters long.
// Requires n >= 0
std::string formatSI(int64_t n);

// Format quantity n in IEC (binary) units (Ki, Mi, Gi, Ti, Pi, Ei).
// The returned string is atmost 6 characters long.
// Requires n >= 0
std::string formatIEC(int64_t n);

} // namespace muduo

#endif //__MUDUO_LOGSTREAM_H__