#ifndef __QLC_MACRO_H
#define __QLC_MACRO_H
#include "util.h"
#include <string.h>
#include <assert.h>
#if defined __GNUC__ || defined __llvm__//意思是：EXP==N的概率很大。
#define QLC_LIKELY(x)  __builtin_expect(!!(x),1) 
#define QLC_UNLIKELY(x) __builtin_expect(!!(x),0)
// QLC_LIKELY(x)：这是一个宏定义，它使用 __builtin_expect 指令来优化编译器的行为。如果 x 是一个很可能会为真的条件，
// 则 __builtin_expect 返回 1，告诉编译器这个条件分支在大多数情况下会被执行。
// 如果 x 是一个不太可能会为真的条件，则 __builtin_expect 返回 0，告诉编译器这个条件分支在大多数情况下不会被执行。
// QLC_UNLIKELY它与 QLC_LIKELY 相反。如果 x 是一个很可能会为假的条件，则 __builtin_expect 返回 0，
// 告诉编译器这个条件分支在大多数情况下不会被执行。如果 x 是一个不太可能会为假的条件，
// 则 __builtin_expect 返回 1，告诉编译器这个条件分支在大多数情况下会被执行。
//不太可能出现的情形用QLC_UNLIKELY
#else
#define QLC_LIKELY(x)  (x) 
#define QLC_UNLIKELY(x) (x)
#endif
#define QLC_ASSERT(x) \
    if(QLC_UNLIKELY(!(x))){ \
        QLC_LOG_ERROR(QLC_LOG_ROOT())<<"ASSERT:" #x "ERROR"<<\
        "\n bracetrace: \n" \
        <<qlc::BacktraceToString(64,2,"    "); \
        assert(x);\
    }
#define QLC_ASSERT2(x,m) \
    if(QLC_UNLIKELY(!(x))){ \
        QLC_LOG_ERROR(QLC_LOG_ROOT())<<"ASSERT:" #x "ERROR"<<\
        "\n" <<m<<\
        "\n bracetrace: \n" \
        <<qlc::BacktraceToString(64,2,"    "); \
        assert(x);\
    }//m是一些自己要加的话


#endif