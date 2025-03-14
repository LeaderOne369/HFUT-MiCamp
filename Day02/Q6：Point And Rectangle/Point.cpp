#include "Point.h"
#include <iostream>
#include <limits>

using namespace std;

Point::Point(double x, double y) : x(x), y(y) {}

double Point::getX() const
{
    return x;
}

double Point::getY() const
{
    return y;
}

void Point::setX(double x)
{
    this->x = x;
}

void Point::setY(double y)
{
    this->y = y;
}

bool Point::inputPoint()
{
    cout << "请输入x坐标: ";
    while (!(cin >> x))
    {
        if (cin.eof())
        {
            return false; // 处理EOF
        }
        cout << "输入无效，请输入一个数字: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "请输入y坐标: ";
    while (!(cin >> y))
    {
        if (cin.eof())
        {
            return false; // 处理EOF
        }
        cout << "输入无效，请输入一个数字: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    return true;
}

void Point::display() const
{
    cout << "(" << x << ", " << y << ")";
}