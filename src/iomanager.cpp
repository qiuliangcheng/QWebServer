#include "iomanager.h"
#include "qlc_log.h"
#include "macro.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>
namespace qlc{
enum  EpollCtlOp {
};
static qlc::Logger::ptr g_logger=QLC_LOG_NAME("system");
void IOManager::contextResize(size_t size)
{
    m_fdContexts.resize(size);
    for(size_t i=0;i<m_fdContexts.size();i++){
        if(!m_fdContexts[i]){
            m_fdContexts[i]=new FdContext();
            m_fdContexts[i]->fd=i;
        }
    }

}
IOManager::IOManager(size_t threads, bool use_caller, const std::string &name):Schedular(threads, use_caller, name)
{
    m_epfd= epoll_create(1);//一个io调度器有一个epoll
    QLC_ASSERT(m_epfd>0);
    int rt=pipe(m_tickleFds);//创建管道 返回值0是成功
    QLC_ASSERT(!rt);
    epoll_event event;
    memset(&event,0,sizeof(epoll_event));
    event.events=EPOLLIN|EPOLLOUT;
    event.data.fd=m_tickleFds[0];//读取管道0通道 有消息epoll就通知了
    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);//设置管道非阻塞
    rt=epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_tickleFds[0],&event);//就相当于把event和这个fd绑在一起了
    QLC_ASSERT(!rt);
    contextResize(32);
    start();//先启动 跑空协程模块 
}

IOManager::~IOManager()
{
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);
    for(size_t i=0;i<m_fdContexts.size();i++){
        if(m_fdContexts[i]){
            delete m_fdContexts[i];
        }
    }
}

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event)
{
    switch(event){
        case IOManager::READ: return read;
        case IOManager::WRITE: return write;
        default:
            QLC_ASSERT2(false,"getcontext失败");
    }

    throw std::invalid_argument("getContext invalid event");
}

void IOManager::FdContext::resetContext(EventContext &ctx)
{
    ctx.cb=nullptr;
    ctx.fiber.reset();
    ctx.schedular=nullptr;
}
void IOManager::FdContext::triggerEvent(Event event)
{
    QLC_ASSERT(events&event);//确保这个fd里面有这个事件
    events = (Event)(events & ~event);//x消除这个事件
    EventContext &ctx=getContext(event);
    if(ctx.cb){
        ctx.schedular->schedule(&ctx.cb);//让这个cb为空 直接交换了 相当于把这个任务直接放进协程里了
    }else{
         ctx.schedular->schedule(&ctx.fiber);
    }
    ctx.schedular=nullptr;//因为这个事件已经结束了 所以把这个调度器也置为空
    return;
}
//对fd添加事件
int IOManager::addEvent(int fd, Event event, std::function<void()> cb)
{
    FdContext* fd_ctx=nullptr;
    RWMutexType::ReadLock lock(m_mutex);//上个读锁
    if((int)m_fdContexts.size()>fd){
        fd_ctx=m_fdContexts[fd];
        lock.unlock();
    }else{//说明添加的fd不在里面 需要扩大
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(fd*1.5);//每次扩大1.5倍
        fd_ctx = m_fdContexts[fd];
    }
    // FdContext::MutexType::Lock lock2(fd_ctx->mutex);//互斥锁
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if((fd_ctx->events&event)){//说明已经有了这个事件 不能再添加了
        QLC_ASSERT(!(fd_ctx->events & event));
    }
    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;//如果本来有事件 就是修改 否则就是添加
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->events | event;//添加上事件
    epevent.data.ptr = fd_ctx;
    int rt=epoll_ctl(m_epfd,op,fd,&epevent);//成功是返回0的
    if(rt){
        QLC_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->events;
        return -1;
    }
    ++m_pendingEventCount;
    fd_ctx->events = (Event)(fd_ctx->events | event);//更新
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);//获取要添加的event的ctx
    QLC_ASSERT(!event_ctx.schedular
                && !event_ctx.fiber
                && !event_ctx.cb);//这些都应该要为空 因为是新添加的
    //接下来设置调度器那些东西

    event_ctx.schedular=Schedular::GetThis();//让运行这个的线程的调度器来跑
    if(cb) {
        event_ctx.cb.swap(cb);
    } else {
        event_ctx.fiber = Fiber::GetThis();//当前协程跑
        QLC_ASSERT2(event_ctx.fiber->getState() == Fiber::EXEC
                      ,"state=" << event_ctx.fiber->getState());
    }

    return 0;
}

bool IOManager::delEvent(int fd, Event event)
{
    RWMutexType::ReadLock lock(m_mutex);//上个读锁
    if((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext *fd_ctx=m_fdContexts[fd];
    lock.unlock();
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events&event)){
        return false;
    }
    Event new_events=(Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;
    int rt=epoll_ctl(m_epfd,op,fd,&epevent);
    if(rt){
        QLC_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->events;
        return false;
    }
    --m_pendingEventCount;
    fd_ctx->events=new_events;
    FdContext::EventContext& event_ctx=fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}
bool IOManager::cancelEvent(int fd, Event event)
{
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    } 
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events&event)){
        return false;
    }
    Event new_events=(Event)(fd_ctx->events & ~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;
    int rt=epoll_ctl(m_epfd,op,fd,&epevent);
    if(rt){
        QLC_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->events;
        return false;
    }
    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}

bool IOManager::cancelAll(int fd)
{
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size() <= fd) {
        return false;
    } 
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!fd_ctx->events){
        return false;
    }
    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;
    int rt=epoll_ctl(m_epfd,op,fd,&epevent);
    if(rt){
        QLC_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
            << (EPOLL_EVENTS)fd_ctx->events;
        return false;
    }
    if(fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if(fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }
    QLC_ASSERT(fd_ctx->events==0);
    return true;
}

IOManager *IOManager::GetThis()
{
    return dynamic_cast<IOManager*>(Schedular::GetThis());//父类转为子类
}
bool IOManager::stopping(uint64_t& timeout) {
    timeout = getNextTimer();//获取最近的定时器  看看有没有定时器运行
    return timeout == ~0ull
        && m_pendingEventCount == 0
        && Schedular::stopping();

}
bool IOManager::stopping() {
    uint64_t timeout = 0;
    return stopping(timeout);
}

void IOManager::tickle()
{
    if(!hasIdleThreads()) { //应该说 得有正在跑idle的线程
        return;
    }
    // QLC_LOG_ERROR(g_logger)<<"trikle";
    int rt = write(m_tickleFds[1], "T", 1);//写T
    QLC_ASSERT(rt==1);
}
//这个函数是当线程空闲的时候一直在监听有没有来的fd
void IOManager::idle()
{
    QLC_LOG_INFO(g_logger)<<"idle";
    const uint64_t MAX_EVENTS=256;
    epoll_event* events=new epoll_event[MAX_EVENTS]();
    std::shared_ptr<epoll_event> share_events(events,[](epoll_event* ptr){
        delete[] ptr;
    });
    while(true){
        uint64_t next_timeout = 0;
        if(stopping(next_timeout)){
            QLC_LOG_INFO(g_logger)<<"name= "<<m_name<<"idle stoping exit";
            break;
        }
        int rt=0;
        do{
            static const int MAX_TIMEOUT=5000;
            if(next_timeout!=~0ull){//说明定时器不为空 
                next_timeout = (int)next_timeout > MAX_TIMEOUT
                                ? MAX_TIMEOUT : next_timeout;
            }else{
                next_timeout=MAX_TIMEOUT;
            }
            rt=epoll_wait(m_epfd,events,MAX_EVENTS,(int)next_timeout);//等待这么多秒 因为这么多秒后 会有定时器完成
            if(rt<0&&errno==EINTR){
            }else{
                break;
            }

        }while(true);
        std::vector<std::function<void()>> cbs;        
        listExpiredCb(cbs);
        if(!cbs.empty()){
            schedule(cbs.begin(),cbs.end());
            cbs.clear();
        }
        for(int i=0;i<rt;i++){
            epoll_event& event=events[i];
            if(event.data.fd == m_tickleFds[0]) {
                uint8_t dummy[256];
                while(read(m_tickleFds[0], dummy, sizeof(dummy)) > 0);//说明是trikle发过来的
                // QLC_LOG_ERROR(g_logger)<<"trikle来消息了";
                continue;
            }
            FdContext *fd_ctx= (FdContext *)event.data.ptr;
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            if(event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
            }
            int real_events = NONE;//用来触发事件的
            if(event.events & EPOLLIN) {
                real_events |= READ;
            }
            if(event.events & EPOLLOUT) {
                real_events |= WRITE;
            }
            if((fd_ctx->events & real_events) == NONE) {//一般来说哦不会出现吧 因为我添加事件的时候这个fd_ctx->events就已经跟新了
                continue;
            }
            int left_events = (fd_ctx->events & ~real_events);//可能单独来了个in
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;
            int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if(rt2){
                QLC_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                    << (EpollCtlOp)op << ", " << fd_ctx->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                    << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events="
                    << (EPOLL_EVENTS)fd_ctx->events;
                continue;
            }
            //接下来就是触发事件
            if(real_events & READ) {
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if(real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        }
        //这样就是一轮的完成了 然后返回主协程看有没有任务
        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->swapOut();
    }

}
void IOManager::onTimerInsertedAtFront()
{
    tickle();
}
}