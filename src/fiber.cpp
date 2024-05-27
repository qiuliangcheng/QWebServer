#include "fiber.h"
#include "qlc_log.h"
#include "config.h"
#include "macro.h"
#include "schedular.h"
namespace qlc{
    static Logger::ptr g_logger=QLC_LOG_NAME("system");\
    //静态变量 静态方法里调用
    static std::atomic<uint64_t> s_fiber_id {0};
    static std::atomic<uint64_t> s_fiber_count {0};
    static thread_local Fiber* t_fiber=nullptr;
    static thread_local Fiber::ptr t_threadFiber =nullptr;//主协程
    static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
        Config::Lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

class MallocStackAllocator{ //统一分配内存
public:
    static void* alloc(size_t size){
        return malloc(size);
    }
    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};
using StackAllocator = MallocStackAllocator;//使用这个可以使得后面更改分配函数更为方便



uint64_t Fiber::GetFiberId()
{
    if(t_fiber){
        return t_fiber->getId();
    }
    return 0;
}
Fiber::Fiber() { //创建主线程
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) {
        QLC_ASSERT2(false, "getcontext");
    }

    ++s_fiber_count;

    QLC_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}

Fiber::Fiber(std::function<void()> cb, size_t stack_size,bool use_caller):m_id(++s_fiber_id)
                                                          ,m_cb(cb)
{
    ++s_fiber_count;
    m_stacksize = stack_size ? stack_size : g_fiber_stack_size->getValue();
    m_stack=StackAllocator::alloc(m_stacksize);
    if(getcontext(&m_ctx)) {
        QLC_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link=nullptr;
    m_ctx.uc_stack.ss_sp=m_stack;
    m_ctx.uc_stack.ss_size=m_stacksize;
    if(!use_caller){
        makecontext(&m_ctx,&Fiber::MainFunc,0);
    }else{
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }

    QLC_LOG_DEBUG(g_logger) << "Fiber::Fiber id= " <<m_id;
}

Fiber::~Fiber()
{
    --s_fiber_count;
    if(m_stack) {
        QLC_ASSERT((m_state == TERM
                || m_state == EXCEPT
                || m_state == INIT));

        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        QLC_ASSERT(!m_cb);
        QLC_ASSERT((m_state == EXEC));

        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    QLC_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
                              << " total=" << s_fiber_count;
}
void Fiber::reset(std::function<void()> cb) {
    QLC_ASSERT(m_stack);
    QLC_ASSERT((m_state == TERM
            || m_state == EXCEPT
            || m_state == INIT));
    m_cb = cb;
    if(getcontext(&m_ctx)) {
        QLC_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}
void Fiber::call() {//子协程调用
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        QLC_ASSERT2(false, "swapcontext");
    }
}
void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}
Fiber::ptr Fiber::GetThis()
{
    if(t_fiber){
        return t_fiber->shared_from_this();
    }
    //如果没有主协程的话
    Fiber::ptr main(new Fiber);
    QLC_ASSERT(t_fiber==main.get());
    t_threadFiber=main;
    return t_fiber->shared_from_this();//当前没有协程但是创建了一个新的
}
void Fiber::YieldToReady()
{
    Fiber::ptr cur = GetThis();
    QLC_ASSERT(cur->m_state==EXEC);
    cur->m_state=READY;
    cur->swapOut();
}
void Fiber::YieldToHold()
{
    Fiber::ptr cur = GetThis();
    QLC_ASSERT(cur->m_state==EXEC);
    cur->m_state=HOLD;
    cur->swapOut();
}
uint64_t Fiber::TotalFibers()
{
    return s_fiber_count;
}
void Fiber::swapIn()
{ // 切断到当前携程
    SetThis(this);
    QLC_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&Schedular::GetMainFiber()->m_ctx, &m_ctx)) {
        QLC_ASSERT2(false, "swapcontext");
    }
}
void Fiber::swapOut() {
    SetThis(Schedular::GetMainFiber());
    if(swapcontext(&m_ctx, &Schedular::GetMainFiber()->m_ctx)) {
        QLC_ASSERT2(false, "swapcontext");
    }
}
void Fiber::back()
{
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        QLC_ASSERT2(false, "swapcontext");
    }
}
void Fiber::MainFunc()
{
    Fiber::ptr cur=GetThis();
    QLC_ASSERT(cur);
    try{
        cur->m_cb();
        cur->m_cb=nullptr;
        cur->m_state=TERM;
    }catch(std::exception& ex){
        cur->m_state=EXCEPT;
        QLC_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << qlc::BacktraceToString();
    }

    //如果不写下面这些的话 运行完了后不回主协程
    auto raw =cur.get();
    cur.reset();
    raw->swapOut();
    QLC_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw->getId()));}

void Fiber::CallerMainFunc()
{
    Fiber::ptr cur=GetThis();
    QLC_ASSERT(cur);
    try{
        cur->m_cb();
        cur->m_cb=nullptr;
        cur->m_state=TERM;
    }catch(std::exception& ex){
        cur->m_state=EXCEPT;
        QLC_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << qlc::BacktraceToString();
    }

    //如果不写下面这些的话 运行完了后不回主协程
    auto raw =cur.get();
    cur.reset();
    raw->back();
    QLC_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw->getId()));
}
}