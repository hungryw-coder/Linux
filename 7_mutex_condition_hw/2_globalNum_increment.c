#include <stdio.h>
#include <pthread.h>

// 思考一下 --- 如何达到交替自增
//      - 实现俩个线程函数，并使用互斥锁与条件变量，去实现同步与互斥，来达到交替自增

#define  MAX_NUMBER 10  // 每个线程自增1000万次, 宏定义不要加分号！！！！ --- 10次便于观察

int g_number = 0;       // 全局变量
pthread_mutex_t mutex;  // 使用动态初始化互斥锁

void * start_routin1(void * arg) 
{
    
    for (int i = 0; i < MAX_NUMBER; ++i) {
        pthread_mutex_lock(&mutex);
        ++g_number;
        printf("id = %ld, g_number = %d\n", pthread_self(), g_number);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}


int main()
{
    pthread_mutex_init(&mutex, NULL);
    
    pthread_t t1, t2;
    pthread_create(&t1, NULL, start_routin1, NULL);
    pthread_create(&t2, NULL, start_routin1, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&mutex); // 避免资源泄露
    
    printf("Final counter value: %d (Expected: %d)\n", g_number, 2 * MAX_NUMBER);

    return 0;
}
