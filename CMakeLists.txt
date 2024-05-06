cmake_minimum_required(VERSION 2.8)
project(qlc)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(/usr/local/include)
include_directories(./include)
set(LIB_SRC
    src/qlc_log.cpp
    src/util.cpp
    src/config.cpp
    )
find_library(YAMLCPP yaml-cpp)
add_library(qlc SHARED ${LIB_SRC})
add_executable(test test/test.cpp)
add_dependencies(test qlc)
target_link_libraries(test qlc ${YAMLCPP})

add_executable(test_config test/test_config.cpp)
add_dependencies(test_config qlc)
target_link_libraries(test_config qlc ${YAMLCPP})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
