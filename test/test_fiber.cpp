#include "qlc.h"
qlc::Logger::ptr g_logger =QLC_LOG_NAME("fiber");
void run_fiber(){
    QLC_LOG_INFO(g_logger)<<"second"<<std::endl;
    qlc::Fiber::YieldToHold();
    QLC_LOG_INFO(g_logger)<<"fourth"<<std::endl;
    qlc::Fiber::YieldToHold();
    QLC_LOG_INFO(g_logger)<<"six"<<std::endl;
    qlc::Fiber::YieldToHold();

}
void run_in_fiber() {
    QLC_LOG_INFO(g_logger) << "run_in_fiber begin";
    qlc::Fiber::YieldToHold();
    QLC_LOG_INFO(g_logger) << "run_in_fiber end";
    qlc::Fiber::YieldToHold();
}
void test_fiber() {
    QLC_LOG_INFO(g_logger) << "main begin -1";
    {
        qlc::Fiber::GetThis();
        QLC_LOG_INFO(g_logger) << "main begin";
        qlc::Fiber::ptr fiber(new qlc::Fiber(run_fiber));
        fiber->swapIn();
        QLC_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        QLC_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    QLC_LOG_INFO(g_logger) << "main after end2";
}
int main(int argc,char ** agrv){
    qlc::Thread::SetName("main");
    std::vector<qlc::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        thrs.push_back(qlc::Thread::ptr(
                    new qlc::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}