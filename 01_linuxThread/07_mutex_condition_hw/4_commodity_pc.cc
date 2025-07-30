#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <queue>

using std::queue;

#define MAX_ITEMS 10       // 队列最大容量
#define INIT_ITEMS 8       // 初始商品数
#define NUM_PRODUCERS 3    // 生产者线程数
#define NUM_CONSUMERS 2    // 消费者线程数

pthread_mutex_t mutex;                  // 互斥锁
pthread_cond_t cond_not_full;           // 队列未满条件
pthread_cond_t cond_not_empty;          // 队列非空条件
queue<int> que;                         // 全局队列

void * producer(void * arg)
{
    int producer_id = *(int*)arg;
    while (1) {
        pthread_mutex_lock(&mutex);

        // 如果队列已满，等待消费者唤醒
        while (que.size() >= MAX_ITEMS) {
            printf("生产者%d: 队列已满(当前大小:%zu)，等待...\n", 
                  producer_id, que.size());
            pthread_cond_wait(&cond_not_full, &mutex);
        }

        // 生产商品（随机数作为ID）
        int commodity_id = rand() % 1000;
        que.push(commodity_id);
        
        printf("生产者%d: 生产商品%d (队列大小:%zu)\n", 
              producer_id, commodity_id, que.size());

        // 通知消费者队列非空
        pthread_cond_signal(&cond_not_empty);
        pthread_mutex_unlock(&mutex);

        sleep(3);  // 每3秒生产一次
    }
    return NULL;
}

void * consumer(void * arg)
{
    int consumer_id = *(int*)arg;
    sleep(5);  // 先睡眠5秒

    while (1) {
        pthread_mutex_lock(&mutex);

        // 如果队列为空，等待生产者唤醒
        while (que.empty()) {
            printf("消费者%d: 队列为空，等待...\n", consumer_id);
            pthread_cond_wait(&cond_not_empty, &mutex);
        }

        // 消费商品
        int commodity_id = que.front();
        que.pop();
        
        printf("消费者%d: 消费商品%d (队列大小:%zu)\n", 
              consumer_id, commodity_id, que.size());

        // 通知生产者队列未满
        pthread_cond_signal(&cond_not_full);
        pthread_mutex_unlock(&mutex);

        sleep(1);  // 每1秒消费一次
    }
    return NULL;
}

int main()
{ 
    srand(time(nullptr));

    // 初始化互斥锁和条件变量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_not_full, NULL);
    pthread_cond_init(&cond_not_empty, NULL);

    // 初始化队列
    for (int i = 0; i < INIT_ITEMS; ++i) {
        int commodity_id = rand() % 1000;
        que.push(commodity_id);
    }
    printf("初始化队列完成, 队列大小为 %zu\n", que.size());

    // 线程id
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    
    // 传给线程函数 -- 线程号   
    int producer_ids[NUM_PRODUCERS] = {1, 2, 3};
    int consumer_ids[NUM_CONSUMERS] = {1, 2};

    // 创建生产者线程
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }

    // 创建消费者线程
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }
    
    // 等待所有子线程执行完毕
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        pthread_join(consumers[i], NULL);
    }

    // 销毁锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_not_full);
    pthread_cond_destroy(&cond_not_empty);
    return 0;
}
