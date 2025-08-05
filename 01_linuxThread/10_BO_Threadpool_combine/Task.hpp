#ifndef TASK_HPP
#define TASK_HPP

#include <functional>

namespace wdf
{

using Task = std::function<void()>; // 基于对象

}

#endif

