#include <iostream>
using namespace std;

template <typename T>
// max 是标准库中的函数，因此不能直接使用。于是我命名为 Max，避免命名冲突。
T Max(T a, T b, T c)
{
    T max = a;
    if (b > max)
        max = b;
    if (c > max)
        max = c;
    return max;
}

int main()
{
    // 为了体现出模板函数的优势，我测试了整数、浮点数和字符类型比较大小的数据
    cout << "最大整数: " << Max(42, 18, 91) << endl;
    cout << "最大浮点数: " << Max(3.14, 5.56, 7.62) << endl;
    cout << "最大字符: " << Max('X', 'A', 'M') << endl;

    return 0;
}