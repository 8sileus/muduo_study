#include "CurrentThread.h"
#include "Thread.h"

#include <chrono>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

using namespace muduo;
using namespace muduo::current_thread;

void f()
{
    int i = 0;
    while (i < 10) {
        printf("tid = %d, t_name = %s,val = %d\n", tid(), name(), ++i);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    std::vector<Thread> threads;
    for (int i = 1; i <= 10; ++i) {
        Thread t(f);
        t.start();
        threads.push_back(std::move(t));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    printf("pid=%d, tid=%d\n", ::getpid(), tid());
    printf("number of created threads %d\n", Thread::numCreated());
    return 0;
}
