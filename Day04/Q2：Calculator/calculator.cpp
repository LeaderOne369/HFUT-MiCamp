#include <iostream>
using namespace std;

template <typename T>
class Calculator
{
private:
    T a, b;

public:
    Calculator(T x, T y) : a(x), b(y) {}

    T add() const { return a + b; }
    T sub() const { return a - b; }
    T mul() const { return a * b; }

    // 处理商可能是浮点数的问题
    double div() const
    {
        if (b == 0)
        {
            throw runtime_error("除数不能为零");
        }
        else
        {
            return static_cast<double>(a) / static_cast<double>(b);
        }
    }
};

int main()
{
    int a, b;
    double c, d;

    // 测试
    cout << "请输入两个整数：" << endl;
    cin >> a >> b;
    Calculator<int> cal1(a, b);
    cout << endl;
    cout << a << " + " << b << " = " << cal1.add() << endl;
    cout << a << " - " << b << " = " << cal1.sub() << endl;
    cout << a << " * " << b << " = " << cal1.mul() << endl;
    try
    {
        cout << a << " / " << b << " = " << cal1.div() << endl; // 会输出浮动结果
    }
    catch (const runtime_error &e)
    {
        cout << e.what() << endl;
    }

    cout << "\n请输入两个浮点数：" << endl;
    cin >> c >> d;
    Calculator<double> cal2(c, d);
    cout << endl;
    cout << c << " + " << d << " = " << cal2.add() << endl;
    cout << c << " - " << d << " = " << cal2.sub() << endl;
    cout << c << " * " << d << " = " << cal2.mul() << endl;
    try
    {
        cout << c << " / " << d << " = " << cal2.div() << endl;
    }
    catch (const runtime_error &e)
    {
        cout << e.what() << endl;
    }

    return 0;
}