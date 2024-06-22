#ifndef __QLC_TIMER_H
#define __QLC_TIMER_H
#include <memory>
#include <functional>
#include "thread.h"
#include <set>
#include <vector>

namespace qlc{
class TimerManager;
class Timer :public std::enable_shared_from_this<Timer> {
friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;
    bool cancel();//取消定时器
    bool refresh();//刷新定时器的执行时间
    bool reset(uint64_t ms, bool from_now);//是否从当前时间开始计算
private:
    Timer(uint64_t ms, std::function<void()> cb,
          bool recurring, TimerManager* manager);//构造函数外面不能调用 只能由定时管理器进行调用增加定时器
    Timer(uint64_t next);//传入下一次执行的时间生成定时器
private:
    /// 是否循环定时器
    bool m_recurring = false;
    /// 执行周期
    uint64_t m_ms = 0;
    /// 精确的执行时间
    uint64_t m_next = 0;
    /// 回调函数
    std::function<void()> m_cb;
    /// 定时器管理器
    TimerManager* m_manager = nullptr;
private:
    struct Comparator {
        bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const; //按照执行的时间进行排序
    };
};

class TimerManager{
friend class Timer;
public:
    typedef std::shared_ptr<TimerManager> ptr;
    typedef RWMutex RWMutexType;
    TimerManager();
    virtual ~TimerManager();
    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb
                        ,bool recurring = false);//定时器管理类添加定时器
    //需要注意的是，由于 std::weak_ptr 无法直接访问对象的成员函数或成员变量，如果你需要使用具体类型的功能，
    ///你需要通过将 std::weak_ptr<void> 转换为适当的类型来实现。例如，你可以使用 std::dynamic_pointer_cast 或 std::static_pointer_cast 
    //将其转换为适当的类型的 std::shared_ptr，然后进行访问
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb
                        ,std::weak_ptr<void> weak_cond
                        ,bool recurring = false);//std::weak_ptr<void> weak_cond 条件
    uint64_t getNextTimer();//到最近一个定时器执行的时间间隔
    void listExpiredCb(std::vector<std::function<void()> >& cbs); //获取需要执行的定时器回调函数列表
    bool hasTimer();//是否还有定时器存在

protected:
    virtual void onTimerInsertedAtFront() = 0;//当有新的定时器直接插入到定时器的最前面一个，执行该函数 说明要对epoll做一些唤醒操作了
    void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);//将定时器添加到管理中
private:

    //检测系统的时间是否被调后了 因为调后的话那些执行的时间就是全错误了
    bool detectClockRollover(uint64_t now_ms);
private:
    RWMutexType m_mutex;
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    /// 是否触发onTimerInsertedAtFront
    bool m_tickled = false;
    uint64_t m_previouseTime = 0;//每次进行检验的时间

};

};



#endif