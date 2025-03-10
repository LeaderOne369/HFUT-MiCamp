#include <iostream>
#include <cmath>
#include <limits>
using namespace std;

bool isValidInput(double &value)
{
    cin >> value;
    if (cin.fail())
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    return true;
}

void calculate(double a, double b, double c)
{
    if (a == 0)
    {
        cout << "这不是一元二次方程（a = 0）" << endl;
        return;
    }

    double delta = b * b - 4 * a * c;

    if (delta > 0)
    {
        double x1 = (-b + sqrt(delta)) / (2 * a);
        double x2 = (-b - sqrt(delta)) / (2 * a);
        cout << "方程有两个不相等的实根：" << endl;
        cout << "x1 = " << x1 << endl;
        cout << "x2 = " << x2 << endl;
    }
    else if (delta == 0)
    {
        double x = -b / (2 * a);
        cout << "方程有两个相等的实根：" << endl;
        cout << "x1 = x2 = " << x << endl;
    }
    else
    {
        double realPart = -b / (2 * a);
        double imaginaryPart = sqrt(-delta) / (2 * a);
        cout << "方程没有实根：" << endl;
    }
}

int main()
{
    double a, b, c;

    cout << "请输入二次方程 ax² + bx + c = 0 的系数：" << endl;

    cout << "a = ";
    while (!isValidInput(a))
    {
        cout << "无效输入，请输入一个有效的数字！a = ";
    }

    cout << "b = ";
    while (!isValidInput(b))
    {
        cout << "无效输入，请输入一个有效的数字！b = ";
    }

    cout << "c = ";
    while (!isValidInput(c))
    {
        cout << "无效输入，请输入一个有效的数字！c = ";
    }

    calculate(a, b, c);

    return 0;
}