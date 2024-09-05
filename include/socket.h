#ifndef __QLC_SOCKET_H
#define __QLC_SOCKET_H
#include "address.h"

namespace qlc{
//domain AF_INET type 
//int socket(int domain, int type, int protocol);
//protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
class Socket:public std::enable_shared_from_this<Socket>{
public:
    typedef std::shared_ptr<Socket> ptr;
    enum Type{
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };
    enum Family{
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX
    };
    static Socket::ptr CreateTCP(qlc::Address::ptr address); //创建TCP的连接
    static Socket::ptr CreateUDP(qlc::Address::ptr address);
    /**
     * @brief 创建IPv4的TCP Socket
     */
    static Socket::ptr CreateTCPSocket();

    /**
     * @brief 创建IPv4的UDP Socket
     */
    static Socket::ptr CreateUDPSocket();

    /**
     * @brief 创建IPv6的TCP Socket
     */
    static Socket::ptr CreateTCPSocket6();

    /**
     * @brief 创建IPv6的UDP Socket
     */
    static Socket::ptr CreateUDPSocket6();

    /**
     * @brief 创建Unix的TCP Socket
     */
    static Socket::ptr CreateUnixTCPSocket();

    /**
     * @brief 创建Unix的UDP Socket
     */
    static Socket::ptr CreateUnixUDPSocket();

    Socket(int family, int type, int protocol);
    virtual ~Socket();
    int64_t getSendTimeout();//获取发送超时时间
    void setSendTimeout(int64_t v);
    int64_t getRecvTimeout();//获取接收超时时间
    void setRecvTimeout(int64_t v);
    //int (*getsockopt_fun)(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
    bool getOption(int level, int option, void *result, socklen_t *optlen);
    //不同类型的长度不用自己写 使用模板
    template<class T>
    bool getOption(int level,int option, T *result){
        socklen_t length=sizeof(T);
        return getOption(level, option, result, &length);
    }
    //设置socket参数
    bool setOption(int level, int option, const void* result, socklen_t optlen);
    template<class T>
    bool setOption(int level, int option, const T& value) {
        return setOption(level, option, &value, sizeof(T));
    }
    // @brief 接收connect链接
    virtual Socket::ptr accept();//接收连接
    //@brief 绑定地址   
    virtual bool bind(const qlc::Address::ptr address);  //限制了服务器只能接收这些ip地址的连接


    virtual bool connect(const qlc::Address::ptr address,uint64_t timeout_ms=-1);
    
    virtual bool reconnect(uint64_t timeout_ms=-1);


    //监听socket 监听到了 然后才accept
    virtual bool listen(int backlog = SOMAXCONN);//backlog 待连接队列的最大长度


    virtual bool close();//关闭socket
    /**
     * @brief 发送数据
     * @param[in] buffer 待发送数据的内存
     * @param[in] length 待发送数据的长度
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int send(const void* buffer, size_t length, int flags = 0);
    virtual int send(const iovec* buffer,size_t iocnt,int flags=0);

    /**
     * @brief 发送数据
     * @param[in] buffer 待发送数据的内存
     * @param[in] length 待发送数据的长度
     * @param[in] to 发送的目标地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);

    /**
     * @brief 发送数据
     * @param[in] buffers 待发送数据的内存(iovec数组)
     * @param[in] length 待发送数据的长度(iovec长度)
     * @param[in] to 发送的目标地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);

    /**
     * @brief 接受数据
     * @param[out] buffer 接收数据的内存
     * @param[in] length 接收数据的内存大小
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recv(void* buffer, size_t length, int flags = 0);

    /**
     * @brief 接受数据
     * @param[out] buffers 接收数据的内存(iovec数组)
     * @param[in] length 接收数据的内存大小(iovec数组长度)
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recv(const iovec* buffers, size_t length, int flags = 0);//ssize_t nwritten = writev(fd, iov, 2);

    /**
     * @brief 接受数据
     * @param[out] buffer 接收数据的内存
     * @param[in] length 接收数据的内存大小
     * @param[out] from 发送端地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recvFrom(void* buffer, size_t length, Address::ptr from, int flags = 0);

    /**
     * @brief 接受数据
     * @param[out] buffers 接收数据的内存(iovec数组)
     * @param[in] length 接收数据的内存大小(iovec数组长度)
     * @param[out] from 发送端地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

    Address::ptr getLocalAddress();
    Address::ptr getRemoteAdress();
    int getProtocol()const{return m_protocol;}
    int getType() const {return m_type;}
    int getFamily() const{return m_family;}
    bool isconnectd() const {return m_isconnected;}

    bool isValid() const;//m_sock=-1的话就是失效
    int getError();

    //输出信息到流中
    virtual std::ostream& dump(std::ostream& os) const;//直接输出Socket信息
    virtual std::string toString() const;

    //获取socket fd句柄
    int getSocket() const { return m_sock;}
    
    bool cancelRead();//取消读事件
    bool cancelWrite();
    bool cancelAll();
    bool cancelAccept();
protected:
    /**
     * @brief 初始化socket
     */
    void initSock();

    /**
     * @brief 创建socket
     */
    void newSock();
    /**
     * @brief 初始化sock
     */
    virtual bool init(int sock);
protected:
    int m_sock;//连接的句柄
    int m_family;
    int m_type; 
    int m_protocol;
    Address::ptr m_localAddress;
    Address::ptr m_remoteAddress;
    bool m_isconnected;


};
//double a = 1.5;  cout.operator<<(a);  //等价于cout << a;  这是用成员函数的方式进行重载
//string b = "hello world";  operator<<(cout,s) //等价于cout << s;  这是用全局函数的方式进行重载


std::ostream& operator<< (std::ostream&os,Socket::ptr sock);

}


#endif