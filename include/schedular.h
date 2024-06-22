#ifndef  __QLC_SCHEDULAR_H
#define  __QLC_SCHEDULAR_H
#include <memory>
#include <functional>
#include <iostream>
#include <list>
#include <vector>
#include "fiber.h"
#include "thread.h"
namespace qlc{

class Schedular{
public:
    typedef std::shared_ptr<Schedular> ptr;
    typedef Mutex MutexType;
    Schedular(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    virtual ~Schedular();
    const std::string& getName() const {return m_name;}
    static Schedular* GetThis();//每个线程启用了调度器的话可以获得那个线程的调度器  这样要用到threadlocal 这个修饰了
    void start();//启动
    void stop();
    static Fiber* GetMainFiber();
    //添加调度任务 这个任务可以是协程或者是cb
    template<class FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
        }

        if(need_tickle) {
            tickle();
        }
    }

    //批量添加任务
    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                ++begin;
            }
        }
        if(need_tickle) {
            tickle();
        }
    }

private:
    struct FiberAndThread{  //传入方法时 可以传协程或者回调函数
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;//运行在哪个线程上的线程id
        FiberAndThread(Fiber::ptr f, int thr)
            :fiber(f), thread(thr) {
        }
        FiberAndThread(Fiber::ptr* f, int thr)//传入协程指针
            :thread(thr) {
            fiber.swap(*f);//swap函数忘记了学一遍
        }
        FiberAndThread(std::function<void()> f, int thr)
            :cb(f), thread(thr) {
        }
        FiberAndThread(std::function<void()>* f, int thr)
            :thread(thr) {
            cb.swap(*f);
        }
        FiberAndThread()
            :thread(-1) {
        }
        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };

    template<class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if(ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }

protected:
    virtual void tickle();//每个子类的ticle方法不一样 
    void run();//每个子类的调度方法也可以不一样
    virtual bool stopping();//返回是否可以停止
    virtual void idle();//线程没有任务的时候 执行空闲协程 运行的空闲任务
    void setThis();//设置当前协程调度器
    bool hasIdleThreads() { return m_idleThreadCount > 0;}

private:
    
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    Fiber::ptr m_rootFiber;//使用主线程时候 用来跑run方法的协程
    std::list<FiberAndThread> m_fibers;

protected:
    std::string m_name;
    std::vector<int> m_threadIds;//一个协程下用了几个线程id
    size_t m_threadCount = 0;//线程数量
    std::atomic<size_t>  m_activeThreadCount = {0};
    std::atomic<size_t> m_idleThreadCount = {0};
    bool m_stopping = true;//是否正在停止
    bool m_autoStop = false;//是否自动停止
    int m_rootThread = 0;//使用主线程后 主线程的id

};















}

#endif