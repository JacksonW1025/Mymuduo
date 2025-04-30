cmake_minimum_required(VERSION 2.5) #设置CMake最低版本要求
project(mymuduo) #设置项目名称
set(CMAKE_CXX_STANDARD 11) #设置C++标准为C++11

#本项目脱离boost库，直接使用C++11的线程库，直接编译成so文件
#mymuduo最终编译成so动态库，设置动态库的路径，放在根目录的lib目录下
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

#设置调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11") #设置编译选项,启动c++11标准

aux_source_directory(. SRC_LIST) #获取当前目录下的所有源文件，定义参与编译的源文件列表
add_library(mymuduo SHARED ${SRC_LIST}) #生成动态库mymuduo，包含所有源文件
