#ifndef __QLC_MUTEX_H
#define __QLC_MUTEX_H

#include <pthread.h>
#include <memory>
#include <stdint.h>
#include <functional>
#include <semaphore.h>
#include <atomic>
#include <string>
#include <iostream>
#include "noncopyable.h"
//把锁和线程的东西自己封装一层
namespace qlc{
class Semaphore :public Noncopyable{
public:
    Semaphore(uint32_t count=0);
    ~Semaphore();
    void wait();
    void notify();
private:
    sem_t m_semaphore;
};

//接下来就是封装锁了
//使用rall机制 
template<class T>
struct ScopedLockImpl{
public:
    ScopedLockImpl(T& mutex):m_mutex(mutex){
        m_mutex.lock();
        m_locked=true;
    }
    ~ScopedLockImpl(){
        unlock();
    }
    //其实下面俩函数没啥必要 因为基本不会用下面俩函数
    void lock(){
        if(!m_locked){
            m_mutex.lock();
        }
    }
    void unlock(){

        // std::cout<<"锁的析构"<<typeid(T).name()<<std::endl;
        if(m_locked){
            m_mutex.unlock();
            m_locked=false;
        }
       
    }
private:
    T& m_mutex;//引用别名
    bool m_locked;
};

//接下来是实现一些锁就可以了
//首先实现一个互斥锁
class Mutex : Noncopyable{
public:
    typedef ScopedLockImpl<Mutex> Lock;
    Mutex(){
        pthread_mutex_init(&m_mutex,nullptr);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }
    void lock(){
        pthread_mutex_lock(&m_mutex);
    }
    void unlock(){
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};

//自旋锁
class Spinlock :public Noncopyable{
public:

    typedef ScopedLockImpl<Spinlock> Lock;

    Spinlock() {
        pthread_spin_init(&m_mutex, 0);
    }

    ~Spinlock() {
        pthread_spin_destroy(&m_mutex);
    }

    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    /// 自旋锁
    pthread_spinlock_t m_mutex;
};
//读写锁的模板类 读锁的模板类

//因为读写锁有俩吧锁 所以得分俩个模板类实现 不能直接用上述的局部锁来锁
template<class T>
struct ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }

    ~ReadScopedLockImpl() {
        unlock();
    }
    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};
template<class T>
struct WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }
    ~WriteScopedLockImpl() {
        unlock();
    }
    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

class RWMutex:public Noncopyable{
public:

    /// 局部读锁
    typedef ReadScopedLockImpl<RWMutex> ReadLock;

    /// 局部写锁
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }
    
    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }
    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }

    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }
    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};
//空锁
class NullMutex :public Noncopyable{
public:
    typedef ScopedLockImpl<NullMutex> Lock;

    NullMutex() {}

    ~NullMutex() {}
    void lock() {}

    void unlock() {}
};
class NullRWMutex :public Noncopyable {
public:
    /// 局部读锁
    typedef ReadScopedLockImpl<NullRWMutex> ReadLock;
    /// 局部写锁
    typedef WriteScopedLockImpl<NullRWMutex> WriteLock;

    NullRWMutex() {}

    ~NullRWMutex() {}
    void rdlock() {}

    void wrlock() {}
    void unlock() {}
};
//CAS原子锁
//cmppxchg(a,b,c);—>if(a==b) a=c;这就是CAS的一种
// clear()：将标志设置为false。
// test_and_set()：设置标志为true，并返回之前的值。
// test_and_set_explicit()：与test_and_set()类似，但可以指定一个内存顺序参数。
// test_and_reset()：设置标志为false，并返回之前的值。
// test_and_reset_explicit()：与test_and_reset()类似，但可以指定一个内存顺序参数。
// wait()：等待标志变为false。
// notify_one()：唤醒等待该标志的一个线程。
// notify_all()：唤醒等待该标志的所有线程。
class CASLock{
public:
    typedef ScopedLockImpl<CASLock> Lock;
    CASLock(){
        m_mutex.clear();
    }
    ~CASLock(){

    }
    void lock(){
        // while(m_mutex.test_and_set(std::memory_order_acquire));
        while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }
    void unlock(){
        std::atomic_flag_clear_explicit(&m_mutex,std::memory_order_release);
    }
private:
    volatile std::atomic_flag m_mutex;

};

}

#endif