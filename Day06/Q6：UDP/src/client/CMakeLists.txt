set(CLIENT_SOURCES
    client.cpp
)

set(CMAKE_AUTOMOC ON)

add_executable(client_qt ${CLIENT_SOURCES})

target_include_directories(client_qt PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(client_qt PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
) 