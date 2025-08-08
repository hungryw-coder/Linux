#include "Thread.hpp"

#include <stdio.h>
#include <string.h>

namespace wdf
{

Thread::Thread(ThreadCallback && cb)
: m_pthid(0)
, m_isRunning(false)
, m_cb(std::move(cb))
{

}

Thread::~Thread()
{

}

void Thread::start() 
{
    if (!m_isRunning) {
        int ret = pthread_create(&m_pthid, nullptr, start_routhine, this);
        if (ret != 0) {
            fprintf(stderr, "%s", strerror(ret));
        }
        m_isRunning = true;
    }
}

// 此函数为静态成员函数，没有 this 指针 --- 无法直接获取 Thread 对象 --- 但可以通过线程入口函数的参数来传递
void * Thread::start_routhine(void * arg) 
{
    // 线程入口函数内需要调用m_cb()方法 --- 线程将执行的任务 --- 所以需要this指针
    Thread * pthread = static_cast<Thread *>(arg);
    if (pthread) {
        pthread->m_cb(); 
    } 
    return nullptr;
}

void Thread::join() 
{
    if (m_isRunning) {
        pthread_join(m_pthid, nullptr); // 等待子线程结束，再往下执行
        m_isRunning = false;
    }
}

} 
