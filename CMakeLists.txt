cmake_minimum_required(VERSION 2.8)
project(qlc)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
#编译优化如果加了的话多线程得不到预期的结果 比如我不加锁 他的到的还是正确的结果
include_directories(/usr/local/include)
include_directories(./include)
set(LIB_SRC
    src/qlc_log.cpp
    src/util.cpp
    src/config.cpp
    src/thread.cpp
    src/mutex.cpp
    src/fiber.cpp
    src/schedular.cpp
    src/iomanager.cpp
    src/timer.cpp
    src/hook.cpp
    src/fd_manager.cpp
    )

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)
add_library(qlc SHARED ${LIB_SRC})
set(LIB_QLC 
    qlc
    dl 
    ${YAMLCPP} 
    ${PTHREAD}
)
add_executable(test test/test.cpp)
# add_dependencies(test qlc)
target_link_libraries(test qlc ${LIB_QLC})

add_executable(test_config test/test_config.cpp)
# add_dependencies(test_config qlc)
target_link_libraries(test_config qlc ${LIB_QLC})

add_executable(test_thread test/test_thread.cpp)
# add_dependencies(test_thread qlc)
target_link_libraries(test_thread qlc ${LIB_QLC})

add_executable(test_util test/test_util.cpp)
# add_dependencies(test_thread qlc)
target_link_libraries(test_util qlc ${LIB_QLC})

add_executable(test_fiber test/test_fiber.cpp)
# add_dependencies(test_thread qlc)
target_link_libraries(test_fiber qlc ${LIB_QLC})

add_executable(test_schedular test/test_schedular.cpp)
# add_dependencies(test_thread qlc)
target_link_libraries(test_schedular qlc ${LIB_QLC})


add_executable(test_iomanager test/test_iomanager.cpp)
# add_dependencies(test_thread qlc)
target_link_libraries(test_iomanager qlc ${LIB_QLC})

add_executable(test_hook test/test_hook.cpp)
# add_dependencies(test_thread qlc)
target_link_libraries(test_hook qlc ${LIB_QLC})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
