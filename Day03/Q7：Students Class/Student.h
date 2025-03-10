#ifndef STUDENT_H
#define STUDENT_H

#include <string>

using namespace std;

class Student
{
private:
    string id;
    string name;
    int age;
    string sex;
    double score;
    string major;
    string grade;

public:
    Student() = default;
    Student(const string &id, const string &name, int age, const string &sex,
            double score, const string &major, const string &grade);

    string getId() const { return id; }
    string getName() const { return name; }
    int getAge() const { return age; }
    string getsex() const { return sex; }
    double getScore() const { return score; }
    string getMajor() const { return major; }
    string getGrade() const { return grade; }

    void setId(const string &id) { this->id = id; }
    void setName(const string &name) { this->name = name; }
    void setAge(int age) { this->age = age; }
    void setsex(const string &sex) { this->sex = sex; }
    void setScore(double score) { this->score = score; }
    void setMajor(const string &major) { this->major = major; }
    void setGrade(const string &grade) { this->grade = grade; }

    bool isPass() const { return score >= 60.0; }
    string level() const;
    void update(double newScore);

    void save(ostream &out) const;
    void load(istream &in);

    void display() const;
};

#endif