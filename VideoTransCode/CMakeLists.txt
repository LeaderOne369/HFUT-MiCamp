cmake_minimum_required(VERSION 3.10)

project(VideoTransCode)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找pthread库
find_package(Threads REQUIRED)

# 检测操作系统
if(APPLE)
    # Mac OS X 特定配置
    set(CMAKE_C_COMPILER "/usr/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    
    # 在Mac上，我们使用brew安装的FFmpeg
    execute_process(
        COMMAND brew --prefix ffmpeg
        OUTPUT_VARIABLE FFMPEG_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    include_directories(${FFMPEG_PREFIX}/include)
    link_directories(${FFMPEG_PREFIX}/lib)
    
    set(FFMPEG_LIBRARIES
        avcodec
        avformat
        avutil
        swscale
        swresample
        avfilter
    )
elseif(UNIX AND NOT APPLE)
    # 检测是否为Ubuntu
    execute_process(
        COMMAND lsb_release -is
        OUTPUT_VARIABLE LINUX_DISTRO
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    if(LINUX_DISTRO STREQUAL "Ubuntu")
        include_directories(/usr/include)
        include_directories(/usr/include/x86_64-linux-gnu)
        
        # 使用pkg-config查找FFmpeg库
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
            libavcodec
            libavformat
            libavutil
            libswscale
            libswresample
            libavfilter
        )
        include_directories(${FFMPEG_INCLUDE_DIRS})
        link_directories(${FFMPEG_LIBRARY_DIRS})
        
        set(FFMPEG_LIBRARIES
            avcodec
            avformat
            avutil
            swscale
            swresample
            avfilter
        )
        
        # 如果存在自定义的FFmpeg库，则使用它
        set(CUSTOM_FFMPEG_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/libffmpeg_merge.so")
        if(EXISTS ${CUSTOM_FFMPEG_LIB_PATH})
            set(CUSTOM_FFMPEG_LIB ${CUSTOM_FFMPEG_LIB_PATH})
            file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
            file(COPY ${CUSTOM_FFMPEG_LIB} DESTINATION ${CMAKE_BINARY_DIR}/lib)
            set(CMAKE_INSTALL_RPATH "${CMAKE_BINARY_DIR}/lib")
            set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
            message(STATUS "使用自定义FFmpeg库: ${CUSTOM_FFMPEG_LIB}")
        else()
            message(STATUS "未找到自定义FFmpeg库，将使用系统FFmpeg库")
            set(USE_SYSTEM_FFMPEG TRUE)
        endif()
        
        # Ubuntu系统需要的额外依赖库
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2 REQUIRED sdl2)
        pkg_check_modules(X11 REQUIRED x11)
        pkg_check_modules(XCB REQUIRED xcb)
        
        include_directories(
            ${SDL2_INCLUDE_DIRS}
            ${X11_INCLUDE_DIRS}
            ${XCB_INCLUDE_DIRS}
        )
        
        set(EXTRA_LIBS
            asound
            xcb
            X11
            SDL2
            Xv
            Xext
            sndio
        )
    else()
        message(FATAL_ERROR "本程序仅支持macOS和Ubuntu。")
    endif()
else()
    message(FATAL_ERROR "本程序仅支持macOS和Ubuntu。")
endif()

# 包含当前源目录和头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 添加源文件（排除main.cpp和video_transcode.cpp）
file(GLOB COMMON_SOURCES 
    "src/*.cpp"
)
list(REMOVE_ITEM COMMON_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/video_transcode.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/audio_transcode.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/muxer_execute.cpp"
)

# 创建主可执行文件
add_executable(transcode ${COMMON_SOURCES} src/main.cpp)

# 创建视频转码可执行文件
add_executable(video_transcode ${COMMON_SOURCES} src/video_transcode.cpp)

# 创建音频转码可执行文件
add_executable(audio_transcode ${COMMON_SOURCES} src/audio_transcode.cpp)

# 创建音视频封装测试可执行文件
add_executable(muxer_execute ${COMMON_SOURCES} src/muxer_execute.cpp)

# 链接库
if(APPLE)
    target_link_libraries(transcode
        ${FFMPEG_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )
    target_link_libraries(video_transcode
        ${FFMPEG_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )
    target_link_libraries(audio_transcode
        ${FFMPEG_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )
    target_link_libraries(muxer_execute
        ${FFMPEG_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )
elseif(UNIX AND NOT APPLE AND LINUX_DISTRO STREQUAL "Ubuntu")
    if(DEFINED USE_SYSTEM_FFMPEG AND USE_SYSTEM_FFMPEG)
        target_link_libraries(transcode
            ${FFMPEG_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${EXTRA_LIBS}
        )
        target_link_libraries(video_transcode
            ${FFMPEG_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${EXTRA_LIBS}
        )
        target_link_libraries(audio_transcode
            ${FFMPEG_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${EXTRA_LIBS}
        )
        target_link_libraries(muxer_execute
            ${FFMPEG_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${EXTRA_LIBS}
        )
        target_link_libraries(test_advanced_transcode
            ${FFMPEG_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${EXTRA_LIBS}
        )
    else()
        target_link_libraries(transcode
            ${CUSTOM_FFMPEG_LIB}
            ${CMAKE_THREAD_LIBS_INIT}
            ${EXTRA_LIBS}
        )
        target_link_libraries(video_transcode
            ${CUSTOM_FFMPEG_LIB}
            ${CMAKE_THREAD_LIBS_INIT}
            ${EXTRA_LIBS}
        )
        target_link_libraries(audio_transcode
            ${CUSTOM_FFMPEG_LIB}
            ${CMAKE_THREAD_LIBS_INIT}
            ${EXTRA_LIBS}
        )
        target_link_libraries(muxer_execute
            ${CUSTOM_FFMPEG_LIB}
            ${CMAKE_THREAD_LIBS_INIT}
            ${EXTRA_LIBS}
        )
    endif()
endif()

# 安装
install(TARGETS transcode video_transcode audio_transcode muxer_execute DESTINATION bin)
if(UNIX AND NOT APPLE AND LINUX_DISTRO STREQUAL "Ubuntu")
    if(EXISTS ${CUSTOM_FFMPEG_LIB_PATH})
        install(FILES ${CUSTOM_FFMPEG_LIB} DESTINATION lib)
    endif()
endif()

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) 
