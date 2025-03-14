#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "Point.h"

class Rectangle
{
private:
    Point topLeft;     // 左上角点
    Point bottomRight; // 右下角点

    // 验证两个点是否能构成有效的矩形
    bool isValid() const;

public:
    // 构造函数
    Rectangle(const Point &tl = Point(), const Point &br = Point());

    // 设置矩形的顶点
    bool setPoints(const Point &tl, const Point &br);

    // 从用户输入读取矩形的顶点
    bool inputRectangle();

    // 计算并显示矩形的信息
    void displayInfo() const;

    // 计算矩形面积
    double getArea() const;
};

#endif // RECTANGLE_H