#include <iostream>
#include <pthread.h>
#include "qlc_log.h"
//ntpdate ntp.aliyun.com 同步系统时间的命令
int main(int argc,char* argv[]){
    qlc::Logger::ptr logger(new qlc::Logger);
    qlc::LogAppender::ptr fileappender(new qlc::fileAppender("./log.txt"));
    fileappender->setFormatter(qlc::LogFormatter::ptr(new qlc::LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T[%c]%T%f [%p] %m %F %n")));
    //
    logger->addAppender(fileappender);
    QLC_LOG_DEBUG(logger)<<"这是用宏定义来输入的";
    QLC_LOG_DEBUG(logger)<<"这是第二行测试";
    auto l=qlc::Loggermgr::GetInstanceX()->getLogger("aa");
    qlc::LogAppender::ptr stdoutappender(new qlc::stdoutAppender());
    stdoutappender->setFormatter(qlc::LogFormatter::ptr(new qlc::LogFormatter("%f %m %n")));
    // std::cout<<stdoutappender->getFormatter()<<std::endl;
    l->addAppender(stdoutappender);
    QLC_LOG_DEBUG(l)<<"这是第管理器测试";
    QLC_LOG_FMT_DEBUG(l,"这是一个测试文件%s","dsadsadas");
    // std::cout<<__FILE__;
    // qlc::LogEvent::ptr event(new qlc::LogEvent(__FILE__,__LINE__,0,qlc::GetThreadId(),qlc::GetFiberId(),time(0)));//std::this_thread::get_id() 类型不对
    // event->getSS()<<"这是一个测试的文本";
    // logger->Log(qlc::LogLevel::ERROR,event);
    // std::cout<<"hello world"<<std::endl;
    std::string str = "Hello, orld!";
    std::string charset = "Helo, ";
    
    size_t found = str.find_first_not_of(charset);
    if (found != std::string::npos) {
        std::cout << "First character not found in charset is: " << str[found] << std::endl;
    } else {
        std::cout << "All characters found in charset." << std::endl;
    }
    
 
    return 0;

}