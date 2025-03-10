# 设置最低 CMake 版本要求
cmake_minimum_required(VERSION 3.13)

# 设置项目名称
project(HelloWorld)

# 设置 C++ 标准为 C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(hello
    hello.cpp
    hello.h
) 