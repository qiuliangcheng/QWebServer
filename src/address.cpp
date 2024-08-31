#include "address.h"
#include "endian.h"
#include "qlc.h"
#include <netdb.h>
#include <ifaddrs.h>
#include <stddef.h>
#include <sstream>
namespace qlc{
static qlc::Logger::ptr g_logger=QLC_LOG_NAME("system");
template<class T>
static T CreateMask(uint32_t bits){
    return 1<<(sizeof(T)*8-bits)-1;//这样高位的bits位全是1
}
//统计1的个数的算法
template<class T>
static uint32_t CountBytes(T value) {
    uint32_t result = 0;
    for(; value; ++result) {//如果value不是全部为0就一直往下走
        value &= value - 1;//这样一弄 value肯定会少一个1的个数
    }
    return result;
}

Address::ptr Address::Create(const sockaddr *addr, socklen_t len)
{
    if(addr==nullptr){
        return nullptr;
    }
    Address::ptr res;
    switch(addr->sa_family){
        case AF_INET:
            res.reset(new IPv4Address(*(const sockaddr_in*)addr));//让智能指针指向一个新的对象
            break;
        case AF_INET6:
            res.reset(new IPv6Address(*(const sockaddr_in6*)addr));
            break;
        default:
            res.reset(new UnknownAddress(*(const sockaddr*)addr));
            break;
    }
    return res;
}

bool Address::Lookup(std::vector<Address::ptr> &result, const std::string &host, int family, int type, int protocol)
{
    //int getaddrinfo( const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result );
    addrinfo hints, *results, *next;
    //在getaddrinfo函数之前通常需要对以下6个参数进行以下设置：nodename、servname、hints的ai_flags、ai_family、ai_socktype、ai_protocol。
    hints.ai_flags = 0;
    hints.ai_protocol=protocol;
    hints.ai_family=family;
    hints.ai_socktype=type;
    hints.ai_addr=NULL;
    hints.ai_addrlen=NULL;
    hints.ai_next = NULL;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    std::string node;
    const char* service = NULL; 
    //检查ipv6addr的server端
    //Pv6 地址从 IPv4 地址的 32 bits 扩展到 128 bits，IPv6 地址的表示、书写方式也从 IPv4 的点分十进制，如 192.168.1.1，转变为：
    //fe80:0000:0001:0000:0440:44ff:1233:5678
    //一般的格式会是 [ipv6的地址]
    // 即，16 bits 一组，采用 16 进制表示，共 8 组字段，每个字段之间使用 “:” 分隔。
    // 为了方便书写、表示 IPv6 地址，IETF 在标准中规定了 IPv6 的规范文本表示形式：
    // 抑制每个字段中的前导零
    // 使用双冒号 “::” 替换连续全零的字段，注意一个 IPv6 地址只能压缩一个零字段序列
    // 如果一个 IPv6 地址中有多个连续全零字段，只压缩最长的零字段序列
    // 如果一个 IPv6 地址中有多个相等长度的序列，则只压缩第一个
    // 不允许压缩单个零字段
    // 十六进制中的 a, b, c, d, e, f 必须是小写的
    if(!host.empty()&&host[0]=='['){
        //函数原型：void *memchr(const void *buf，int c，sizet count)；
        //参数：buf 缓冲区的指针；c 查找的字符；count 检查的字符个数。
        //返回值：如果成功，返回 buf 中 c 首次出现的位置的指针
        const char *endipv6=(const char*)memchr(host.c_str()+1,']',host.size()-1);
        if(endipv6){//说明找到了
            if(*(endipv6+1)==':'){//[23131]://后面应该是端口吧 在后面跟着‘：’带上端口号，
            //如 [A01F::0]:8000，
                service=endipv6+2;
            }
            node=host.substr(1,endipv6-host.c_str()-1);
        }
    }         
    //检查 node serivce 
    //接下来，代码再次使用 memchr 函数从 service + 1（即第一个冒号之后的位置）开始查找是否还有另一个冒号。如果在剩余的字符串中没有找到第二个冒号，
    //这意味着 host 字符串格式为 node:service 而不是 IPv6 地址（IPv6 地址中会包含多个冒号）
    if(node.empty()) {///host = "example.com:8080"
        service = (const char*)memchr(host.c_str(), ':', host.size());
        if(service) {
            if(!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                node = host.substr(0, service - host.c_str());
                ++service;
            }//node 被设置为 host 从开始到第一个冒号之前的子字符串。这是通过 substr 函数实现的，它接收两个参数：子字符串的起始位置（在这个例子中是 0）和长度（service - host.c_str()，即第一个冒号的位置减去字符串的开始位置）。
        }//host = "example.com:8080"
        // node 是一个空字符串（假设初始为空）
        // service 初始为 NULL

    }   
    if(node.empty()) {//那就是纯网址了 一个冒号没有 或者是ipv4的地址
        node = host;
    }
    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if(error) {
        QLC_LOG_DEBUG(g_logger) << "Address::Lookup getaddress(" << host << ", "
            << family << ", " << type << ") err=" << error << " errstr="
            << gai_strerror(error);
        return false;
    }
    next = results;
    while(next) {
        result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
        next = next->ai_next;
    }
    freeaddrinfo(results);
    return !result.empty();
}

Address::ptr Address::LookupAny(const std::string &host, int family, int type, int protocol)
{
    std::vector<Address::ptr> result;
    if(Lookup(result,host,family,type,protocol)){
        return result[0];
    }
    return nullptr;
}
std::shared_ptr<IPAddress> Address::LookupAnyIPAddress(const std::string &host, int family, int type, int protocol)
{
    std::vector<Address::ptr> results;
    if(Lookup(results,host,family,type,protocol)){
        for(auto &i:results){
            IPAddress::ptr v=std::dynamic_pointer_cast<IPAddress>(i);//它类似于 C++ 里的 dynamic_cast，但专门用于智能指针。
            //shared_ptr<Derived> derived_ptr = dynamic_pointer_cast<Derived>(base_ptr);
            //多态性：需要基类和派生类之间存在多态关系，即基类中至少有一个虚函数
            if(v){
                return v;
            }
        }
    }
    return nullptr;
}

bool Address::GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t>> &result, int family)
{
    ifaddrs *results,*next;
    if(getifaddrs(&results)!=0){
        QLC_LOG_DEBUG(g_logger) << "Address::GetInterfaceAddresses getifaddrs "
            " err=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    //int getifaddrs(struct ifaddrs **ifap);
    //void freeifaddrs(struct ifaddrs *ifa);
    try{
        for(next=results;next!=NULL;next=next->ifa_next){
            Address::ptr addr;
            uint32_t prefix_len=~0u;
            if(family!=AF_UNSPEC&&family!=next->ifa_addr->sa_family){
                continue;//说明不是自己想要的那个网卡地址
            }
            switch(next->ifa_addr->sa_family) {
                case AF_INET://IPV4
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                        uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;//网卡掩码的地址
                        prefix_len = CountBytes(netmask);//eth0 192.168.10.10 netmask 255.255.255.0 
                    }
                    break;
                case AF_INET6:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                        in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                        prefix_len = 0;
                        for(int i = 0; i < 16; ++i) {
                            prefix_len += CountBytes(netmask.s6_addr[i]);//8*16
                        }
        //         struct in6_addr
        // {
        //     union
        //     {
        //         uint8_t __u6_addr8[16];   // 128 bit
        //         #if defined __USE_MISC || defined __USE_GNU
        //         uint16_t __u6_addr16[8];  // 64 bit
        //         uint32_t __u6_addr32[4];  // 32 bit
        //         #endif
        //     } __in6_u;
        //     #define s6_addr         __in6_u.__u6_addr8
        //     #if defined __USE_MISC || defined __USE_GNU
        //     # define s6_addr16      __in6_u.__u6_addr16
        //     # define s6_addr32      __in6_u.__u6_addr32
        //     #endif
        // };

        //  in_addr 结构体（ipv4）和 in6_addr结构体（ipv6）
        // 1）struct in_addr 结构体：表示一个32位的IPv4地址；
        //         struct in_addr {
        //                 in_addr_t s_addr;    //in_addr_t一般为32位的unsigned int，其字节顺序为网络字节序，即该无符号数采用大端字节序；其中每8位表示一个IP地址中的一个数值；
        //         };
        // 2）struct in6_addr结构体：表示一个32位的十六进制的IPv6地址；

                    }
                    break;
                default:
                    break;
            }
            if(addr) {
                result.insert(std::make_pair(next->ifa_name,
                            std::make_pair(addr, prefix_len)));
            }

        }


    }catch(...){//// 捕获所有其他类型的异常
        QLC_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses exception";
        freeifaddrs(results);
        return false;
    }
    freeifaddrs(results);
    return !result.empty();
}

bool Address::GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t>> &result, const std::string &iface, int family)
{
    if(iface.empty() || iface == "*") {//*代表所有
        if(family == AF_INET || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address::ptr(new IPv4Address()), 0u));
        }
        if(family == AF_INET6 || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address::ptr(new IPv6Address()), 0u));
        }
        return true;
    }
    std::multimap<std::string, std::pair<Address::ptr, uint32_t>> results;
    if(!GetInterfaceAddresses(results,family)){
        return false;
    }
    // lower_bound.(k)	返回一个迭代器，指向第一个值不小于k的元素
    // upper_bound(k)	返回一个迭代器，指向第一个值大于k的元素
    // equal_range(k)	返回一个迭代器pair,表示值等于k的元素的范围。若k不存在，pair两个成员均等于end()–尾迭代器
    auto its=results.equal_range(iface);
    for(; its.first != its.second; ++its.first) {
        result.push_back(its.first->second);
    }
    return !result.empty();}
int Address::getFamily() const
{
    return getAddr()->sa_family;
}

std::string Address::toString() const
{
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address &rhs) const
{
    //有可能是ipv4和ipv6的对比
    socklen_t minlen=std::min(getAddrLen(),rhs.getAddrLen());
    int res=memcmp(getAddr(),rhs.getAddr(),minlen);
    if(res<0){
        return true;
    }else if(res>0){
        return false;
    }else if(getAddrLen()<rhs.getAddrLen()){
        return true;
    }
    return false;
}
bool Address::operator==(const Address &rhs) const
{    
    return getAddrLen()==rhs.getAddrLen()&&memcmp(getAddr(),rhs.getAddr(),getAddrLen());
}
bool Address::operator!=(const Address &rhs) const
{
    return !(*this==rhs);
}

IPAddress::ptr IPAddress::Create(const char *address, uint16_t port)
{
    addrinfo hints,*result;
    memset(&hints,0,sizeof(hints));
    hints.ai_flags=AI_NUMERICHOST;//这告诉 getaddrinfo 函数 address 参数是一个数值地址（即不是域名）。
    hints.ai_family=AF_UNSPEC;//这意味着 getaddrinfo 函数可以为 address 参数返回任何协议族（IPv4 或 IPv6）的地址信息。
    int error=getaddrinfo(address,NULL,&hints,&result);
    if(error){
        QLC_LOG_DEBUG(g_logger) << "IPAddress::Create(" << address
            << ", " << port << ") error=" << error
            << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    try{
        IPAddress::ptr res=std::dynamic_pointer_cast<IPAddress>(Address::Create(result->ai_addr,(socklen_t)result->ai_addrlen));
        //create会自己创建不同的ipADDRESS
        if(res){
            res->setPort(port);
        } 
        freeaddrinfo(result);
        return res;
    }catch(...){
        QLC_LOG_ERROR(g_logger) << "IPAddress::Create exception";
        freeaddrinfo(result);
        return nullptr;
    }
    return nullptr;
}

IPv4Address::ptr IPv4Address::Create(const char *address, uint16_t port)
{
    // 新型网路地址转化函数inet_pton和inet_ntop
    // 这两个函数是随IPv6出现的函数，对于IPv4地址和IPv6地址都适用，函数中p和n分别代表表达（presentation)和数值（numeric)。
    // 地址的表达格式通常是ASCII字符串，数值格式则是存放到套接字地址结构的二进制值。
    IPv4Address::ptr res(new IPv4Address());
    res->m_addr.sin_port=byteswapOnLittleEndian(port);
    res->m_addr.sin_family=AF_INET;
    // int inet_pton(int family, const char *strptr, void *addrptr);     //将点分十进制的ip地址转化为用于网络传输的数值格式
    // 返回值：若成功则为1，若输入不是有效的表达式则为0，若出错则为-1
    
    // const char * inet_ntop(int family, const void *addrptr, char *strptr, size_t len);     //将数值格式转化为点分十进制的ip地址格式

    int rt=inet_pton(AF_INET,address,&res->m_addr.sin_addr);
    if(rt<0){
        QLC_LOG_DEBUG(g_logger)<<"IPv4Address::Create(" << address << ", "
                << port << ") rt=" << rt << " errno=" << errno
                << " errstr=" << strerror(errno);
        return nullptr;
    }

    return res;
}

IPv4Address::IPv4Address(const sockaddr_in &address)
{
    m_addr=address;
}
IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = byteswapOnLittleEndian(port);
    m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
}

const sockaddr *IPv4Address::getAddr() const
{
    return (sockaddr*)&m_addr;
}

sockaddr *IPv4Address::getAddr()
{
    return (sockaddr*)&m_addr;
}

socklen_t IPv4Address::getAddrLen() const
{
    return sizeof(m_addr);
}

std::ostream &IPv4Address::insert(std::ostream &os) const
{
    // TODO: insert return statement here
    uint32_t addr=m_addr.sin_addr.s_addr;
    os<<((addr>>24)&0xff)<<"."
        <<((addr>>16)&0xff)<<"."
        <<((addr>>8)&0xff)<<"."
        <<((addr)&0xff);
    os<<":"<<byteswapOnLittleEndian(m_addr.sin_port);
    return os;

}

IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len)//广播地址
{
    if(prefix_len>32){
        return nullptr;
    }
    sockaddr_in tmp(m_addr);
    tmp.sin_addr.s_addr|=byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    IPAddress::ptr res(new IPv4Address(tmp));
    return res;
}
//网关地址
IPAddress::ptr IPv4Address::networdAddress(uint32_t prefix_len)
{
    if(prefix_len > 32) {
        return nullptr;
    }

    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr &= ~byteswapOnLittleEndian(
            CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
}

IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::getPort() const
{
    return byteswapOnLittleEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t v)
{
    m_addr.sin_port=byteswapOnLittleEndian(v);
}

IPv6Address::ptr IPv6Address::Create(const char *address, uint16_t port)
{
    IPv6Address::ptr res(new IPv6Address());
    res->m_addr.sin6_port=byteswapOnLittleEndian(port);
    int error=inet_pton(AF_INET6,address,&res);
    if(error<0){
        QLC_LOG_ERROR(g_logger)<<"IPV6Adress Create ("<<address<<","<<port<<")"<<"error="<<error
        <<"strerror="<<strerror(errno);
        return nullptr;
    }
    return nullptr;
}

IPv6Address::IPv6Address(const sockaddr_in6 &address)
{
    m_addr=address;
}

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port)
{
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sin6_port=byteswapOnLittleEndian(port);
    m_addr.sin6_family=AF_INET6;
    memcpy(&m_addr.sin6_addr.s6_addr,address,16);//将连续的n个字节数据
}

const sockaddr *IPv6Address::getAddr() const
{
    return (sockaddr*)&m_addr;
}

sockaddr *IPv6Address::getAddr()
{
    return (sockaddr*)&m_addr;
}
socklen_t IPv6Address::getAddrLen() const
{
    return socklen_t(sizeof(m_addr));
}

std::ostream &IPv6Address::insert(std::ostream &os) const
{
    // TODO: insert return statement here
// 每组前面的0的可以省略不写
// 完整：2403:A200:A200:1100:0000:0000:0F00:0003
// 简写：2403:A200:A200:1100:0000:0000:F00:3
// 连续为0的一组或多组，可以用 :: 代替，注意一个IP中只能用一次 ::
// 完整：2403:A200:A200:1100:0000:0000:0F00:0003
// 简写：2403:A200:A200:1100::F00:3
// 如果一组中全为0，又不想用 :: 代替，每组中要保留1个0
// 完整：2403:A200:A200:0000:AFFF:0000:0000:0003
// 简写：2403:A200:A200:0:AFFF::3
    os<<"[";
    uint16_t *data=(uint16_t*)&m_addr.sin6_addr.s6_addr;
    bool used_Zero=false;
    for(int i=0;i<8;i++){
        if(data[i]==0&&!used_Zero){//useZero 是要不要对零进行缩写 false是要因为不使用那么多零
            continue;
        }
        if(i&&data[i-1]==0&&!used_Zero){
            os<<":";//只使用这一次 就算后面再长 也不用了
            used_Zero=true;
        }
        if(i){
            os<<":";
        }
        os<<std::hex<<(int)byteswapOnLittleEndian(data[i])<<std::dec;//先转成16进制 然后转换成十进制
    }
    if(!used_Zero&&data[7]==0){
        //最后就变成了：0：：了
        os<<"::";
    }
    os<<"]:"<<byteswapOnLittleEndian(m_addr.sin6_port);
    return os;
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len)
{
    sockaddr_in6 tmp(m_addr);
    tmp.sin6_addr.s6_addr[prefix_len/8]|=CreateMask<uint8_t>(prefix_len%8);
    for(int i=prefix_len/8+1;i<16;i++){
        tmp.sin6_addr.s6_addr[i]|=0xff;
    }
    IPv6Address::ptr res(new IPv6Address(tmp));
    return res;
}

IPAddress::ptr IPv6Address::networdAddress(uint32_t prefix_len)
{
    sockaddr_in6 tmp(m_addr);
    tmp.sin6_addr.s6_addr[prefix_len/8]&=~CreateMask<uint8_t>(prefix_len%8);
    for(int i=prefix_len/8+1;i<16;i++){
        tmp.sin6_addr.s6_addr[i]|=0x00;
    }
    IPv6Address::ptr res(new IPv6Address(tmp));
    return res;
}
IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len)
{
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.s6_addr[prefix_len /8] =
        ~CreateMask<uint8_t>(prefix_len % 8);

    for(uint32_t i = 0; i < prefix_len / 8; ++i) {
        subnet.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(subnet));
}

uint32_t IPv6Address::getPort() const
{
    return byteswapOnLittleEndian(m_addr.sin6_port);
}
void IPv6Address::setPort(uint16_t v)
{
    m_addr.sin6_port=byteswapOnLittleEndian(v);
}

//struct sockaddr_un
// {
// 	uint16_t sun_family;
// 	char sun_path[108]; /* Path name. */ 
// };
static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path)-1;

UnixAddress::UnixAddress()
{
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sun_family=AF_UNIX;
    m_length=offsetof(sockaddr_un,sun_path)+MAX_PATH_LEN;

}

UnixAddress::UnixAddress(const std::string &path)
{
    memset(&m_addr,0,sizeof(m_addr));
    m_addr.sun_family=AF_UNIX;
    m_length=path.length()+1;
    if(!path.empty()&&path[0]=='\0'){
        m_length--;
    }
    if(m_length>sizeof(m_addr.sun_path)){
        //它属于运行时逻辑错误异常的基类。它表示程序中的错误，这些错误在程序逻辑上是不应该发生的，例如无效的操作、不正确的使用API等。
        throw std::logic_error("path too long");
    }
    //c_str()生成一个const char *指针
    memcpy(&m_addr.sun_path,path.c_str(),m_length);
    m_length+=offsetof(sockaddr_un,sun_path);

}
const sockaddr *UnixAddress::getAddr() const
{
    return (sockaddr*)&m_addr;
}

void UnixAddress::setAddlen(uint32_t v)
{
    m_length=v;
}
std::string UnixAddress::getPath() const
{
// 如果 sun_path[0] 等于 '\0'，这表示该 Unix 域套接字是一个抽象套接字。抽象套接字不与文件系统中的文件相对应，
// 而是仅存在于内存中，并且它们的路径名是在内存中定义的。这样的套接字路径名必须以空字符开头，
// 随后跟着实际的路径名。这意味着，抽象套接字的名字是在 sun_path 数组的第一个字符之后开始的。
    std::stringstream ss;
    if(m_length>offsetof(sockaddr_un,sun_path)&&m_addr.sun_path[0]=='\0'){
        ss << "\\0" << std::string(m_addr.sun_path + 1,
                m_length - offsetof(sockaddr_un, sun_path) - 1);
    }else{
        ss<<m_addr.sun_path;
    }
    return ss.str();
}

std::ostream &UnixAddress::insert(std::ostream &os) const
{
    // TODO: insert return statement here
    if(m_length>offsetof(sockaddr_un,sun_path)&&m_addr.sun_path[0]=='\0'){
        return os << "\\0" << std::string(m_addr.sun_path + 1,
                m_length - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os<<m_addr.sun_path;
}

UnknownAddress::UnknownAddress(int family)
{
    m_addr.sa_family=family;
}

UnknownAddress::UnknownAddress(const sockaddr &addr)
{
    m_addr=addr;
}
const sockaddr *UnknownAddress::getAddr() const
{
    return (sockaddr*)&m_addr;
}

sockaddr *UnknownAddress::getAddr()
{
    return (sockaddr*)&m_addr;
}

socklen_t UnknownAddress::getAddrLen() const
{
    return (socklen_t)sizeof(m_addr);
}

std::ostream &UnknownAddress::insert(std::ostream &os) const
{
    os << "[UnknownAddress family=" << m_addr.sa_family << "]";
    return os;
}
// 当你调用 std::cout << myAddress 时，实际上是调用了 myAddress.insert(std::cout)，
// 该方法将地址信息写入 std::cout，然后 operator<< 返回 std::cout 的引用，允许你继续链式调用其他输出操作。
// 需要注意的是，为了使这个例子正常工作，Address 类必须有一个正确实现的 insert 成员函数，
// 该函数应该将地址信息格式化并输出到传入的 std::ostream 对象中。
std::ostream& operator<<(std::ostream& os, const Address& addr) { //用全局函数去重载<<操作符
    return addr.insert(os);
}

}