#ifndef __QLC_FIBER_H
#define __QLC_FIBER_H
#include <memory>
#include <functional>
#include <ucontext.h>
// #include "schedular.h"
namespace qlc{
class Schedular;
class Fiber : public std::enable_shared_from_this<Fiber>{

friend class Schedular;

public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State{
        INIT ,//初始化
        HOLD,//挂起
        EXEC,//执行
        TERM,//终止
        READY,//准备就绪
        EXCEPT//异常结束
    };
private:
    //主协程构造
    Fiber();
public:
    //子协程构造
    Fiber(std::function<void()> cb, size_t stack_size=0,bool use_caller = false);
    ~Fiber();
    void reset(std::function<void ()> cb); //重置协程函数
    void swapIn();//转到此协程工作
    void swapOut();//切换到主协程
    void back();
    void call();//将此协程切换到执行状态
    uint64_t getId() const { return m_id;}
    State getState() const { return m_state;}

public:
//静态函数
    //设置当前线程运行的协程
    static void SetThis(Fiber* f);
    static Fiber::ptr GetThis();//返回当前所在协程
    static void YieldToReady();//将当前协程切换到后台,并设置为READY状态
    static void YieldToHold();//将当前协程切换到后台,并设置为HOLD状态
    static uint64_t TotalFibers();//返回当前协程的总数量
    static void MainFunc();//协程的执行函数  执行完成后回到线程主协程
    static void CallerMainFunc();
    static uint64_t GetFiberId();//获取当前协程的id
private:
    uint64_t m_id=0;//协程id
    uint32_t m_stacksize = 0;
    State m_state = INIT;
    ucontext_t m_ctx;//协程上下文
    void* m_stack = nullptr;//协程运行栈指针
    std::function<void()> m_cb;//协程运行函数

};




}


#endif