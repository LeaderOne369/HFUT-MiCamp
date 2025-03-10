#include "Student.h"
#include <iostream>
#include <iomanip>

Student::Student(const string &id, const string &name, int age, const string &sex,
                 double score, const string &major, const string &grade)
    : id(id), name(name), age(age), sex(sex),
      score(score), major(major), grade(grade) {}

string Student::level() const
{
    if (score >= 90)
        return "优秀";
    if (score >= 80)
        return "良好";
    if (score >= 70)
        return "中等";
    if (score >= 60)
        return "及格";
    return "不及格";
}

void Student::update(double newScore)
{
    if (newScore >= 0 && newScore <= 100)
    {
        score = newScore;
    }
    else
    {
        cout << "分数必须在0-100之间" << endl;
    }
}

void Student::save(ostream &out) const
{
    out << id << " " << name << " " << age << " " << sex << " "
        << score << " " << major << " " << grade << endl;
}

void Student::load(istream &in)
{
    in >> id >> name >> age >> sex >> score >> major >> grade;
}

void Student::display() const
{
    cout << "学号：" << id << endl
         << "姓名：" << name << endl
         << "年龄：" << age << endl
         << "性别：" << sex << endl
         << "分数：" << fixed << setprecision(1) << score << endl
         << "专业：" << major << endl
         << "年级：" << grade << endl
         << "成绩等级：" << level() << endl;
}