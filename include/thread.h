#ifndef __QLC_THREAD_H
#define __QLC_THREAD_H
#include "qlc_log.h"
#include <pthread.h>
#include <memory>
#include <functional>
namespace qlc{
class Thread{
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();
    pid_t getId() const { return m_id;}
    const std::string& getName() const { return m_name;}//获取当前线程类的名称
    void join();
    static Thread* GetThis();
    static const std::string& GetName();//获取当前线程的名称
    static void SetName(const std::string& name);
private:
    Thread(const Thread&)=delete;
    Thread(const Thread&&)=delete;
    Thread operator=(const Thread&)=delete;
    static void* run(void* arg);//在使用 pthread 线程库时，线程函数必须是静态的或是全局函数，
    //而不能是类的成员函数。这是因为线程函数需要与 pthread 线程库的 C 接口兼容。
private:
    pid_t m_id=-1;//线程id
    pthread_t m_thread=0;
    //线程执行函数
    std::function<void()> m_cb;
    std::string m_name;//线程名称


};

}



#endif