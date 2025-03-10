#include <iostream>
using namespace std;

int main()
{
    // 第一问：定义大小为10的一维数组arr，打印arr、&arr和&arr[0]的值并比较
    int arr[10];
    printf("arr = %p\n", arr);
    printf("&arr = %p\n", &arr);
    printf("&arr[0] = %p\n\n", &arr[0]);

    // 第二问：定义数组buf和指针p，比较sizeof(buf)和sizeof(p)
    int buf[10];
    int *p = buf;
    cout << "sizeof(buf) = " << sizeof(buf) << endl;
    cout << "sizeof(p) = " << sizeof(p) << endl
         << endl;

    // 第三问：定义二维数组data并初始化，用指针p指向data。由于p已被第二问使用，此处我使用q代替
    int data[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}};
    int(*q)[4] = data;

    cout << "data[1][2] = " << data[1][2] << endl;
    cout << "q[1][2] = " << q[1][2] << endl;

    return 0;
}