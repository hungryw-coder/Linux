#include "Condition.hpp"
#include "MutexLock.hpp"

namespace wdf
{

Condition::Condition(MutexLock &m)
:m_mutex(m) // 引用成员必须使用初始化列表
{
    pthread_cond_init(&m_cond, nullptr);
}

Condition::~Condition()
{
    pthread_cond_destroy(&m_cond);
}

void Condition::wait()
{
    pthread_cond_wait(&m_cond, m_mutex.getMutexPtr());
}

void Condition::notify()
{
    pthread_cond_signal(&m_cond);
}

void Condition::notifyAll()
{
    pthread_cond_broadcast(&m_cond);
}

}
