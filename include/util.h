#ifndef __QLC_UTIL_H__
#define __QLC_UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <string>
#include "qlc_log.h"

namespace qlc{
    pid_t GetThreadId();
    uint32_t GetFiberId();
    //将栈信息保存再bt数组里 size 是能返回的最大的栈的数量 
    void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");
    uint64_t GetCurrentMS();
    uint64_t GetCurrentUS();

}


#endif