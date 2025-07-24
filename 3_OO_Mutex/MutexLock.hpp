#ifndef MUTEXLOCK_HPP
#define MUTEXLOCK_HPP

#include "../2_OO_Thread/Noncopyable.hpp"

#include <pthread.h>

namespace wdf 
{

// 继承 wdf 中的工具类 -- Noncopyable，让 MutexLock 类隐式调用 Noncopyable 的拷贝复制函数(被删除，禁用了)；
//      - 此时 MutexLock 在没有显式定义拷贝复制函数的情况下，会默认隐式调用父类中的拷贝复制函数，因为父类中删除了拷贝复制函数，故编译器报错
//      - 如果 MutexLock 显式定义了拷贝复制函数，编译器则不会隐式调用父类中的拷贝复制函数，会调用显式定义的拷贝复制函数
//
// 这样操作原因是：互斥锁与条件变量是系统资源，不允许拷贝赋值
class MutexLock : Noncopyable
{
public:
    // 构造函数，初始化一个互斥锁
    MutexLock();
    // 析构函数，销毁一个互斥锁
    ~MutexLock();

    // 加锁
    void lock();
    // 解锁
    void unlock();
    
    // 获取底层 pthread_mutex_t*（供 Condition 类使用）
    pthread_mutex_t * getMutexPtr() { return &m_mutex; }

private:
    // 实际的 pthread 互斥锁
    pthread_mutex_t m_mutex;
};

class MutexLockGuard 
{
public:
    MutexLockGuard(MutexLock & m)
    :m_mutex_guard(m)
    {
        m_mutex_guard.lock();
    }

    ~MutexLockGuard()
    {
        m_mutex_guard.unlock();
    }
private:
    MutexLock & m_mutex_guard;
};

}

#endif

