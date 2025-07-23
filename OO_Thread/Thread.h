#ifndef THREAD_H
#define THREAD_H

#include "Noncopyable.h"
#include <pthread.h>

namespace wd
{

// 私有继承的特性
//      - 基类的 public 和 protected 成员在派生类中变为 private
//      - 私有继承时，基类的 private 成员对派生类 不可见，无论继承方式如何
//      - 派生类的外部使用者无法访问基类的任何成员（包括 public）
//      - 派生类的派生类也无法访问基类的成员（继承链断裂）
//
// 在我的场景中：
//      - Noncopyable 的构造函数和析构函数是 protected 的，私有继承后仍可在 Thread 内部访问；如果是 private 的，私有继承后不可在 Thread 内部访问
//      - 因为 protected 成员对派生类可见，无论继承方式是 public、protected 还是 private
//
// 私有继承确保 Noncopyable 的细节对 Thread 的使用者不可见，符合封装原则

class Thread : Noncopyable // 等价于 class Thread : private Noncopyable
{
public:
    Thread();
    virtual ~Thread();

    // 启动一个线程
    void start();
    // 等待一个子线程结束运行
    void join();

private:
    // 含有纯虚函数的类，一定是抽象类，不能被实例化 --- 子类必须实现，包含线程实际逻辑
    virtual void run() = 0;
    // 子线程入口函数 --- 加上 static 为了消除类中自带的 this 指针(这是选择静态成员函数作为 子线程入口函数的主要原因) --- 静态线程入口函数（实际调用虚函数run()）
    static void * start_routhine( void * );
    
    /*
     * start_routhine 设为静态的具有原因:
     *      1. C 线程库的限制
     *          - pthread_create 的线程函数签名必须为: void* (*start_routine)(void*)
     *          - 函数必须是普通的 C 函数或 静态成员函数（因为非静态成员函数隐含 this 参数，签名不匹配）
     *      
     *      2. 非静态成员函数的问题
     *          - void* Thread::start_routine(void*); // 错误！实际隐含this参数，签名不符
     *          - C++ 编译器会隐式添加 this 参数，导致函数签名与 pthread_create 需要的 void*(*)(void*) 不匹配，编译失败
     */


    // 继承了工具类 Noncopyable ，无需再手动禁用拷贝复制与赋值
    // Thread( const Thread& ) = delete;
    // Thread& operator=( const Thread& ) = delete;

private:
    // 线程id
    pthread_t m_pthid;
    // 线程当前运行状态
    bool m_isRunning;
};

} // end of namespace wd

#endif // THREAD_H

