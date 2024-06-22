#include "qlc.h"
static qlc::Logger::ptr g_logger=QLC_LOG_ROOT();
void test_fiber() {
    static int s_count = 5;
    QLC_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        qlc::Schedular::GetThis()->schedule(&test_fiber);
    }
}
int main(int argc,char **argv){
    QLC_LOG_INFO(g_logger) << "main";
    qlc::Schedular sc(3, false, "test");
    sc.start();
    sleep(2);
    QLC_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber);
    sc.stop();
    QLC_LOG_INFO(g_logger) << "over";
    return 0;
}