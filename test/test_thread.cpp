#include "qlc_log.h"
#include "thread.h"
#include <unistd.h>
static qlc::Logger::ptr g_logger = QLC_LOG_NAME("root");
void fun1(){
    QLC_LOG_DEBUG(g_logger)<<"线程名字: "<<qlc::Thread::GetName()<<"  "<< "线程类的名字:  " << qlc::Thread::GetThis()->getName()<<"  "<<
        "线程id:  " <<qlc::GetThreadId() <<"  "<<"线程类的id:  "<<qlc::Thread::GetThis()->getId();
    sleep(20);
}
int main(int argc,char **argv){
    QLC_LOG_DEBUG(g_logger)<<"线程开始了";
    std::vector<qlc::Thread::ptr> vec;
    for(int i=0;i<5;i++){
        qlc::Thread::ptr m_thread(new qlc::Thread(&fun1,"线程"+std::to_string(i)));
        vec.push_back(m_thread);
    }
    for(int i=0;i<5;i++){
        vec[i]->join(); 
    }
    QLC_LOG_DEBUG(g_logger)<<"线程结束了";
    return 0;
}