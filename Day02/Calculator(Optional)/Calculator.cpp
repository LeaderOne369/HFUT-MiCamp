#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cctype>

using namespace std;

double add(double a, double b)
{
    return a + b;
}

double subtract(double a, double b)
{
    return a - b;
}

double multiply(double a, double b)
{
    return a * b;
}

double divide(double a, double b)
{
    if (b == 0)
    {
        throw runtime_error("错误: 除数不能为零。");
    }
    return a / b;
}

double calculate(double a, char op, double b)
{
    switch (op)
    {
    case '+':
        return add(a, b);
    case '-':
        return subtract(a, b);
    case '*':
        return multiply(a, b);
    case '/':
        return divide(a, b);
    default:
        throw runtime_error("错误: 无效的运算符。");
    }
}

int main()
{
    double operand1, operand2;
    char op;
    string input;

    while (true)
    {
        cout << "请输入表达式 (操作数1 运算符 操作数2) 或输入 'q' 退出: ";
        getline(cin, input);

        if (input == "q")
        {
            break;
        }

        stringstream ss(input);
        ss >> operand1 >> op >> operand2;

        if (ss.fail())
        {
            cout << "错误: 输入格式无效。" << endl;
            continue;
        }

        try
        {
            double result = calculate(operand1, op, operand2);
            cout << "结果: " << result << endl;
        }
        catch (const runtime_error &e)
        {
            cout << e.what() << endl;
        }
    }

    return 0;
}