#include <iostream>
#include "macro.h"
void fun1(){
    QLC_LOG_ERROR(QLC_LOG_ROOT())<<qlc::BacktraceToString(64,2,"    ");
    QLC_ASSERT(false);
}
int main(int argc,char** argv){
    fun1();
    return 0;
}