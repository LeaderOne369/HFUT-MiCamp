#!/bin/bash

# 创建build目录
mkdir -p build

# 进入build目录
cd build

# 运行CMake
cmake ..

# 构建项目
cmake --build .

echo "构建完成！"
echo "在Mac上运行服务端: ./build/server.app/Contents/MacOS/server"
echo "在Mac上运行客户端: ./build/client.app/Contents/MacOS/client"
echo "或者使用以下命令: open ./build/server.app 或 open ./build/client.app"