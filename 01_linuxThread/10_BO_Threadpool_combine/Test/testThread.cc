#include "Thread.hpp"

#include <unistd.h>
#include <iostream>
#include <memory>

class MyThread 
: public wdf::Thread // 必须公有继承Thread，因为 new MyThread 时，需要调用将派生类指针隐式转换为基类指针
                     // 1. 当私有继承时：
                     //     私有继承意味着基类的公有和保护成员在派生类中都变成私有的
                     //     外部代码无法将派生类指针隐式转换为基类指针
{
    void run() override
    {
        std::cout << "啊哈哈\n";
    }
};

int main()
{
    std::unique_ptr<wdf::Thread> thread(new MyThread);
    thread->start();
    
    sleep(3);

    thread->join();

    return 0;
}
