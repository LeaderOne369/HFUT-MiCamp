cmake_minimum_required(VERSION 3.10)
project(TCPTimeApp VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置Qt路径 (Mac homebrew安装的位置)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")

# 查找Qt包
find_package(Qt6 COMPONENTS Widgets Network REQUIRED)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
endif()

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# 服务端源文件
set(SERVER_SOURCES
    src/server.cpp
    src/socket.cpp
    src/server_window.cpp
    src/server_window.h
)

# 客户端源文件
set(CLIENT_SOURCES
    src/client.cpp
    src/socket.cpp
    src/client_window.cpp
    src/client_window.h
)

# 创建服务端可执行文件
add_executable(server ${SERVER_SOURCES})
if(Qt6_FOUND)
    target_link_libraries(server PRIVATE Qt6::Widgets Qt6::Network)
else()
    target_link_libraries(server PRIVATE Qt5::Widgets Qt5::Network)
endif()

# 创建客户端可执行文件
add_executable(client ${CLIENT_SOURCES})
if(Qt6_FOUND)
    target_link_libraries(client PRIVATE Qt6::Widgets Qt6::Network)
else()
    target_link_libraries(client PRIVATE Qt5::Widgets Qt5::Network)
endif()

# Mac专用：设置为GUI应用
if(APPLE)
    set_target_properties(server PROPERTIES
        MACOSX_BUNDLE TRUE
    )
    set_target_properties(client PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

# 安装目标
install(TARGETS server client DESTINATION bin) 