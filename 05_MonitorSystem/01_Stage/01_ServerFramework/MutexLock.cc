#include "MutexLock.hpp"

namespace wdf
{

MutexLock::MutexLock()
{
    pthread_mutex_init(&m_mutex, nullptr);
}

MutexLock::~MutexLock()
{
    pthread_mutex_destroy(&m_mutex);
}

void MutexLock::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void MutexLock::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

}
