#include <iostream>
using namespace std;

int findMax(int *arr, int size)
{
    int max = arr[0];
    for (int i = 1; i < size; i++)
    {
        if (arr[i] > max)
        {
            max = arr[i];
        }
    }
    return max;
}

int main()
{
    int size;
    cout << "请输入数组大小：";
    cin >> size;

    // 动态分配内存
    cout << "使用 [new] 分配内存" << endl;
    int *numbers = new int[size];

    cout << "请输入" << size << "个整数：" << endl;
    for (int i = 0; i < size; i++)
    {
        cin >> numbers[i];
    }

    int maxNum = findMax(numbers, size);
    cout << "最大值是：" << maxNum << endl;

    // 释放内存
    cout << "使用 [delete] 释放内存" << endl;
    delete[] numbers;

    return 0;
}