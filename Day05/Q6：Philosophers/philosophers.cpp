#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <condition_variable>

using namespace std;

mutex chopsticks[5];
condition_variable cv;
int num = 0;
mutex mtx;

void philosopher(int id)
{
    {
        lock_guard<mutex> lock(mtx);
        cout << "哲学家 " << id << " 正在思考" << endl;
    }
    this_thread::sleep_for(chrono::milliseconds(100));

    {
        unique_lock<mutex> lk(mtx);
        cv.wait(lk, [id]
                { return num == id; });
    }

    // 确保哲学家按顺序进餐
    this_thread::sleep_for(chrono::milliseconds(100 * id));

    int left = id;
    int right = (id + 1) % 5;

    chopsticks[left].lock();
    {
        lock_guard<mutex> lock(mtx);
        cout << "哲学家 " << id << " 拿起了左边的筷子" << endl;
    }
    chopsticks[right].lock();
    {
        lock_guard<mutex> lock(mtx);
        cout << "哲学家 " << id << " 拿起了右边的筷子" << endl;
    }
    {
        lock_guard<mutex> lock(mtx);
        cout << "哲学家 " << id << " 正在进餐" << endl;
    }
    this_thread::sleep_for(chrono::milliseconds(100));
    chopsticks[right].unlock();
    {
        lock_guard<mutex> lock(mtx);
        cout << "哲学家 " << id << " 放下了右边的筷子" << endl;
    }
    chopsticks[left].unlock();
    {
        lock_guard<mutex> lock(mtx);
        cout << "哲学家 " << id << " 放下了左边的筷子" << endl;
    }

    // 通知下一个哲学家
    {
        lock_guard<mutex> lock(mtx);
        num++;
        cv.notify_all();
    }
}

int main()
{
    vector<thread> t;
    for (int i = 0; i <= 4; ++i)
    {
        t.push_back(thread(philosopher, i));
    }
    for (auto &p : t)
    {
        p.join();
    }
    return 0;
}