#include "EventLoop.h"
#include "EventLoopThread.h"
#include "Thread.h"

#include <stdio.h>
#include <unistd.h>

#include <chrono>

using namespace muduo;

void print(EventLoop* p = NULL)
{
    printf("print: pid = %d, tid = %d, loop = %p\n",
        getpid(), current_thread::tid(), p);
}

void quit(EventLoop* p)
{
    print(p);
    p->quit();
}

int main()
{
    print();

    {
        EventLoopThread thr1; // never start
    }

    {
        // dtor calls quit()
        EventLoopThread thr2;
        EventLoop* loop = thr2.startLoop();
        loop->runInLoop(std::bind(print, loop));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    {
        // quit() before dtor
        EventLoopThread thr3;
        EventLoop* loop = thr3.startLoop();
        loop->runInLoop(std::bind(quit, loop));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
