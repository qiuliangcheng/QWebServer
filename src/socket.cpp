#include "socket.h"
#include "qlc.h"
#include "fd_manager.h"
#include "hook.h"
#include "iomanager.h"
#include <limits.h>
#include <netinet/tcp.h> 
namespace qlc{
static qlc::Logger::ptr g_logger =QLC_LOG_NAME("system");

Socket::Socket(int family, int type, int protocol):m_sock(-1),
                m_family(family),
                m_type(type),
                m_protocol(protocol),
                m_isconnected(false)
{
}

Socket::~Socket()
{
    close();
}
int64_t Socket::getSendTimeout()
{
    FdCtx::ptr ctx=FdMgr::GetInstanceX()->get(m_sock);
    if(ctx){
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::setSendTimeout(int64_t v)
{
    struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};//ms单位
    setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
    FdCtx::ptr ctx=FdMgr::GetInstanceX()->get(m_sock);
    if(ctx){
        ctx->setTimeout(SO_SNDTIMEO,v);//也要更新一下吧应该
    }
}

int64_t Socket::getRecvTimeout()
{
    FdCtx::ptr ctx=FdMgr::GetInstanceX()->get(m_sock);
    if(ctx){
        return ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::setRecvTimeout(int64_t v)
{
    struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};//ms单位
    setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
    FdCtx::ptr ctx=FdMgr::GetInstanceX()->get(m_sock);
    if(ctx){
        ctx->setTimeout(SO_RCVTIMEO,v);//也要更新一下吧应该
    }
}

bool Socket::getOption(int level, int option, void *result, socklen_t *optlen)
{
    int res=getsockopt(m_sock,level,option,result,optlen);
    if(res){
        QLC_LOG_DEBUG(g_logger)<<"getSockOpt Error m_sock= "<<m_sock \
        <<" level= "<<level \
        <<"option ="<<option \
        <<"errno= "<<errno<<"error string= "<<strerror(errno);
    }
    return true;
}
bool Socket::setOption(int level, int option, const void *result, socklen_t optlen)
{
    int res=setsockopt(m_sock,level,option,result,optlen);
    if(res){
        QLC_LOG_DEBUG(g_logger)<<"getSockOpt Error m_sock= "<<m_sock \
        <<" level= "<<level \
        <<"option ="<<option \
        <<"errno= "<<errno<<"error string= "<<strerror(errno);
    }
    return true;
}

Socket::ptr Socket::accept()
{
    Socket::ptr sock(new Socket(m_family,m_type,m_protocol));
    int newsock=::accept(m_sock,nullptr,nullptr);
    if(newsock == -1) {
        QLC_LOG_ERROR(g_logger) << "accept(" << m_sock << ") errno="
            << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    if(sock->init(newsock)) { //newsock
        return sock;
    }
    
    return nullptr;
}
bool Socket::bind(const qlc::Address::ptr address)
{
    if(!isValid()){
        newSock();
        if(QLC_UNLIKELY(!isValid())) {
            return false;
        }
    }
    if(QLC_UNLIKELY(address->getFamily() != m_family)) {
        QLC_LOG_ERROR(g_logger) << "bind sock.family("
            << m_family << ") addr.family(" << address->getFamily()
            << ") not equal, addr=" << address->toString();
        return false;
    }
    UnixAddress::ptr uaddr=std::dynamic_pointer_cast<UnixAddress>(address);
    if(uaddr){//如果我传入的不是unix类型的 那这个值是为空的
        Socket::ptr sock = Socket::CreateUnixTCPSocket();
        if(sock->connect(uaddr)) {
            return false;
        }
    }
    if(!::bind(m_sock,address->getAddr(),address->getAddrLen())){//服务器端要用 bind() 函数将套接字与特定的IP地址和端口绑定起来，只有这样，流经该IP地址和端口的数据才能交给套接字处理
        QLC_LOG_ERROR(g_logger) << "bind error errrno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }
    getLocalAddress();
    return true;
}
bool Socket::connect(const qlc::Address::ptr address, uint64_t timeout_ms)
{
    m_remoteAddress=address;
    if(!isValid()){
        newSock();
        if(QLC_UNLIKELY(!isValid())) {
            return false;
        }
    }
    if(QLC_UNLIKELY(address->getFamily() != m_family)) {
        QLC_LOG_ERROR(g_logger) << "connect sock.family("
            << m_family << ") addr.family(" << address->getFamily()
            << ") not equal, addr=" << address->toString();
        return false;
    }
    if(timeout_ms==(uint64_t)-1){
        if(::connect(m_sock, address->getAddr(), address->getAddrLen())){
            QLC_LOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << address->toString()
                << ")" << " error errno="<< errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    }else{
        if(connect_with_timeout(m_sock, address->getAddr(), address->getAddrLen(),timeout_ms)) {
            QLC_LOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << address->toString()
                << ") timeout=" << timeout_ms << " error errno="
                << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    }
    m_isconnected=true;
    getRemoteAdress();
    getLocalAddress();
    return true;
}
bool Socket::reconnect(uint64_t timeout_ms)
{
    if(!m_remoteAddress) {
        QLC_LOG_ERROR(g_logger) << "reconnect m_remoteAddress is null";
        return false;
    }
    m_localAddress.reset();
    return connect(m_remoteAddress, timeout_ms);
}
bool Socket::listen(int backlog)
{
    if(!isValid()) {
        QLC_LOG_ERROR(g_logger) << "listen error sock=-1";
        return false;
    }
    if(::listen(m_sock, backlog)) {
        QLC_LOG_ERROR(g_logger) << "listen error errno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}
bool Socket::close()
{
    if(!m_isconnected&&m_sock==-1){
        return true;
    }
    m_isconnected=false;
    if(m_sock!=-1){
        if(::close(m_sock)==0){
            m_sock=-1;
            return true;
        }
    }
    return false;
}
int Socket::send(const void *buffer, size_t length, int flags)
{
    if(isconnectd()){ //每个外部的ip服务器都会有一个专门的sock与他对应
        return ::send(m_sock,buffer,length,flags);  
    }
    return -1;
}
int Socket::send(const iovec *buffer, size_t iocnt, int flags)
{
    if(isconnectd()){
        msghdr msg;
        memset(&msg,0,sizeof(msg));
        msg.msg_iovlen=iocnt;
        msg.msg_iov=(iovec *)buffer;
        return ::sendmsg(m_sock, &msg, flags);

    }
    return -1;
}
int Socket::sendTo(const void *buffer, size_t length, const Address::ptr to, int flags)
{
    if(isconnectd()) {
        return ::sendto(m_sock, buffer, length, flags, to->getAddr(), to->getAddrLen());
    }
    return -1;
}
int Socket::sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags)
{
    if(isconnectd()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        msg.msg_name = to->getAddr();
        msg.msg_namelen = to->getAddrLen();
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}
int Socket::recv(void *buffer, size_t length, int flags)
{
    if(isconnectd()) {
        return ::recv(m_sock, buffer, length, flags);
    }
    return -1;
}
int Socket::recv(const iovec *buffers, size_t length, int flags)
{
    if(isconnectd()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}
int Socket::recvFrom(void *buffer, size_t length, Address::ptr from, int flags)
{
    if(isconnectd()) {
        socklen_t len = from->getAddrLen();
        return ::recvfrom(m_sock, buffer, length, flags, from->getAddr(), &len);
    }
    return -1;
}
int Socket::recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags)
{
    if(isconnectd()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        msg.msg_name = from->getAddr();
        msg.msg_namelen = from->getAddrLen();
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}
Address::ptr Socket::getLocalAddress()
{
    if(m_localAddress) {
        return m_localAddress;
    }

    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknownAddress(m_family));
            break;
    }
    socklen_t addrlen = result->getAddrLen();
    if(getsockname(m_sock, result->getAddr(), &addrlen)) {
        QLC_LOG_ERROR(g_logger) << "getsockname error sock=" << m_sock
            << " errno=" << errno << " errstr=" << strerror(errno);
        return Address::ptr(new UnknownAddress(m_family));
    }
    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddlen(addrlen);
    }
    m_localAddress = result;
    return m_localAddress;
}
Address::ptr Socket::getRemoteAdress()
{
    if(m_remoteAddress) {
        return m_remoteAddress;
    }

    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknownAddress(m_family));
            break;
    }
    socklen_t addrlen = result->getAddrLen();
    if(getpeername(m_sock, result->getAddr(), &addrlen)) {
        // QLC_LOG_ERROR(g_logger) << "getsockname error sock=" << m_sock
        //     << " errno=" << errno << " errstr=" << strerror(errno);
        return Address::ptr(new UnknownAddress(m_family));
    }
    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddlen(addrlen);
    }
    m_remoteAddress = result;
    return m_remoteAddress;
}
bool Socket::isValid() const
{
    return m_sock!=-1;;
}
int Socket::getError()
{
    int error=0;
    socklen_t length=sizeof(error);
    if(!getOption(SOL_SOCKET,SO_ERROR,&error,&length)){
        error=errno;//如果这个函数处理失败 则把全局的错误赋值给errOr 
    }
    return error;
}

std::ostream &Socket::dump(std::ostream &os) const
{
    // TODO: insert return statement here
    os << "[Socket sock=" << m_sock
       << " is_connected=" << m_isconnected
       << " family=" << m_family
       << " type=" << m_type
       << " protocol=" << m_protocol;
    if(m_localAddress) {
        os << " local_address=" << m_localAddress->toString();
    }
    if(m_remoteAddress) {
        os << " remote_address=" << m_remoteAddress->toString();
    }
    os << "]";
    return os;
}

std::string Socket::toString() const
{
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

bool Socket::cancelRead()
{
    return IOManager::GetThis()->cancelEvent(m_sock,qlc::IOManager::READ);
}

bool Socket::cancelWrite()
{
    return IOManager::GetThis()->cancelEvent(m_sock,qlc::IOManager::WRITE);
}

bool Socket::cancelAll()
{
    return IOManager::GetThis()->cancelAll(m_sock);
}

bool Socket::cancelAccept()
{
    return IOManager::GetThis()->cancelEvent(m_sock,qlc::IOManager::READ);
}

void Socket::initSock()
{
    int val=1;
    if(!setOption(SOL_SOCKET,SO_REUSEADDR,val)){//一般来说，一个端口释放后会等待两分钟之后才能再被使用，SO_REUSEADDR是让端口释放后立即就可以被再次使用
        QLC_LOG_DEBUG(g_logger)<<"SET OPTION FAILED";
        // 这个套接字选项通知内核，如果端口忙，但TCP状态位于 TIME_WAIT ，
        // 可以重用端口。如果端口忙，而TCP状态位于其他状态，重用端口时依旧得到一个错误信息，
        // 指明"地址已经使用中"。
        // 如果你的服务程序停止后想立即重启，
        // 而新套接字依旧使用同一端口，此时SO_REUSEADDR 选项非常有用。
    }
    if(m_type==SOCK_STREAM){
         setOption(IPPROTO_TCP, TCP_NODELAY, val);//启动TCP_NODELAY，就意味着禁用了Nagle算法，允许小包的发送。对于延时敏感型，同时数据传输量比较小的应用，开启TCP_NODELAY选项无疑是一个正确的选择。
    }

}

void Socket::newSock()
{
    m_sock=socket(m_family,m_type,m_protocol);
    if(QLC_LIKELY(m_sock!=-1)){
        initSock();
    }else{
        QLC_LOG_DEBUG(g_logger)<<"socket create failed, (family = "<<m_family \
        <<"m_type = "<<m_type<<")"<<"errno="<<errno<<"errstr="<<strerror(errno);
    }
}

bool Socket::init(int sock)
{
    FdCtx::ptr ctx = FdMgr::GetInstanceX()->get(sock);
    if(ctx && ctx->isSocket() && !ctx->isClose()) {
        m_sock = sock;
        m_isconnected = true;
        initSock();
        getLocalAddress();
        getRemoteAdress();
        return true;
    }
    return false;
}

Socket::ptr Socket::CreateTCP(qlc::Address::ptr address)
{
    Socket::ptr sock(new Socket(address->getFamily(),TCP,0));
    return sock;
}

Socket::ptr Socket::CreateUDP(qlc::Address::ptr address)
{
    Socket::ptr sock(new Socket(address->getFamily(),UDP,0));
    sock->newSock();
    sock->m_isconnected=true;
    return sock;
}
Socket::ptr Socket::CreateTCPSocket()
{
    Socket::ptr sock(new Socket(IPv4,TCP,0));
    return sock;
}
Socket::ptr Socket::CreateUDPSocket()
{
    Socket::ptr sock(new Socket(IPv4,UDP,0));
    sock->newSock();
    sock->m_isconnected=true;
    return sock;
}
Socket::ptr Socket::CreateTCPSocket6() {
    Socket::ptr sock(new Socket(IPv6, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPSocket6() {
    Socket::ptr sock(new Socket(IPv6, UDP, 0));
    sock->newSock();
    sock->m_isconnected = true;
    return sock;
}
Socket::ptr Socket::CreateUnixTCPSocket() {
    Socket::ptr sock(new Socket(UNIX, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUnixUDPSocket() {
    Socket::ptr sock(new Socket(UNIX, UDP, 0));
    return sock;
}



}
