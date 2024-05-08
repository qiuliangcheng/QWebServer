#include "util.h"
#include "qlc_log.h"
#include <execinfo.h>

namespace qlc{
    qlc::Logger::ptr g_logger=QLC_LOG_NAME("system");
    pid_t GetThreadId(){
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberId(){
        return 0;
    }

    void Backtrace(std::vector<std::string> &bt, int size, int skip)
    {
        void ** array=(void **)malloc(sizeof(void*)*size);//第一个指针保存栈的地址 第二个指针保存栈的信息 所以需要二级指针
        size_t len=::backtrace(array,size);
        char** stings=backtrace_symbols(array,len);
        if(stings==NULL){
            QLC_LOG_ERROR(g_logger)<<"获取函数栈信息错误";
            return;

        }
        for(size_t i=skip;i<len;i++){
            bt.push_back(stings[i]);
        }
        free(stings);
        free(array);

    }
    std::string BacktraceToString(int size, int skip, const std::string &prefix)
    {
        std::vector<std::string> vec;
        Backtrace(vec,size,skip);
        std::stringstream ss;
        for(size_t i=0;i<vec.size();i++){
            ss<<prefix<<vec[i]<<std::endl;
        }
        return ss.str();
    }
}
