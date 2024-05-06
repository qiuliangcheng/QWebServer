#ifndef __QLC_UTIL_H__
#define __QLC_UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <stdio.h>
namespace qlc{
    pid_t GetThreadId();


    uint32_t GetFiberId();

}


#endif