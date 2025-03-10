# 定义编译器
CXX = g++
# 定义编译选项
CXXFLAGS = -std=c++11 -Wall
# 定义目标文件
TARGET = reverse
# 定义源代码文件
SRCS = reverse.cpp
# 定义编译生成的目标文件
OBJS = $(SRCS:.cpp=.o)

# 默认规则，编译目标文件
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

# 编译规则：.cpp -> .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理目标文件
clean:
	rm -f $(OBJS) $(TARGET)