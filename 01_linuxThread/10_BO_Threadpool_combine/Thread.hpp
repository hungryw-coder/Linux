#ifndef THREAD_HPP
#define THREAD_HPP

#include "Noncopyable.hpp"
#include <pthread.h>

namespace wdf
{

class Thread
: Noncopyable
{
public:
    Thread();
    virtual ~Thread();
    void start();
    void join();
    virtual void run() = 0;

private:
    static void * start_routine(void * agr);

private:
    pthread_t   _tid;

};


}

#endif

