cmake_minimum_required(VERSION 3.10)
project(http_test VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加libghttp子项目
add_subdirectory(libghttp)

# 源文件
set(SOURCES
    main.cpp
    src/get.cpp
    src/download.cpp
)

# 头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 创建可执行文件
add_executable(http_test ${SOURCES})

# 链接动态库
target_link_libraries(http_test PRIVATE ghttp)

# 安装规则
install(TARGETS http_test
    RUNTIME DESTINATION bin
)
