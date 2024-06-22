#include "schedular.h"
#include "qlc_log.h"
#include "macro.h"
namespace qlc{
static Logger::ptr g_logger=QLC_LOG_NAME("system");
static thread_local Schedular* t_schedular = nullptr;
static thread_local Fiber* t_schedular_fiber = nullptr;


Schedular::Schedular(size_t threads, bool use_caller, const std::string &name):m_name(name)
{
    if(use_caller){
        QLC_ASSERT(threads > 0);
        Fiber::GetThis();
        --threads;//当前线程不算
        QLC_ASSERT(GetThis() == nullptr);
        t_schedular=this;
        m_rootFiber.reset(new Fiber(std::bind(&Schedular::run,this),0,true));
        qlc::Thread::SetName(m_name);
        t_schedular_fiber=m_rootFiber.get();//对于主线程来说 调度协程就是这个协程
        m_rootThread=qlc::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    }
    else{
        // t_schedular=this;
        // std::cout<<"构造函数： "<<t_schedular<<std::endl;
        m_rootThread=-1;
    }
    m_threadCount=threads;
}

Schedular::~Schedular()
{
    QLC_ASSERT(m_stopping);
    if(GetThis()==this){
        t_schedular=nullptr;
    }
}

Schedular *Schedular::GetThis()
{
    return t_schedular;
}
Fiber *Schedular::GetMainFiber()
{
    return t_schedular_fiber;
}
void Schedular::start()
{

    MutexType::Lock lock(m_mutex);
    if(!m_stopping) return; //因为一开始是true
    m_stopping=false;
    QLC_ASSERT(m_threads.empty());
    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i){
        m_threads[i].reset(new Thread(std::bind(&Schedular::run,this),m_name+"_"+std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();//如果不unlock的话 会一直死锁

}
void Schedular::stop()
{
    m_autoStop=true;
    if(m_rootFiber && m_threadCount==0 && (m_rootFiber->getState()==Fiber::TERM || m_rootFiber->getState() == Fiber::INIT)){ //相当于只有主线程的时候
        QLC_LOG_INFO(g_logger)<< this <<"stopped";
        m_stopping=true;
        if(stopping()){
            return;
        }
    }
    //到这里说明还有一些没有执行完  有其他的线程
    //不然上面就直接return了
    if(m_rootThread != -1) { //用主线程
        QLC_ASSERT(GetThis() == this);
    } else {
        //  std::cout<<"析构函数： "<<this<<"   "<<GetThis()<<std::endl;
        QLC_ASSERT(GetThis() != this);
    }
    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    } 
    if(m_rootFiber) {
        tickle();
    }
    if(m_rootFiber){
        if(!stopping()) {//如果还有活跃的线程数量  也就是说其他的协程还没跑完 主协程也开始运行
            m_rootFiber->call();
        }
    }
    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);//没太懂这一步是要干嘛
    }
    for(auto& i : thrs) {
        i->join();
    }
}
void Schedular::run()
{
    setThis();
    std::cout<<"线程函数:  "<<t_schedular<<std::endl;
    if(qlc::GetThreadId()!=m_rootThread){ //不是主线程的协程跑的
        t_schedular_fiber=Fiber::GetThis().get();
    }
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Schedular::idle, this)));
    Fiber::ptr cb_fiber;
    FiberAndThread ft;
    while(true){
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            //在这段作用域里面 取出一个不为空的协程
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) {
                if(it->thread != -1 && it->thread != qlc::GetThreadId()) { //不是所有线程都可以跑的话也不是当前要跑的线程就继续
                //  QLC_LOG_INFO(g_logger) << "进来这里了: "<<it->thread;
                    ++it;
                    tickle_me = true;
                    continue;
                }
                QLC_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }
                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;//活跃的线程数量加1  应该说需要的活跃的线程数量
                is_active=true;
                break;
            }
            
            tickle_me |= (it != m_fibers.end());//表示取出了一个可用的协程
        }
        if(tickle_me){
            // QLC_LOG_INFO(g_logger) << "tickle_me的值为"<<tickle_me<<"  "<<"m_fiber的数量是："<<m_fibers.size()<<" "<<is_active;
            // QLC_LOG_INFO(g_logger) << "我正在通知";
            tickle();
        }
        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM&& ft.fiber->getState() != Fiber::EXCEPT)){
            //协程可以用 在这个线程里面开始跑协程
            ft.fiber->swapIn();
            //暂时跑出来了
            --m_activeThreadCount;
             if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);//重新放进队列里
             }else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        }else if(ft.cb){
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        }else{
            
            if(is_active) {
                --m_activeThreadCount; //说明是判断有问题了
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM) {
                QLC_LOG_INFO(g_logger) << "idle fiber term";
                break;//说明空闲的协程都跑完了  也说明++m_idleThreadCount;已经要停止了
            }
            ++m_idleThreadCount; //这个是说明有哪几个线程正在运行空闲协程
            idle_fiber->swapIn();//这个空闲协程里面跑的是返回hold状态 
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD; //感觉是为了加双重保险
            }

        }
    }
}

void Schedular::setThis() {
    t_schedular = this;
}


void Schedular::tickle() {
    QLC_LOG_INFO(g_logger) << "tickle";
}

void Schedular::idle() {
    QLC_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        qlc::Fiber::YieldToHold();
    }
    
}
bool Schedular::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}
}
