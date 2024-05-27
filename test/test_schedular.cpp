#include "qlc.h"
static qlc::Logger::ptr g_logger=QLC_LOG_ROOT();
void test(){
    QLC_LOG_INFO(g_logger)<<"test——in-fiber";
}
int main(int argc,char **argv){
    QLC_LOG_INFO(g_logger)<<"main";
    qlc::Schedular sc;
    sc.start();//开始跑了
    sc.schedule(&test);

    QLC_LOG_INFO(g_logger)<<"schedular";
    
    sc.stop();
    QLC_LOG_INFO(g_logger)<<"end";
    return 0;
}