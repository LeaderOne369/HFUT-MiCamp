#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
using namespace std;
using namespace chrono;

// 计算数组部分和
void psum(const vector<int> &arr, long long &result, int start, int end)
{
    for (int i = start; i < end; ++i)
    {
        result += arr[i];
    }
}

int main()
{
    const int n = 10000;
    const int k = 5; // 线程数
    vector<int> arr(n);

    for (int i = 0; i < n; ++i)
    {
        arr[i] = i + 1;
    }

    vector<long long> sums(k, 0);
    vector<thread> t;
    int part = n / k;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < k; ++i)
    {
        int begin = i * part;
        int end = (i == k - 1) ? n : (i + 1) * part;
        t.push_back(thread(psum, cref(arr), ref(sums[i]), begin, end));
    }

    for (auto &t : t)
    {
        t.join();
    }

    long long result = 0;
    for (const auto &sum : sums)
    {
        result += sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<microseconds>(end - start);

    cout << "总和: " << result << endl;
    cout << "计算时间: " << time.count() << " 微秒" << endl;

    return 0;
}