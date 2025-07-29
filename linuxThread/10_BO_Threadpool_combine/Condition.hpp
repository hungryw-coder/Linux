#ifndef CONDITION_HPP
#define CONDITION_HPP

#include "MutexLock.hpp"
#include "Noncopyable.hpp"
#include <pthread.h>

namespace wdf
{

class Condition
: Noncopyable
{
public:
    Condition(MutexLock & m)
    :_mutex(m)
    {
        pthread_cond_init(&_cond, nullptr);
    }

    ~Condition()
    {
        pthread_cond_destroy(&_cond);
    }

    void notifyOne()
    {
        pthread_cond_signal(&_cond);
    }
    
    void notifyAll()
    {
        pthread_cond_broadcast(&_cond);
    }

    void wait()
    {
        pthread_cond_wait(&_cond, _mutex.getMutexLockPtr());
    }

private:
    pthread_cond_t  _cond;
    MutexLock   &   _mutex;  
};

}

#endif

