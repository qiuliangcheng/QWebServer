#ifndef __QLC_FDMAN_H
#define __QLC_FDMAN_H
#include <memory>
#include <vector>
#include "thread.h"
#include "singleton.h"
namespace qlc{
class FdCtx : public std::enable_shared_from_this<FdCtx>{
public:
    typedef std::shared_ptr<FdCtx> ptr;
    FdCtx(int fd);
    ~FdCtx();
    /**
     * @brief 是否socket
     */
    bool isSocket() const { return m_isSocket;}

    /**
     * @brief 是否已关闭
     */
    bool isClose() const { return m_isClosed;}
    void setUserNonblock(bool v) { m_userNonblock = v;}
    /**
     * @brief 获取是否用户主动设置的非阻塞
     */
    bool getUserNonblock() const { return m_userNonblock;}
    bool isInit() const { return m_isInit;}
    void setSysNonblock(bool v) { m_sysNonblock = v;}
    bool getSysNonblock() const { return m_sysNonblock;}
    /*
     * @param[in] type 类型SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
     * @return 超时时间毫秒
     */
    uint64_t getTimeout(int type);
    void setTimeout(int type, uint64_t v);
private:
    bool init();

private:
    bool m_isInit : 1;
    /// 是否socket
    bool m_isSocket: 1;
    /// 是否hook非阻塞
    bool m_sysNonblock: 1;
    /// 是否用户主动设置非阻塞
    bool m_userNonblock: 1;
    /// 是否关闭
    bool m_isClosed: 1;
    int m_fd;
    /// 读超时时间毫秒
    uint64_t m_recvTimeout;
    /// 写超时时间毫秒
    uint64_t m_sendTimeout;

}; 

class FdManager{
public:
    typedef RWMutex RWType;
    FdManager();
    //若没有句柄 是否自动创建
    FdCtx::ptr get(int fd, bool auto_create = false);
    void del(int fd);//删除句柄信息
private:
    RWType m_mutex;
    std::vector<FdCtx::ptr> m_datas;

};
typedef Singleton<FdManager> FdMgr;

}


#endif