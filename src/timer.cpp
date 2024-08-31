#include "timer.h"
#include "util.h"
#include "macro.h"


namespace qlc{

bool Timer::Comparator::operator()(const Timer::ptr &lf,const Timer::ptr &rg) const {
    if(!lf&&!rg){
        return false;
    }
    if(!lf){
        return true;
    }
    if(!rg){
        return false;
    }
    if(lf->m_next<rg->m_next){
        return true;
    }
    if(lf->m_next>rg->m_next){
        return false;
    } 
    return lf.get()<rg.get();//比较地址了
}

bool Timer::cancel()
{
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(m_cb){
        m_cb=nullptr;
        auto it=m_manager->m_timers.find(shared_from_this());
        if(it!=m_manager->m_timers.end())
        {
            m_manager->m_timers.erase(it);
            return true;
        }
    }
    return false;
}

Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager *manager):m_recurring(recurring),
            m_ms(ms),m_cb(cb),m_manager(manager){
    m_next=ms+qlc::GetCurrentMS();
}
Timer::Timer(uint64_t next)
    :m_next(next) {
}
bool Timer::refresh() {//得先删除 再插入 因为要重新进行定时器排序
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb) {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);
    m_next = qlc::GetCurrentMS() + m_ms;
    m_manager->m_timers.insert(shared_from_this());
    return true;
}

bool Timer::reset(uint64_t ms, bool from_now)
{
    if(ms==m_ms&&!from_now){
        return true;
    }
    //说明肯定有一个条件满足
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb){
        return false;//如果没有回调函数直接返回
    }
    auto it=m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);
    uint64_t start=0;
    if(from_now) {
        start = qlc::GetCurrentMS();
    } else {
        start = m_next - m_ms;
    }
    m_ms=ms;                                                                                           
    m_next=start+m_ms;
    m_manager->addTimer(shared_from_this(),lock);
    return true;
}

TimerManager::TimerManager()
{
    m_previouseTime=qlc::GetCurrentMS();

}

TimerManager::~TimerManager()
{

}
//这段代码使用了 std::weak_ptr 和 std::shared_ptr 来处理共享资源的生命周期。std::weak_ptr 是一种弱引用，
//它可以监视 std::shared_ptr 指向的对象，但不会增加该对象的引用计数。通过调用 lock() 函数，可以尝试将 std::weak_ptr 转换为 
//std::shared_ptr，如果转换成功，就说明资源仍然存在，可以安全地使用。
//void  可以指向任意对象类型
static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
    std::shared_ptr<void> tmp = weak_cond.lock();// weak_cond 转换为 shared_ptr 类型
    if(tmp) {
        cb();
    }
}


Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring)
{
    Timer::ptr timer(new Timer(ms,cb,recurring,this));
    RWMutexType::WriteLock lock(m_mutex);
    addTimer(timer,lock);
    return timer;
}


//增加条件定时器
Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring)
{
    return addTimer(ms,std::bind(&OnTimer,weak_cond,cb),recurring);
}

uint64_t TimerManager::getNextTimer()
{
    //读取距离最近的定时器的时间
    RWMutexType::ReadLock lock(m_mutex);
    m_tickled=false;//
    if(m_timers.empty()){
        return ~0ull;
    }
    const Timer::ptr& next = *m_timers.begin();//const和引用绑定了
    uint64_t now_ms = qlc::GetCurrentMS();
    if(now_ms >= next->m_next) {
        return 0;
    } else {
        return next->m_next - now_ms;
    }
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>> &cbs)
{
    uint64_t now_ms=qlc::GetCurrentMS();
    std::vector<Timer::ptr> expired;
    {
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty())
        {
            return;
        }
    }
    RWMutexType::WriteLock lock(m_mutex);
    bool rollover = detectClockRollover(now_ms);//检测时间是否发生往后移
    if(!rollover&&(*m_timers.begin())->m_next>now_ms){
        return ;
    }
    Timer::ptr now_timer(new Timer(now_ms));//以现在的时间创建一个定时器
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);//在这种情况下，m_timers.lower_bound(now_timer) 返回的迭代器可以用于定位 m_timers 容器中大于等于 now_timer 的元素。
    while(it != m_timers.end() && (*it)->m_next == now_ms) {//可能有相同的
        ++it;
    }
    //将 m_timers 容器中的元素范围 [m_timers.begin(), it) 插入到 expired 容器的起始位置。
    expired.insert(expired.begin(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it);
    cbs.reserve(expired.size());
    for(auto &it:expired){
        cbs.push_back(it->m_cb);
        //如果有循环的话
        if(it->m_recurring){
            it->m_next=it->m_ms+now_ms;
            m_timers.insert(it);
        }else{
            it->m_cb=nullptr;
        }
    }

}

bool TimerManager::hasTimer()
{
    RWMutexType::ReadLock lock(m_mutex);
    return !m_timers.empty();
}

void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock &lock)
{ 

    auto it=m_timers.insert(val).first;//看看是不是插进去是第一个first：返回插入位置的迭代器或指向已存在元素的迭代器
    bool on_first=(it==m_timers.begin())&&!m_tickled;
    if(on_first){
        m_tickled=true;
    }
    lock.unlock();
    if(on_first){
        onTimerInsertedAtFront();
    }
}

bool TimerManager::detectClockRollover(uint64_t now_ms)//检验时间有没有往前移动
{
    bool rover=false;
    if(now_ms>m_previouseTime&&now_ms>(m_previouseTime+60*60*1000)){//如果说现在的时间远远大于之前的时间那时间是有我往后挑
        rover=true;
    }
    m_previouseTime=now_ms;
    return rover;
}
}