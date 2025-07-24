#include <stdio.h>
#include <pthread.h>

// 思考一下 --- 如何达到交替自增
//      - 实现俩个线程函数，并使用互斥锁与条件变量，去实现同步与互斥，来达到交替自增 --- 同步时，需要还需要一个状态变量

#define  MAX_NUMBER 10  //  宏定义不要加分号！！！！

int g_number = 0;       // 全局变量
pthread_mutex_t mutex;  // 使用动态初始化互斥锁
pthread_cond_t cond;    // 使用动态初始化条件变量
int state = 1;          // 1: t1 的回合
                        // 2: t2 的回合

void * start_routin1(void * arg) 
{

    for (int i = 0; i < MAX_NUMBER; ++i) {
        pthread_mutex_lock(&mutex);

        while (state != 1) {
            pthread_cond_wait(&cond, &mutex);
        }

        ++g_number;
        printf("Thread 1, g_number = %d\n", g_number);

        state = 2;
        pthread_cond_signal(&cond); // 通知 t2 执行
                                    
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

void * start_routin2(void * arg) 
{

    for (int i = 0; i < MAX_NUMBER; ++i) {
        pthread_mutex_lock(&mutex);

        while (state != 2) {
            pthread_cond_wait(&cond, &mutex);
        }

        ++g_number;
        printf("Thread 2, g_number = %d\n", g_number);

        state = 1;
        pthread_cond_signal(&cond); // 通知 t1 执行
                                    
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

int main()
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    
    pthread_t t1, t2;
    pthread_create(&t1, NULL, start_routin1, NULL);
    pthread_create(&t2, NULL, start_routin2, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&mutex); // 避免资源泄露
    pthread_cond_destroy(&cond);

    printf("Final counter value: %d (Expected: %d)\n", g_number, 2 * MAX_NUMBER);

    return 0;
}
