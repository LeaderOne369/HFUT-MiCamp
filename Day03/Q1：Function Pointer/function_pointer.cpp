#include <stdio.h>

int add(int a, int b)
{
    printf("两数之和为：%d\n", a + b);
    return a + b;
}

int main()
{
    int (*pf)(int, int) = add;

    int a, b, c, d;

    // 两种调用方式
    printf("函数指针调用 - 请输入两个数字：");
    scanf("%d %d", &a, &b);
    printf("函数指针调用：pf(%d, %d) = %d\n", a, b, pf(a, b));

    printf("\n解引用函数指针调用 - 请输入两个数字：");
    scanf("%d %d", &c, &d);
    printf("解引用函数指针调用：(*pf)(%d, %d) = %d\n", c, d, (*pf)(c, d));

    // 打印各种地址并比较
    printf("add  的地址：%p\n", add);
    printf("*add 的地址：%p\n", *add);
    printf("&add 的地址：%p\n", &add);
    printf("pf   的地址：%p\n", pf);
    printf("*pf  的地址：%p\n", *pf);
    printf("&pf  的地址：%p\n", &pf);

    return 0;
}