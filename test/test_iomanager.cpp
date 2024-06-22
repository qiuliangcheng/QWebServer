#include "qlc.h"
#include "iomanager.h"
#include <sys/types.h>          
#include <sys/socket.h>
 #include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
int sock = 0;
qlc::Logger::ptr g_logger =QLC_LOG_NAME("system");
void test_fiber(){
    QLC_LOG_INFO(g_logger)<<"fiber_test";
}
void test1(){
    //直接运行这个函数会出问题 因为主线程再运行add的时候 他用到了调度器的getthis 但是这个时候他是为空的
    qlc::IOManager iom(2,false);
    int sock=socket(AF_INET,SOCK_STREAM,0);
    fcntl(sock,F_SETFL,O_NONBLOCK);
    sockaddr_in addr;
    memset(&addr,0,sizeof(sockaddr_in));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(80);
    inet_pton(AF_INET, "220.181.38.149", &addr.sin_addr.s_addr);//应该是去连接这个地址吧
    //连上了就对这个fd添加一个写的事件
    iom.addEvent(sock,qlc::IOManager::WRITE,[](){
        QLC_LOG_INFO(g_logger)<<"连接";
    });

}
void test2(){
    sock=socket(AF_INET,SOCK_STREAM,0);
    fcntl(sock,F_SETFL,O_NONBLOCK);
    sockaddr_in addr;
    memset(&addr,0,sizeof(sockaddr_in));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(80);
    inet_pton(AF_INET, "220.181.38.149", &addr.sin_addr.s_addr);//应该是去连接这个地址吧
    //连上了就对这个fd添加一个写的事件
    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        QLC_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        qlc::IOManager::GetThis()->addEvent(sock, qlc::IOManager::READ, [](){
            QLC_LOG_INFO(g_logger) << "read callback";
        });
        qlc::IOManager::GetThis()->addEvent(sock, qlc::IOManager::WRITE, [](){
            QLC_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            qlc::IOManager::GetThis()->cancelEvent(sock, qlc::IOManager::READ);
            close(sock);
        });
    } else {
        QLC_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}
void test(){
    qlc::IOManager iom(2,false,"test");
    iom.schedule(&test2);

}
qlc::Timer::ptr s_timer;
void test_timer(){
    qlc::IOManager iom(2,false,"test");
    s_timer=iom.addTimer(1000,[](){
        static int i = 0;
        QLC_LOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 3) {
            s_timer->reset(2000, false);
        }
    },true);

}



int main(int agrc,char** argv){
    test_timer();
    return 0;
}