#ifndef CONDITION_HPP
#define CONDITION_HPP

// #include "../3_OO_Mutex/MutexLock.hpp" // 该头文件可以放到实现文件中去

#include "../2_OO_Thread/Noncopyable.hpp"

#include <pthread.h>

namespace wdf 
{

// 类的向前声明 --- 减少了头文件的依赖
class MutexLock; 

class Condition : Noncopyable
{
public:
    // 为什么 Condition 构造函数要传入 MutexLock&
    //      - POSIX 的 pthread_cond_wait 必须配合一个 pthread_mutex_t 使用
    //          - pthread_cond_wait(&cond, &mutex);  // 必须传入 mutex
    //      - 因此，Condition 类必须知道它要操作哪个 MutexLock
    
    // 为什么用引用（MutexLock&）而不是指针（MutexLock*）
    //      - 引用 (&) 不能为 nullptr，而指针 (*) 可能为空; 如果传入指针，调用时可能传 nullptr，导致未定义行为（UB）
    //      - 引用语义更清晰，表示 Condition 必须 绑定一个 MutexLock，不能无关联

    // 为什么不能直接传值（MutexLock）
    //      - MutexLock 不可拷贝
    //          - pthread_mutex_t 是 不可拷贝 的（拷贝会导致两个对象管理同一个锁，行为未定义）
    
    Condition(MutexLock &);
    ~Condition();

    // 等待条件变量
    void wait();
    // 唤醒一个等待线程
    void notify();
    // 唤醒所有等待线程
    void notifyAll();

private:
    // 实际的 pthread 条件变量
    pthread_cond_t m_cond;
    // 关联的 MutexLock（必须引用，不能拷贝）--- m_mutex 是一个引用成员（MutexLock&），而引用必须在构造时初始化 --- 必须用初始化列表
    MutexLock& m_mutex;

    //  Condition 类存储 MutexLock& 的原因 与 构造函数为什么传参 MutexLock& 是同样的原因
};

}

#endif

