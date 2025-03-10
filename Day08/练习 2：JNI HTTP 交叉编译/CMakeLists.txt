cmake_minimum_required(VERSION 3.10)
project(HttpLibrary)

# 设置 Android NDK 交叉编译
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 21) # Android API level
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(CMAKE_ANDROID_NDK ${ANDROID_NDK})
set(CMAKE_ANDROID_STL_TYPE c++_shared)

# 添加 HTTP 库依赖
find_package(CURL REQUIRED)

# 创建共享库
add_library(httplib SHARED
    src/http_wrapper.cpp
)

target_link_libraries(httplib
    PRIVATE
    CURL::libcurl
) 