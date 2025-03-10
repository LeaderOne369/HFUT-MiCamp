cmake_minimum_required(VERSION 3.22.1)

project(cchttp)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置 libghttp 源文件目录
set(LIBGHTTP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libghttp)

# 添加 libghttp 源文件
set(LIBGHTTP_SOURCES
    ${LIBGHTTP_DIR}/ghttp.c
    ${LIBGHTTP_DIR}/http_base64.c
    ${LIBGHTTP_DIR}/http_date.c
    ${LIBGHTTP_DIR}/http_hdrs.c
    ${LIBGHTTP_DIR}/http_req.c
    ${LIBGHTTP_DIR}/http_resp.c
    ${LIBGHTTP_DIR}/http_trans.c
    ${LIBGHTTP_DIR}/http_uri.c
)

# 创建 libghttp 静态库
add_library(ghttp STATIC
    ${LIBGHTTP_SOURCES}
)

# 设置 libghttp 头文件目录
target_include_directories(ghttp PUBLIC
    ${LIBGHTTP_DIR}
)

# 创建 JNI 库
add_library(cchttp SHARED
    http_jni.cpp
)

# 设置 cchttp 的头文件目录
target_include_directories(cchttp PRIVATE
    ${LIBGHTTP_DIR}
)

# 链接需要的库
target_link_libraries(cchttp
    ghttp
    android
    log
) 