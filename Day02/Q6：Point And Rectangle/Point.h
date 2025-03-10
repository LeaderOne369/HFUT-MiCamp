#ifndef POINT_H
#define POINT_H

class Point {
private:
    double x;
    double y;

public:
    // 构造函数
    Point(double x = 0, double y = 0);

    // 获取坐标值
    double getX() const;
    double getY() const;

    // 设置坐标值
    void setX(double x);
    void setY(double y);

    // 从用户输入读取坐标
    bool inputPoint();

    // 打印点的坐标
    void display() const;
};

#endif // POINT_H 