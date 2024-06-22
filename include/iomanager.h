#ifndef __QLC_IOMANAGER_H
#define __QLC_IOMANAGER_H

#include "schedular.h"
#include "timer.h"
namespace qlc{

class IOManager : public Schedular,public TimerManager{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;
    enum Event{
        NONE    = 0x0,
        /// 读事件(EPOLLIN)
        READ    = 0x1,
        /// 写事件(EPOLLOUT)
        WRITE   = 0x4,
    };
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    ~IOManager();

private:
    //一个文件句柄所包含的信息
    struct FdContext{
        typedef Mutex MutexType;
        struct EventContext{
            Schedular* schedular = nullptr;
            Fiber::ptr fiber;
            std::function<void()> cb;

        };
        //返回对应事件的上下文
        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);//重置事件上下文
        void triggerEvent(Event event);//对fd触发事件
        EventContext read;
        EventContext write;
        int fd;//事件关联的句柄
        Event events =NONE;//当前句柄的事件
        MutexType mutex;

    };
public:
    //为句柄fd添加事件和回调函数
    //添加成功返回0,失败返回-1
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    //直接删除事件
    bool delEvent(int fd, Event event);
    //删除事件并且首先先触发事件
    bool cancelEvent(int fd, Event event);
    //取消句柄中的所有事件
    bool cancelAll(int fd);
    static IOManager* GetThis();

protected:
    bool stopping() override;
    void tickle() override;
    void idle() override;
    void onTimerInsertedAtFront() override;
    //m_fdContexts 设置这个的大小
    void contextResize(size_t size);
    //timeout:最近要出发的定时器事件间隔
    bool stopping(uint64_t& timeout);
private:
    RWMutexType m_mutex;
    int m_epfd=0;//epoll文件句柄
    int m_tickleFds[2];//pipe文件句柄   【0】用来读 1用来写
    std::atomic<size_t> m_pendingEventCount = {0}; //当前等待执行的事件数量
    std::vector<FdContext*> m_fdContexts;



};






}

#endif