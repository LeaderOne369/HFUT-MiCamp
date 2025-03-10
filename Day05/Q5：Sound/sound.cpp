#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>

using namespace std;

queue<int> setSound;
mutex mtx;
condition_variable cv;
bool done = false;

void set()
{
    for (int i = 1; i <= 30; ++i)
    {
        this_thread::sleep_for(chrono::milliseconds(100)); // 模拟拖动
        {
            lock_guard<mutex> lock(mtx);
            setSound.push(i);
        }
        cv.notify_one();
    }
}

void sound()
{
    while (true)
    {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, []
                { return !setSound.empty() || done; });
        if (done && setSound.empty())
            break;
        if (!setSound.empty())
        {
            int volume = setSound.front();
            setSound.pop();
            lock.unlock();

            cout << "播放音量: " << volume << endl;
            this_thread::sleep_for(chrono::seconds(1));

            // 清空队列，只保留最后一个音量
            lock.lock();
            while (setSound.size() > 1)
            {
                setSound.pop();
            }
        }
        lock.unlock();
    }
}

void test()
{
    vector<int> test = {1, 15, 20, 30};
    for (int n : test)
    {
        this_thread::sleep_for(chrono::milliseconds(100)); // 模拟快速拖动
        {
            lock_guard<mutex> lock(mtx);
            setSound.push(n);
        }
        cv.notify_one();
    }
    {
        lock_guard<mutex> lock(mtx);
        done = true;
    }
    cv.notify_all();
}

int main()
{
    thread t1(test);
    thread t2(sound);

    t1.join();
    t2.join();

    return 0;
}