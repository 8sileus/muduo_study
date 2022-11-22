#ifndef __MUDUO_NONCOPYABLE_H__
#define __MUDUO_NONCOPYABLE_H__

namespace muduo {

/**
 * @brief 不可拷贝类
 */
class Noncopyable {
public:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};

} // namespace muduo

#endif //__MUDUO_NONCOPYABLE_H__