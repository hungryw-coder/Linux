#include "TaskQueue.hpp"
#include "Thread.hpp"

#include <iostream>
#include <memory>

using std::unique_ptr;
using wdf::Thread;

class ProducerThread
: public Thread 
{
    void run() override
    {
        std::cout << "producer\n";
    }
};

class ConsumerThread
: public Thread 
{
    void run() override
    {
        std::cout << "consumer\n";
    }
};

int main()
{
    unique_ptr<Thread> producer(new ProducerThread);
    unique_ptr<Thread> consumer(new ConsumerThread);

    producer->start();
    consumer->start();
    
    std::cout << "pc threads going\n";

    producer->join();
    consumer->join();

    std::cout << "ready out\n";
    return 0;
}
