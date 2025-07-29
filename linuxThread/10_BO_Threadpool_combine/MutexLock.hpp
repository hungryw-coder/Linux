#ifndef MUTEXLOCK_HPP
#define MUTEXLOCK_HPP

#include "Noncopyable.hpp"
#include <pthread.h>

namespace wdf
{

class MutexLock
: Noncopyable
{
public:
    MutexLock()
    {
        pthread_mutex_init(&_mutex, nullptr);
    }

    ~MutexLock()
    {
        pthread_mutex_destroy(&_mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&_mutex);
    }
    
    pthread_mutex_t * getMutexLockPtr()
    {
        return &_mutex;
    }

private:
    pthread_mutex_t _mutex;

};


// 自动释放锁 -- RAII
class MutexLockGuard
{
public:
    MutexLockGuard(MutexLock & ml)
    : _lock_guard(ml)
    {
        _lock_guard.lock();
    }

    ~MutexLockGuard()
    {
        _lock_guard.unlock();
    }
        
private:
    MutexLock & _lock_guard;
};

}

#endif

