#include "hook.h"
#include "qlc.h"
#include "thread.h"
#include <dlfcn.h>
#include "fiber.h"
#include "iomanager.h"
#include "fd_manager.h"

qlc::Logger::ptr g_logger=QLC_LOG_NAME("system");


namespace qlc{
static qlc::ConfigVar<int>::ptr g_tcp_connect_timeout = qlc::Config::Lookup("tcp.connect.timeout",5000,"tcp.connect.timeout");
static thread_local bool t_hook_enable = false;
static uint64_t s_connect_timeout = -1;
#define HOOK_Fun(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep)\
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)


void init_hook(){
    static bool is_init=false;
    if(is_init){
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_Fun(XX);
#undef XX
}
struct _init_HOOK{
    _init_HOOK(){
        init_hook();
        s_connect_timeout=g_tcp_connect_timeout->getValue();
        g_tcp_connect_timeout->addListener([](const int& old_value, const int& new_value){
                QLC_LOG_INFO(g_logger) << "tcp connect timeout changed from "
                                         << old_value << " to " << new_value;
                s_connect_timeout = new_value;
        });
    }
    
};
static _init_HOOK s_init_hook;
void set_hook_enable(bool flag){
    t_hook_enable=flag;
}
bool is_hook_enable(){
    return t_hook_enable;
}
struct timer_info {
    int cancelled = 0;
};

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name,
        uint32_t event, int timeout_so, Args&&... args) {//Args&&... args) 需要用到完美转发 不然地址啥的不一样 可能会进行拷贝构造啥的
    if(!t_hook_enable){
        return fun(fd,std::forward<Args>(args)...);//std::forward<Args>(args)... 中的 ... 用于展开参数包。这意味着对于参数包 args 中的每个参数，都会应用 std::forward<Args>。
        //结果是，每个参数都会以它原本的值类别（左值或右值）被转发到 fun 函数。
    }
    qlc::FdCtx::ptr ctx = qlc::FdMgr::GetInstanceX()->get(fd);
    if(!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if(ctx->isClose()) {
        errno = EBADF;
        return -1;
    }
    /*
    EACCES：表示权限被拒绝。
    EFAULT：表示地址错误。
    EINTR：表示系统调用被中断。
    EINVAL：表示无效参数。
    EIO：表示I/O错误。
    ENODEV：表示没有这样的设备。
    ENOENT：表示没有这样的文件或目录。
    ENOMEM：表示没有足够的内存。
    ENOSPC：表示没有空间。
    ENOTDIR：表示不是一个目录。
    */
    if(!ctx->isSocket() || ctx->getUserNonblock()) { //如果不是socket
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);//SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    ssize_t n=fun(fd, std::forward<Args>(args)...);
    while( n==-1 && errno ==EINTR){//io函数被信号中断
        n=fun(fd, std::forward<Args>(args)...);
    }
    if(n == -1 && errno == EAGAIN)//这个错误通常在非阻塞I/O操作中遇到，当所请求的操作不能立即完成时，系统会返回 EAGAIN 而不是阻塞调用者。
    {
        //如果说这个调用被阻塞了 那就切换到后面去执行
        qlc::IOManager* iom=qlc::IOManager::GetThis();
        qlc::Fiber::ptr fiber = qlc::Fiber::GetThis();
        qlc::Timer::ptr timer;
        std::weak_ptr<timer_info> week_timer(tinfo);
        if(to!=(uint64_t)-1){
            timer=iom->addConditionTimer(to,[week_timer, fd, iom, event](){
                std::shared_ptr<timer_info> wk=week_timer.lock();
                if(!wk||wk->cancelled){//如果其他线程已经取消了 那这就直接return了
                    return;
                }
                wk->cancelled=ETIMEDOUT;
                iom->cancelEvent(fd, (qlc::IOManager::Event)(event));
            },week_timer);
        }
        int rt=iom->addEvent(fd,(qlc::IOManager::Event)(event));//阻塞的挂在epoll树上
        if(rt==-1){
            QLC_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            if(timer) {
                timer->cancel();
            }
            return -1;
        }else{
            qlc::Fiber::YieldToHold();
            if(timer){
                timer->cancel();
            }
            if(tinfo->cancelled){
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;//等到我们的数据准备好 不阻塞 再调用一次io函数
        }

    }
    return n;
    }
}

extern "C"{

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_Fun(XX);
#undef XX
unsigned int sleep(unsigned int seconds){
    if(!qlc::t_hook_enable){
        return sleep_f(seconds);
    }
    qlc::Fiber::ptr fiber =qlc::Fiber::GetThis();
    qlc::IOManager *iom =qlc::IOManager::GetThis();
    iom->addTimer(seconds * 1000, std::bind((void(qlc::Schedular::*)
            (qlc::Fiber::ptr, int thread))&qlc::IOManager::schedule
            ,iom, fiber, -1));
    qlc::Fiber::YieldToHold();
    //iom - 这是 IOManager 类的对象，schedule 成员函数将在 iom 对象上调用。
    return 0;
}
/*
class qlc::Schedular {
public:
    void some_member_function();
};

void(qlc::Schedular::*) member_function_pointer = &qlc::Schedular::some_member_function;
*/

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) {
//If the connection or binding succeeds, zero is returned.  On error, -1 is returned, and errno is set appropriately.
    if(!qlc::t_hook_enable){
        return connect_f(fd,addr,addrlen);
    }
    qlc::FdCtx::ptr ctx = qlc::FdMgr::GetInstanceX()->get(fd);
    if(!ctx) {
        return connect_f(fd,addr,addrlen);
    }

    if(ctx->isClose()) {
        errno = EBADF;
        return -1;
    }
    /*
    EACCES：表示权限被拒绝。
    EFAULT：表示地址错误。
    EINTR：表示系统调用被中断。
    EINVAL：表示无效参数。
    EIO：表示I/O错误。
    ENODEV：表示没有这样的设备。
    ENOENT：表示没有这样的文件或目录。
    ENOMEM：表示没有足够的内存。
    ENOSPC：表示没有空间。
    ENOTDIR：表示不是一个目录。
    */
    if(!ctx->isSocket()) {
        return connect_f(fd, addr, addrlen);
    }

    if(ctx->getUserNonblock()) {
        return connect_f(fd, addr, addrlen);
    }

    std::shared_ptr<qlc::timer_info> tinfo(new qlc::timer_info);

    int n = connect_f(fd, addr, addrlen);
    if(n == 0) {
        return 0;
    } else if(n != -1 || errno != EINPROGRESS) {
        return n;
    }

        //如果说这个调用被阻塞了 那就切换到后面去执行
        qlc::IOManager* iom=qlc::IOManager::GetThis();
        qlc::Fiber::ptr fiber = qlc::Fiber::GetThis();
        qlc::Timer::ptr timer;
        std::weak_ptr<qlc::timer_info> week_timer(tinfo);
        if(timeout_ms!=(uint64_t)-1){
            timer=iom->addConditionTimer(timeout_ms,[week_timer, fd, iom](){
                auto wk=week_timer.lock();
                if(!wk||wk->cancelled){//如果其他线程已经取消了 那这就直接return了
                    return;
                }
                wk->cancelled=ETIMEDOUT;
                iom->cancelEvent(fd, qlc::IOManager::WRITE);
            },week_timer);
        }
        int rt=iom->addEvent(fd,qlc::IOManager::WRITE);//阻塞的挂在epoll树上
        if(rt==0){
            fiber->YieldToHold();
            if(timer){
                timer->cancel();
            }
            if(tinfo->cancelled){
                errno = tinfo->cancelled;
                return -1;
            }

        }else{
            QLC_LOG_ERROR(g_logger) <<"connect addEvent(" << fd << ", WRITE) error";
            if(timer) {
                timer->cancel();
            }
        }
    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
    // 调用 getsockopt 来检查套接字 fd 的待处理错误。
    // 如果 getsockopt 调用失败，它将返回 -1，并且通常会设置 errno 来指示错误的原因。
    // 如果 getsockopt 调用成功，它将返回 0，并且待处理错误码（如果有）会被写入 error 变量
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

int usleep(useconds_t usec){
    if(!qlc::t_hook_enable){
        return usleep_f(usec);
    }
    qlc::Fiber::ptr fiber =qlc::Fiber::GetThis();
    qlc::IOManager *iom =qlc::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(qlc::Schedular::*)
            (qlc::Fiber::ptr, int thread))&qlc::IOManager::schedule
            ,iom, fiber, -1));
    qlc::Fiber::YieldToHold();  
    return 0;
}
int nanosleep(const struct timespec *req, struct timespec *rem){
    if(!qlc::t_hook_enable){
        return nanosleep_f(req,rem);
    }
    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;
    qlc::Fiber::ptr fiber =qlc::Fiber::GetThis();
    qlc::IOManager *iom =qlc::IOManager::GetThis();
    iom->addTimer(timeout_ms, std::bind((void(qlc::Schedular::*)
            (qlc::Fiber::ptr, int thread))&qlc::IOManager::schedule
            ,iom, fiber, -1));
    qlc::Fiber::YieldToHold();  
    return 0;
}
int socket(int domain, int type, int protocol) {
    if(!qlc::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        return fd;
    }
    qlc::FdMgr::GetInstanceX()->get(fd, true);
    return fd;
}
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, qlc::s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = do_io(s, accept_f, "accept", qlc::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {
        qlc::FdMgr::GetInstanceX()->get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", qlc::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", qlc::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", qlc::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", qlc::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", qlc::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", qlc::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", qlc::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", qlc::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", qlc::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", qlc::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}




}