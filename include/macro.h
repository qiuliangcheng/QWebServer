#ifndef __QLC_MACRO_H
#define __QLC_MACRO_H
#include "util.h"
#include <string.h>
#include <assert.h>

#define QLC_ASSERT(x) \
    if(!x){ \
        QLC_LOG_ERROR(QLC_LOG_ROOT())<<"ASSERT:" #x "ERROR"<<\
        "\n bracetrace: \n" \
        <<qlc::BacktraceToString(64,2,"    "); \
        assert(x);\
    }
#define QLC_ASSERT2(x,m) \
    if(!x){ \
        QLC_LOG_ERROR(QLC_LOG_ROOT())<<"ASSERT:" #x "ERROR"<<\
        <<"\n" <<m <<\
        "\n bracetrace: \n" \
        <<qlc::BacktraceToString(64,2,"    "); \
        assert(x);\
    }


#endif