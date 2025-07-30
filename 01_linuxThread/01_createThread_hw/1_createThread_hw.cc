#include <my_header.h>
#include <pthread.h>

int g_number = 1024; // 全局变量位于全局静态区，被主线程与子线程所共享

void * start_routine1(void * arg) 
{
    printf("sub thread id1 = %ld\n", pthread_self());
    
    int *p = (int*)malloc(sizeof(int));
    *p = 888;

    pthread_exit((void *) p);
}

void * start_routine2(void * arg) 
{
    printf("sub thread id2 = %ld\n", pthread_self());
    
    int *p = (int*)malloc(sizeof(int));
    *p = 666;
    
    int *pVal = (int*)arg;
    printf("g_number = %d\n", *pVal);

    pthread_exit(p);
}

void * start_routine3(void * arg) 
{
    printf("sub thread id3 = %ld\n", pthread_self());

    int *p = (int*)arg; 
    printf("main's *p = *arg = %d\n", *p);
    *p = 222;

    return NULL;
}

int main()
{                                  
    pthread_t pthid1 = 0;
    pthread_t pthid2 = 0;
    pthread_t pthid3 = 0;

#if 0
           int pthread_create(pthread_t *restrict thread,           // 该函数执行结束后，会修改线程ID
                          const pthread_attr_t *restrict attr,      // 线程属性，一般直接设为 NULL
                          void *(*start_routine)(void *),           // 函数指针 ---- 线程入口函数
                          void *restrict arg);                      // 给线程入口函数传递的参数

            int pthread_join(pthread_t thread, void **retval);      // 线程ID；线程函数的返回值

            void pthread_exit(void *retval);                        // 线程函数的退出函数，退出时返回 retval
#endif
    
    int *p = (int*)malloc(sizeof(int));
    *p = 111;

    pthread_create(&pthid1, NULL, start_routine1, NULL); 
    pthread_create(&pthid2, NULL, start_routine2, &g_number);
    pthread_create(&pthid3, NULL, start_routine3, p);

    printf("main: mian id = %ld\n", pthread_self());
    
    void *ret1 = NULL;
    pthread_join(pthid1, &ret1);            
    // printf("ret1 = %d", *ret1);          // error
    printf("ret1 address = %p\n", ret1);    // bingo
    
    int *p1 = (int *)ret1;
    printf("ret1(*p1) = %d , p1 address = %p\n", *p1, p1);  // bingo
    free(p1);                                               // 释放由子线程申请的堆空间
    printf("p1 = %p\n", p1);                                // 释放空间成功，但 p1 的地址值依然存在
    p1 = NULL;                                              // 安全释放
    // printf("*p1 = %d\n", *p1); // error 不仅释放了空间，还将 p1 的地址值置NULL，此时打印 p1 的地址为段错误

    printf("\n==============================\n\n");

    void *ret2 = NULL;
    pthread_join(pthid2, &ret2);
    printf("ret2 address = %p\n", ret2);

    int *p2 = (int *)ret2;
    printf("ret1(*p2) = %d , p2 address = %p\n", *p2, p2);  
    free(p2);                                               
    printf("p2 = %p\n", p2);                                
    p1 = NULL;                                              
    printf("\n==============================\n\n");

    pthread_join(pthid3, NULL);
    printf("*p = %d\n", *p);      // 子线程中修改了主线程中的局部变量 

    printf("main thread exit. \n");
    return 0;
}

