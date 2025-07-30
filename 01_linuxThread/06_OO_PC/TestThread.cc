#include "Thread.h"

#include "unistd.h"
#include "time.h"
#include "stdlib.h"

#include <iostream>
#include <memory>

using std::cout;
using std::endl;
using std::unique_ptr;

class MyThread : public wdf::Thread
{
    void run() override 
    {
        srand(time(nullptr));
        int cnt = 10;
        while (cnt-- > 0) {
            int number = rand() % 100;
            cout << number << " " << endl;
            sleep(1);
        }
    }   
};

int main()
{
    unique_ptr<wdf::Thread> thread(new MyThread());
    thread->start();
    thread->join();
    printf("main thread is exiting. \n"); // 相对于 cout 更高效
    return 0;
}

