#ifndef THREAD_H
#define THREAD_H

#include "Noncopyable.hpp"
#include <pthread.h>
#include <functional>

using std::function;


// 类定义前使用命名空间包裹的主要原因：
//      - 避免命名冲突	通过 wd::Thread 区分其他库的同名类
//      - 代码组织与模块化  将相关功能组织在 wdf 命名空间下
namespace wdf
{

using ThreadCallback = function<void()>; 

// 将 Threadpool 内部的 doTask 方法打包成一个函数对象，注册给 Thread 对象
class Thread : Noncopyable          
{
public:
    Thread(ThreadCallback && cb);   // 参数为右值引用，需要绑定右值
    virtual ~Thread();

    void start();                   // 启动一个线程
    void join();                    // 等待一个子线程结束运行

private:
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

private:
    pthread_t       m_pthid;        // 线程id
    bool            m_isRunning;    // 线程当前运行状态-- 主要作用是防止 重复 start 与 join
    ThreadCallback  m_cb;           // 回调函数, 保存的是void类型无参的函数地址
    
};

} 

#endif // THREAD_H

