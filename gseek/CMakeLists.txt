cmake_minimum_required(VERSION 2.8)
project(gseek)
# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

SET(CMAKE_CXX_COMPILER "/opt/rh/devtoolset-7/root/usr/bin/g++")

# 配置最终的可执行文件输出的路径 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/sql)

include_directories(${PROJECT_SOURCE_DIR}/include/utf)
# include_directories(${PROJECT_SOURCE_DIR}/thirdparty/uthash)

# 加载子目录
add_subdirectory(${PROJECT_SOURCE_DIR}/src)