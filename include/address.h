#ifndef __QLC_ADDRESS_H
#define __QLC_ADDRESS_H
#include <memory>
#include <sys/types.h>          
#include <sys/socket.h>
#include <vector>
#include <map>
#include <sys/un.h>
#include <arpa/inet.h>

namespace qlc{
class IPAddress;
class Address {
public:
    typedef std::shared_ptr<Address> ptr;
    static Address::ptr Create(const sockaddr* addr,socklen_t len);//
    Address()=default;

    /**
     * @brief 通过host地址返回对应条件的所有Address
     * @param[out] result 保存满足条件的Address
     * @param[in] host 域名,服务器名等.举例: www.sylar.top[:80] (方括号为可选内容)
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param[in] type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回是否转换成功
     */
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
    static Address::ptr LookupAny(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
    //根据主机名返回任意一个ip地址
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);
    //根据本机所有网卡地址
    /**
     * @brief 返回本机所有网卡的<网卡名, 地址, 子网掩码位数>
     * @param[out] result 保存本机所有地址
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @return 是否获取成功
     */
    static bool GetInterfaceAddresses(std::multimap<std::string
                    ,std::pair<Address::ptr, uint32_t> >& result,
                    int family = AF_INET);
    //获取指定网卡的网卡地址和子网掩码位数
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result
                    ,const std::string& iface, int family = AF_INET);
    

    virtual ~Address(){}

    int getFamily() const;
    /**
     * @brief 返回sockaddr指针,只读
     */
    virtual const sockaddr* getAddr() const =0;
    /**
     * @brief 返回sockaddr指针,读写
     */
    virtual sockaddr* getAddr()=0;
    virtual socklen_t getAddrLen() const =0;

    /**
     * @brief 可读性输出地址
     */
    virtual std::ostream& insert(std::ostream& os) const = 0;
    std::string toString() const;
    bool operator<(const Address&rhs) const;
    bool operator==(const Address&rhs) const;
    bool operator!=(const Address&rhs) const;
};

//IP类的基地址
class IPAddress:public Address{
public:
    typedef std::shared_ptr<IPAddress> ptr;
    /**
     * @brief 通过域名,IP,服务器名创建IPAddress
     * @param[in] address 域名,IP,服务器名等.举例: www.sylar.top
     * @param[in] port 端口号
     * @return 调用成功返回IPAddress,失败返回nullptr
     */
    static IPAddress::ptr Create(const char* address, uint16_t port = 0);
    // 获取该地址的广播地址 prefix_len 子网掩码位数
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    //获取该IP地址的网段
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
    //获取该IP地址的子网掩码地址
     virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;
    virtual uint32_t getPort() const=0;
    virtual void setPort(uint16_t v) =0;

};
class IPv4Address : public IPAddress {
public:
    using ptr=std::shared_ptr<IPv4Address>;
    /**
     * @brief 使用点分十进制地址创建IPv4Address
     * @param[in] address 点分十进制地址,如:192.168.1.1
     * @param[in] port 端口号
     * @return 返回IPv4Address,失败返回nullptr
     */
    //inet_addr()作用是将一个IP字符串转化为一个网络字节序的整数值，用于sockaddr_in.sin_addr.s_addr。
    static IPv4Address::ptr Create(const char* address, uint16_t port = 0);
    IPv4Address(const sockaddr_in& address);

    /**
     * @brief 通过二进制地址构造IPv4Address
     * @param[in] address 二进制地址address
     * @param[in] port 端口号
     */
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in m_addr;
};
//还有IPV6没弄
class IPv6Address : public IPAddress {
public:
    using ptr=std::shared_ptr<IPv6Address>;
    /**
     * @brief 使用点分十进制地址创建IPv4Address
     * @param[in] address 点分十进制地址,如:192.168.1.1
     * @param[in] port 端口号
     * @return 返回IPv4Address,失败返回nullptr
     */
    //inet_addr()作用是将一个IP字符串转化为一个网络字节序的整数值，用于sockaddr_in.sin_addr.s_addr。
    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);
    IPv6Address(const sockaddr_in6& address);

    /**
     * @brief 通过二进制地址构造IPv4Address
     * @param[in] address 二进制地址address
     * @param[in] port 端口号
     */
    IPv6Address();
    IPv6Address(const uint8_t address[16], uint16_t port = 0);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in6 m_addr;
};
//unix socket 又称本地套接字，用于系统内的进程间通信
// struct sockaddr_un {
//    sa_family_t sun_family;               /* AF_UNIX */
//    char        sun_path[108];            /* Pathname */
// };
class UnixAddress :public Address{
public:
    using ptr=std::shared_ptr<UnixAddress>;
    UnixAddress();
    
    /**
     * @brief 通过路径构造UnixAddress
     * @param[in] path UnixSocket路径(长度小于UNIX_PATH_MAX)
     */

    UnixAddress(const std::string& path);
    const sockaddr* getAddr() const override;

    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    void setAddlen(uint32_t v);
    std::string getPath() const;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr_un m_addr;
    socklen_t m_length;
};



/**
 * @brief 未知地址
 */
class UnknownAddress : public Address {
public:
    typedef std::shared_ptr<UnknownAddress> ptr;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr m_addr;
};

std::ostream& operator<<(std::ostream& os, const Address& addr);


}
#endif