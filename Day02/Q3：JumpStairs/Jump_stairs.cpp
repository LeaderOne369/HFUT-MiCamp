#include <iostream>
#include <vector>

using namespace std;

// 方法1: 简单递归
int jumpCount(int steps)
{
    if (steps <= 1)
        return 1;
    return jumpCount(steps - 1) + jumpCount(steps - 2);
}

// 方法2: 记忆化递归
int memoryJump(int steps, vector<int> &memory)
{
    if (steps <= 1)
        return 1;
    if (memory[steps] != -1)
        return memory[steps];
    memory[steps] = memoryJump(steps - 1, memory) + memoryJump(steps - 2, memory);
    return memory[steps];
}

// 方法3: 空间优化法
int quickJump(int steps)
{
    if (steps <= 1)
        return 1;
    int prev = 1, curr = 1;
    for (int i = 2; i <= steps; ++i)
    {
        int temp = prev + curr;
        prev = curr;
        curr = temp;
    }
    return curr;
}

int main()
{
    int steps;
    cout << "请问楼梯有几级？";
    cin >> steps;

    cout << "用递归计算的结果: " << jumpCount(steps) << endl;

    vector<int> memory(steps + 1, -1);
    cout << "用记忆化递归计算的结果: " << memoryJump(steps, memory) << endl;

    cout << "用空间优化法计算的结果: " << quickJump(steps) << endl;

    return 0;
}