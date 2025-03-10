#include <iostream>
using namespace std;

template <typename T>
// swap 是标准库中的函数，因此不能直接使用。于是我命名为 Swap，避免命名冲突。
void Swap(T &a, T &b)
{
    T temp = a;
    a = b;
    b = temp;
}

int main()
{
    // 为了体现出模板函数的优势，我测试了整数、浮点数和字符类型交换的数据
    int x = 10, y = 20;
    cout << "整数:" << endl;
    cout << "交换前: x = " << x << ", y = " << y << endl;
    Swap(x, y);
    cout << "交换后: x = " << x << ", y = " << y << endl;

    cout << "\n浮点数:" << endl;
    double a = 3.14, b = 7.62;
    cout << "交换前: a = " << a << ", b = " << b << endl;
    Swap(a, b);
    cout << "交换后: a = " << a << ", b = " << b << endl;

    cout << "\n字符:" << endl;
    char c = 'A', d = 'B';
    cout << "交换前: c = " << c << ", d = " << d << endl;
    Swap(c, d);
    cout << "交换后: c = " << c << ", d = " << d << endl;

    return 0;
}