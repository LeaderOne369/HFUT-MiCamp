#include <iostream>
#include <unordered_set>
#include <vector>
#include <algorithm>

using namespace std;

void Big_k(const vector<int> &nums, int k)
{
    if (k > nums.size())
    {
        cout << "错误: k值大于数字个数。" << endl;
        return;
    }
    unordered_multiset<int> num_set(nums.begin(), nums.end());
    vector<int> sorted(num_set.begin(), num_set.end());
    sort(sorted.rbegin(), sorted.rend());

    cout << "最大的" << k << "个数字是: ";
    for (int i = 0; i < k && i < sorted.size(); ++i)
    {
        cout << sorted[i] << " ";
    }
    cout << endl;
}

int main()
{
    vector<int> nums;
    int num, k;

    cout << "输入数字（按空格分隔，按回车结束）: ";
    while (cin >> num)
    {
        nums.push_back(num);
        if (cin.peek() == '\n')
            break;
    }

    cout << "输入k值: ";
    cin >> k;

    Big_k(nums, k);

    return 0;
}