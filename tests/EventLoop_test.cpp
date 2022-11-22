#include "EventLoop.h"
#include "Thread.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;

EventLoop* g_loop;

void callback()
{
    printf("callback(): pid = %d, tid = %d\n", getpid(), current_thread::tid());
    EventLoop anotherLoop;
}

void threadFunc()
{
    printf("threadFunc(): pid = %d, tid = %d\n", getpid(), current_thread::tid());

    assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
    EventLoop loop;
    assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
    loop.runAfter(1.0, callback);
    loop.loop();
}

int main()
{
    printf("main(): pid = %d, tid = %d\n", getpid(), current_thread::tid());

    assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
    EventLoop loop;
    assert(EventLoop::getEventLoopOfCurrentThread() == &loop);

    Thread thread(threadFunc);
    thread.start();

    loop.loop();
}
