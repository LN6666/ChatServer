#cmake版本号
cmake_minimum_required(VERSION 3.0)
project(main)
#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
#配置头文件搜索路径
#include_directories() 
#配置库文件搜索路径
#link_directories()
#设置需要编译的源文件列表
set(SRC_LIST muduo_server.cpp)
#设置可执行文件最终存储的路径,PROJECT_SOURCE_DIR工程的根目录长度b
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_executable(server ${SRC_LIST})
#把.指定路径下的源文件名字放入变量名SPC_LIST里面
#aux_source_directory(. SRC_LIST)
#add_executable(server muduo_server.cpp)#生成可执行文件
#add_library()生成库文件
target_link_libraries(server muduo_net muduo_base pthread)
#链接库,表示server这个目标程序需要链接三个库文件