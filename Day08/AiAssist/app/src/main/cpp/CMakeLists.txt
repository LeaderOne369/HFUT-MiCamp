#说明 android cmake生成的target需要主动链接两个库： android和log，
#其中android库用来调用相关android的功能，log用来打印日志
#例子：target_link_libraries(aiassist
#        android
#        log

#请补全以下cmakelists.txt的内容
#作业要求：
#1. 要求可以生成动态库，库的名字叫做aiassist。
#2. 工程具有子工程cJSON，生成库叫cjson。
#3. aiassist库需要依赖cjson。
#说明：
#cJSON里面的cmake是编写好的不用做修改，只需要不全以下cmakelists.txt文件即可。

cmake_minimum_required(VERSION 3.22.1)

project("assist")

# 设置输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_ABI})

# 在 project() 之后添加
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

# 添加 cJSON 库
add_library(cjson SHARED
    cJSON/cJSON.c
    cJSON/cJSON_Utils.c)

# 添加主要的 JNI 库
add_library(aiassist SHARED
    chat_jni.cpp)

# 链接 cJSON 库到 aiassist
target_link_libraries(aiassist
    cjson
    android
    log)

# 包含头文件目录
target_include_directories(aiassist PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/cJSON)

# 设置安装路径
install(TARGETS cjson aiassist
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${ANDROID_ABI})