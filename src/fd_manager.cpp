#include "fd_manager.h"
#include "qlc_log.h"
#include "hook.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
namespace qlc{

FdCtx::FdCtx(int fd)
    :m_isInit(false)
    ,m_isSocket(false)
    ,m_sysNonblock(false)
    ,m_userNonblock(false)
    ,m_isClosed(false)
    ,m_fd(fd)
    ,m_recvTimeout(-1)
    ,m_sendTimeout(-1) {
    init();
}
FdCtx::~FdCtx() {

}
bool FdCtx::init(){
    if(m_isInit) {
        return true;
    }
    m_recvTimeout = -1;
    m_sendTimeout = -1;
    struct stat fd_stat;
    if(-1==fstat(m_fd,&fd_stat)){
        m_isInit = false;
        m_isSocket = false;
    }else{
        m_isInit = true;
        m_isSocket = S_ISSOCK(fd_stat.st_mode);//以下是一些常用的POSIX宏，用于检查st_mode字段中的文件类型：

    }
/*
S_ISREG(m)	检查m是否表示一个普通文件。
S_ISDIR(m)	检查m是否表示一个目录。
S_ISCHR(m)	检查m是否表示一个字符设备文件。
S_ISBLK(m)	检查m是否表示一个块设备文件。
S_ISFIFO(m)或S_ISPIPE(m)	检查m是否表示一个FIFO（命名管道）或管道文件。
S_ISLNK(m)	检查m是否表示一个符号链接（在UNIX系统上）。
S_ISSOCK(m)	检查m是否表示一个套接字（在某些系统上）。
*/
    if(m_isSocket) {
        int flags = fcntl_f(m_fd, F_GETFL, 0);
        if(!(flags & O_NONBLOCK)) {//如果是阻塞的
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);//设置为非阻塞
        }
        m_sysNonblock = true;
    } else {
        m_sysNonblock = false;
    }
    m_userNonblock = false;
    m_isClosed = false;
    return m_isInit;
}
void FdCtx::setTimeout(int type, uint64_t v) {
    if(type == SO_RCVTIMEO) {
        m_recvTimeout = v;
    } else {
        m_sendTimeout = v;
    }
}
uint64_t FdCtx::getTimeout(int type) {
    if(type == SO_RCVTIMEO) {
        return m_recvTimeout;
    } else {
        return m_sendTimeout;
    }
}
FdManager::FdManager() {
    m_datas.resize(64);
}
FdCtx::ptr FdManager::get(int fd, bool auto_create) {
    if(fd == -1) {
        return nullptr;
    }
    RWType::ReadLock lock(m_mutex);
    if((int)m_datas.size() <= fd) {
        if(auto_create == false) {
            return nullptr;
        }
    } else {
        if(m_datas[fd] || !auto_create) {
            return m_datas[fd];
        }
    }
    lock.unlock();

    RWType::WriteLock lock2(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));//这边就调用了init函数了  私有成员的东西只有在自己类里面使用 在其他类里面是不可以使用的
    if(fd >= (int)m_datas.size()) {
        m_datas.resize(fd * 1.5);
    }
    m_datas[fd] = ctx;
    return ctx;
}
void FdManager::del(int fd){
    RWType::WriteLock lock(m_mutex);
    if((int)m_datas.size() <= fd) {
        return;
    }
    m_datas[fd].reset();

}
}