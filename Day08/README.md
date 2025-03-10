# Day08：交叉编译

- 今日的三道练习题同样在各自的文件夹中附有 README 文档，请分别查阅，此处仅列出题目要求。

## HTTP 下载 MP4 文件

- 1：libghttp 为子工程，实现了 http 功能，需要编译生成动态库。
- 2：主工程为 http 请求测试工程，编译生成可执行文件，依赖子工程的 http 库，完成 http 的 get 和 download 请求测试。

## JNI HTTP 交叉编译

- 创建 http jni 接口可以调用 http 请求
- 调用 get 请求，请求网页内容
- 把请求的内容 jni 返回给 android 的 receive 接口，显示出来

## AiAssist

- 完成 cJSON 交叉编译，并把生成的解析库放入 android 工程当中，对网络请求的回来的数据解析，可以完成大模型对话功能。
- 要求：
  - 1．把 cJSON 编译成 android 可用的 so 库。
  - 2．把 cJSON 的使用接口封装成 jni 可以供 java 层调用。
  - 3、实现和 AI 对话功能。
