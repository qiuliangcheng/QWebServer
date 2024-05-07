#include "mutex.h"
#include "qlc_log.h"
namespace qlc{
Semaphore::Semaphore(uint32_t count)
{
    if(sem_init(&m_semaphore,0,count)){//如果返回的不是0
        throw std::logic_error("信号量初始化错误");
    }
}

Semaphore::~Semaphore()
{
    sem_destroy(&m_semaphore);
}

void Semaphore::wait()
{
    if(sem_wait(&m_semaphore)) {
        throw std::logic_error("信号量等待函数错误");
    }
}

void Semaphore::notify()
{
    //就是给信号量+1 
    if(sem_post(&m_semaphore)){
        throw std::logic_error("信号量唤醒错误");
    }
}


}

