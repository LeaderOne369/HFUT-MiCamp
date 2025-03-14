#include "Rectangle.h"
#include <iostream>
#include <cmath>

using namespace std;

Rectangle::Rectangle(const Point &tl, const Point &br) : topLeft(tl), bottomRight(br) {}

bool Rectangle::isValid() const
{
    // 检查两点是否在对角线上（即x和y坐标都不相同）
    if (topLeft.getX() == bottomRight.getX() || topLeft.getY() == bottomRight.getY())
    {
        return false;
    }

    // 确保topLeft真的是左上角点（x坐标更小，y坐标更大）
    if (topLeft.getX() > bottomRight.getX() || topLeft.getY() < bottomRight.getY())
    {
        return false;
    }

    return true;
}

bool Rectangle::setPoints(const Point &tl, const Point &br)
{
    topLeft = tl;
    bottomRight = br;
    return isValid();
}

bool Rectangle::inputRectangle()
{
    cout << "\n请输入矩形的左上角坐标：\n";
    if (!topLeft.inputPoint())
    {
        return false;
    }

    cout << "\n请输入矩形的右下角坐标：\n";
    if (!bottomRight.inputPoint())
    {
        return false;
    }

    if (!isValid())
    {
        cout << "错误：输入的点无法构成有效的矩形！\n";
        cout << "请确保：\n";
        cout << "1. 第一个点是左上角点（x坐标更小，y坐标更大）\n";
        cout << "2. 第二个点是右下角点（x坐标更大，y坐标更小）\n";
        cout << "3. 两点不在同一条垂直或水平线上\n";
        return false;
    }

    return true;
}

void Rectangle::displayInfo() const
{
    cout << "\n矩形信息：\n";
    cout << "左上角点：";
    topLeft.display();
    cout << "\n右下角点：";
    bottomRight.display();
    cout << "\n面积：" << getArea() << endl;
}

double Rectangle::getArea() const
{
    double width = bottomRight.getX() - topLeft.getX();
    double height = topLeft.getY() - bottomRight.getY();
    return width * height;
}