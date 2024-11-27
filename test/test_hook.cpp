//#include "hook.h"
#include "qlc.h"
#include "iomanager.h"
 #include <unistd.h>
qlc::Logger::ptr g_logger =QLC_LOG_NAME("system");
void test_sleep(){
    qlc::IOManager iom(1);
    iom.schedule([](){
        sleep(2);
        QLC_LOG_INFO(g_logger)<<"sleep 2";

    });
    iom.schedule([](){
        sleep(3);
        QLC_LOG_INFO(g_logger)<<"sleep 3";

    });
    // while(1){}
    QLC_LOG_INFO(g_logger)<<"sleep";

}

int main(int argc,char** argv)
{
    test_sleep();
    return 0;
}