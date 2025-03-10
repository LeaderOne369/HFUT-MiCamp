#include <iostream>
#include <string>
#include <unordered_set>

using namespace std;

// 根据老师的提示，unordered_set底层实现是哈希表，效率高
int count(const string &s)
{
    unordered_set<char> unique;
    for (char c : s)
    {
        unique.insert(c);
    }
    return unique.size();
}

int main()
{
    string s;
    cout << "请输入字符串: ";
    getline(cin, s);

    int a = count(s);
    cout << "不同字符个数: " << a << endl;

    return 0;
}