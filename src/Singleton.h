#ifndef __MUDUO_SINGLETON_H__
#define __MUDUO_SINGLETON_H__

#include "Noncopyable.h"

namespace muduo {

template <typename T>
class Singleton {
public:
    static T* instance()
    {
        static T v;
        return &v;
    }
};

} // namespace muduo

#endif //__MUDUO_SINGLETON_H__