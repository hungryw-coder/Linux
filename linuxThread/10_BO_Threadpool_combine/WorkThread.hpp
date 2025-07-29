#ifndef WORKTHREAD_HPP
#define WORKTHREAD_HPP

#include "Thread.hpp"
#include "Threadpool.hpp"

namespace wdf
{

class WorkThread
: public Thread
{
public:
    WorkThread(Threadpool & tp)
    : _threadpool(tp)
    {

    }
    
    void run() override
    {
        _threadpool.doTask();
    }

private:
    Threadpool & _threadpool;

};

}

#endif

