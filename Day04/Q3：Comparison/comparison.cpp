#include <iostream>
#include <algorithm>

using namespace std;

struct Class
{
    int total;
    int boy;
    int girl;
};

template <typename T>
void compareClass(const Class &c1, const Class &c2, bool (*compare)(T, T))
{
    cout << (compare(c1.total, c2.total) ? "一班总人数多" : "二班总人数多") << endl;
    cout << (compare(c1.boy, c2.boy) ? "一班男生多" : "二班男生多") << endl;
    cout << (compare(c1.girl, c2.girl) ? "一班女生多" : "二班女生多") << endl;
}

bool compare(int a, int b)
{
    return a > b; // 自定义比较函数
}

bool isValid(int total, int boy, int girl)
{
    return total > 0 && boy > 0 && girl > 0 && total == boy + girl;
}

int main()
{
    Class c1 = {30, 15, 15};
    Class c2 = {25, 10, 15};

    if (!isValid(c1.total, c1.boy, c1.girl))
    {
        cerr << "一班数据无效: 总人数不等于男女生人数之和，且所有人数必须大于0。" << endl;
        return 1;
    }

    if (!isValid(c2.total, c2.boy, c2.girl))
    {
        cerr << "二班数据无效: 总人数不等于男女生人数之和，且所有人数必须大于0。" << endl;
        return 1;
    }

    compareClass<int>(c1, c2, compare);
    return 0;
}
