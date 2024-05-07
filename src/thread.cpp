#include "thread.h"
#include "qlc_log.h"
namespace qlc{
static thread_local Thread* t_thread = nullptr;//每个线程的这个值是不一样的
static thread_local std::string t_thread_name = "UNKNOW";
static qlc::Logger::ptr g_logger = QLC_LOG_NAME("system");


void Thread::SetName(const std::string& name) {
    if(name.empty()) {
        return;
    }
    if(t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

const std::string& Thread::GetName() {
    return t_thread_name;
}

Thread* Thread::GetThis() {
    return t_thread;
}

void Thread::join() {
    if(m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if(rt) {
            QLC_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                << " name=" << m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

Thread::Thread(std::function<void()> cb, const std::string& name)
    :m_cb(cb)
    ,m_name(name){
    if(name.empty()) {
        m_name = "UNKNOW";
    }
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if(rt) {
        QLC_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
            << " name=" << name;
        throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();
}
void* Thread::run(void* arg) {
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = qlc::GetThreadId();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

    std::function<void()> cb;
    cb.swap(thread->m_cb);
    thread->m_semaphore.notify();//这样就是如果初始化完成 那线程肯定运行起来了 不然可能出现 函数运行但是类的初始化还没完成
    cb();
    return 0;
}
 

Thread::~Thread() {
    if(m_thread) {
        pthread_detach(m_thread);
    }
}

}