#include <iostream>
#include "Dictionary.h"
using namespace std;

int main()
{
    Dictionary dict;
    dict.add("HFUT", "合工大");
    dict.add("HUAWEI", "华为");
    dict.add("Apple", "苹果");
    dict.add("XiaoMi", "小米");

    string word;
    while (true)
    {
        cout << "请输入词汇 (输入q退出)：";
        cin >> word;
        if (word == "q")
        {
            break;
        }
        cout << dict.translate(word) << endl;
    }
    return 0;
}