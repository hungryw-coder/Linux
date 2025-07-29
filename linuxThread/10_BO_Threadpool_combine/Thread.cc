#include "Thread.hpp"
#include <stdio.h>
#include <string.h>

namespace wdf
{

Thread::Thread()
: _tid(0)
{

}

Thread::~Thread()
{
    
}

void Thread::start()
{   
    if (_tid == 0) {
        int ret = pthread_create(&_tid, NULL, start_routine, this);
        if (ret != 0) {
            fprintf(stderr, "pthread_create failed: %s\n", strerror(ret));
        }
    }
}

void Thread::join()
{
    if (_tid != 0) {
        pthread_join(_tid, nullptr);
        _tid = 0;
    }
}

void  * Thread::start_routine(void * arg)
{
    // 静态函数无法访问非静态成员（缺少this指针），其次静态成员的初始化需在类外定义
    Thread * thread = static_cast<Thread *>(arg);
    if (thread) {
        thread->run();
    }
    return nullptr;
}

}
