cmake_minimum_required(VERSION 3.14)

project(UDP_Protocol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置Qt组件
find_package(Qt6 COMPONENTS Core Widgets Network REQUIRED)

# 包含子目录
add_subdirectory(src/client)
add_subdirectory(src/server) 