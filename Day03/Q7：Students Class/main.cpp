#include "Student.h"
#include <iostream>
#include <fstream>
#include <vector>

int main()
{
    vector<Student> students = {
        Student("1", "甲", 21, "男", 90.5, "软件工程", "大三"),
        Student("2", "乙", 20, "男", 86.0, "软件工程", "大三"),
        Student("3", "丙", 21, "男", 85.5, "软件工程", "大三")};

    // 写入文件
    ofstream outFile("students.txt");
    if (!outFile)
    {
        cout << "无法打开文件" << endl;
        return 1;
    }

    for (const auto &student : students)
    {
        student.save(outFile);
    }
    outFile.close();

    // 读取文件
    ifstream inFile("students.txt");
    if (!inFile)
    {
        cout << "无法打开文件" << endl;
        return 1;
    }

    vector<Student> stds;
    Student temp;
    while (inFile.good())
    {
        temp.load(inFile);
        if (inFile.good())
        {
            stds.push_back(temp);
        }
    }
    inFile.close();

    // 显示读取的学生信息
    for (const auto &student : stds)
    {
        student.display();
        cout << "-------------------" << endl;
    }

    // 测试功能
    stds[0].update(60.0);
    cout << stds[0].getName() << "的新成绩等级: "
         << stds[0].level() << endl;

    return 0;
}