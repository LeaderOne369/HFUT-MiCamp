#include "Rectangle.h"
#include <iostream>

using namespace std;

int main() {
    cout << "矩形坐标计算程序\n";
    cout << "================\n\n";

    Rectangle rect;
    
    cout << "请按提示输入矩形的两个对角点坐标。\n";
    cout << "注意：第一个点应该是左上角点，第二个点应该是右下角点。\n\n";

    // 尝试输入矩形的坐标
    while (!rect.inputRectangle()) {
        cout << "\n是否要重新输入？(y/n): ";
        char choice;
        cin >> choice;
        if (choice != 'y' && choice != 'Y') {
            cout << "程序结束\n";
            return 0;
        }
        // 清除输入缓冲区
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    // 显示矩形信息
    rect.displayInfo();

    return 0;
} 