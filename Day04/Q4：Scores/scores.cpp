#include <iostream>
#include <vector>
#include <iomanip>
using namespace std;

int main()
{
    vector<int> scores(5);
    cout << "请输入5个学生的成绩（0~100）:" << endl;
    for (int i = 0; i < 5; ++i)
    {
        while (true)
        {
            cin >> scores[i];
            if (cin.fail() || scores[i] < 0 || scores[i] > 100)
            {
                cin.clear();
                cin.ignore(INT_MAX, '\n');
                cout << "输入不合法，请重新输入：" << endl;
            }
            else
            {
                break;
            }
        }
    }
    double sum = 0;
    int max = scores[0], min = scores[0];
    for (int a : scores)
    {
        sum += a;
        if (a > max)
            max = a;
        if (a < min)
            min = a;
    }
    double average = sum / scores.size();
    cout << fixed << setprecision(2);
    cout << "平均分: " << average << endl;
    cout << "最高分: " << max << endl;
    cout << "最低分: " << min << endl;
    return 0;
}