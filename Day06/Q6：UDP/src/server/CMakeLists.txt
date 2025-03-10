set(SERVER_SOURCES
    server.cpp
)

set(CMAKE_AUTOMOC ON)

add_executable(server_qt ${SERVER_SOURCES})

target_include_directories(server_qt PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(server_qt PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
) 