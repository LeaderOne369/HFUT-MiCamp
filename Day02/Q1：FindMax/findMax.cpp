#include <iostream>
#include <limits>
#include <sstream>
using namespace std;

bool isValidInput(double &value)
{
    string input;
    cin >> input;
    stringstream ss(input);
    ss >> value;
    return !ss.fail() && ss.eof();
}

double findMax(double a, double b, double c)
{
    double max = a;

    if (b > max)
    {
        max = b;
    }

    if (c > max)
    {
        max = c;
    }

    return max;
}

int main()
{
    double num1, num2, num3;

    cout << "请输入第一个数字: ";
    while (!isValidInput(num1))
    {
        cout << "无效输入，请输入一个有效的数字: ";
    }

    cout << "请输入第二个数字: ";
    while (!isValidInput(num2))
    {
        cout << "无效输入，请输入一个有效的数字: ";
    }

    cout << "请输入第三个数字: ";
    while (!isValidInput(num3))
    {
        cout << "无效输入，请输入一个有效的数字: ";
    }

    double result = findMax(num1, num2, num3);
    cout << "最大值是: " << result << endl;

    return 0;
}