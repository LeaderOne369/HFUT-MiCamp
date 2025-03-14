### Q5：文件基本操作练习

### 题目描述：

- 请手动创建一个文件 text，并使用标准 C 文件操作函数打开文件并写入自己的名字到文件，然后再打开文件，读取文件内容打印到控制台。

### 程序设计：

- 1.使用终端手动创建 myName.txt：

```c++
touch myName.txt
```

- 2.编写程序并执行：

```c++
    // 打开文件，写入我的名字到文件并关闭文件
    file = fopen(filename, "w");
    if (file == NULL) {
        printf("无法打开文件\n");
        return 1;
    }
    fprintf(file, "我的名字\n");
    fclose(file);
```

- 3.读取文件内容：

```c++
    // 重新打开文件，读取文件内容并打印到控制台
    file = fopen(filename, "r");
    if (file == NULL) {
        printf("无法打开文件\n");
        return 1;
    }
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }
```

### 输出结果涉及个人隐私，已经删除。
