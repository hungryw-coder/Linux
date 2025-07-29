#ifndef TASK_HPP
#define TASK_HPP

namespace wdf
{

class Task
{
public:
    virtual ~Task() {}

    virtual void process() = 0;
};

}

#endif

