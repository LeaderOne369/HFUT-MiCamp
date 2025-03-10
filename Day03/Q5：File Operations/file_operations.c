#include <stdio.h>

int main() {
    FILE *file;
    const char *filename = "myName.txt";

    // 打开文件，写入我的名字到文件并关闭文件
    file = fopen(filename, "w");
    if (file == NULL) {
        printf("无法打开文件\n");
        return 1;
    }
    fprintf(file, "徐力行\n");
    fclose(file);

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

    fclose(file);

    return 0;
}