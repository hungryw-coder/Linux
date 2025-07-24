#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// Q2: mutex 和 cond 可以是局部变量吗？
// A2： 可以，但不要这样操作（徒增烦恼）---- 应该避免普通局部变量，除非能确保所有线程在变量销毁前完成
//      结论：
//          - 如果 mutex 和 cond 需要跨多个函数使用，全局变量或 static 局部变量更合适
//          - 全局变量	        简单场景，所有线程共享	    程序结束时销毁	    线程安全
//          - static 局部变量	函数内初始化，全局使用	    程序结束时销毁	    线程安全
//
//          - 如果需要动态创建（如线程池），动态分配（堆内存） 是更好的选择
//          - 动态分配（堆内存）	需要运行时动态创建	手动 free	线程安全（需确保释放时机）
//
//          - 避免普通局部变量！！！

pthread_mutex_t mutex;
pthread_cond_t cond; 
int state = 0; // 0: A 未打印
               // 1：B打印
               // 2：C打印

// Q1: 条件变量加锁的目的
// A1：锁保护共享数据，而条件变量用于线程间的同步。两者结合才能正确实现复杂的线程协作。


#if 0

// 以下写是错误的！！
//      1. pthread_cond_wait(&cond, &mutex) 应该在检查某个条件时使用 --- 例如使用2.中的共享的 state 来同步线程
//          - 由于没有状态变量，线程的执行顺序无法保证，可能导致
//              - t2 先获取 mutex 并打印 B
//              - t2 调用 pthread_cond_signal(&cond)，但此时 t1 还未调用 pthread_cond_wait
//              - 信号被丢弃（因为没有线程在等待）
//              - t2 释放 mutex
//              - t1 获取 mutex，打印 A，然后调用 pthread_cond_wait，但此时 cond 已经错过信号，导致 t1 永远阻塞
//
//      2. 条件变量通常需要搭配一个共享的 state 变量（如 int state）来同步线程


void * start_routine1(void * arg)
{
    // printf("thread1 id = %ld\n", pthread_self());
    
    pthread_mutex_lock(&mutex);
    
    printf("A ");

    pthread_cond_wait(&cond, &mutex);

    printf("C\n");

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void * start_routine2(void * arg)
{
    // printf("thread2 id = %ld\n", pthread_self());

    pthread_mutex_lock(&mutex);

    printf("B ");

    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&mutex);

    return NULL;
}

#endif

void * start_routine1(void * arg)
{
    // printf("thread1 id = %ld\n", pthread_self());
    
    pthread_mutex_lock(&mutex);
    
    printf("A ");   // 首先打印 A
    state = 1;      // 修改共享的状态变量 -- 配合 pthread_cond_wait 
    pthread_cond_signal(&cond); // 通知 pthid2 打印 B

    // 等待 B 的打印
    while (state != 2) {
                                          // 如果在俩个线程并发执行时，线程2 很快将 state = 2，就不会进入该循环 --- 直接打印 C ，因为 B 已经打印完了
                                          //    - 对于这种情况，线程2 中释放的信号（通知线程1打印 C ）将会被丢弃 --- 因为没有线程在等待
                                          //
        pthread_cond_wait(&cond, &mutex); // 并发执行时，走这里代表 state 还不等于2 --- 先释放锁，再阻塞在此处等待信号
                                          // 等到信号后，先加锁，再结束执行；此时 state = 2，跳出循环，执行 C 得打印
    }

    printf("C\n");

    pthread_mutex_unlock(&mutex);
    return NULL;
}

void * start_routine2(void * arg)
{
    // printf("thread2 id = %ld\n", pthread_self());

    pthread_mutex_lock(&mutex);
    
    // 等待 A 打印完成
    while (state != 1) {
        // pthread_cond_wait 的实现细节：
        //      1. 执行到 pthread_cond_wait 时，释放锁并阻塞在这个函数中 --- 阻塞前释放锁，然后阻塞在此处等待信号的到来
        //      2. 等到信号后，立刻加锁，随后 pthread_cond_wait 执行结束 --- 如果加锁未成功，则继续阻塞在此处，直至加锁成功后执行结束
                                          
                                          // 同理线程1的思路
                                          //
        pthread_cond_wait(&cond, &mutex); // 在 A 打印完之前，如果先走的 pthid2 的话；在这里，先释放锁，再阻塞至此处，等待 pthid1 的信号
                                          // A 打印完，发送信号，pthid2 收到信号后，在先加锁的操作后，执行结束
    }

    printf("B "); // 其次打印 B
    state = 2;
    pthread_cond_signal(&cond); // 通知 pthid1 打印 C

    pthread_mutex_unlock(&mutex);
    return NULL;
}
int main()
{
    // init mutex
    pthread_mutex_init(&mutex, NULL);
    // init condition variable 
    pthread_cond_init(&cond, NULL);
    // thread id
    pthread_t pthid1 = 0, pthid2 = 0;
    // create thread
    pthread_create(&pthid1, NULL, start_routine1, NULL);
    pthread_create(&pthid2, NULL, start_routine2, NULL);

    // LWP
    // printf("main thread id = %ld\n", pthread_self());
    
    pthread_join(pthid1, NULL);
    pthread_join(pthid2, NULL);
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}

