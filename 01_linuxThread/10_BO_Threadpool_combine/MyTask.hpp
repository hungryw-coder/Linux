#ifndef MYTASK_HPP
#define MYTASK_HPP

#include "Task.hpp"
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <cstdio>

namespace wdf
{
// 不继承 Task， MyTask是个具体类
class MyTask 
{
public:
    void process() 
    {
        int numer = rand() % 100;
        printf("producer make a numer >>> %d\n", numer);
        // 慢点展示
        sleep(1);
    }

};

}
#endif
