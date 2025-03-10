#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

// void task(int id)
// {
//     cout << "线程 " << id << " 执行" << endl;
// }

mutex mtx;

// void task(int id)
// {
//     mtx.lock();
//     cout << "线程 " << id << " 执行" << endl;
//     mtx.unlock();
// }

condition_variable cv;
int counter = 0;
void task(int id)
{
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [id]
            { return id == counter; });
    cout << "线程 " << id << " 执行" << endl;
    ++counter;
    cv.notify_all();
}

int main()
{
    const int k = 5;
    thread t[k];

    for (int i = 0; i < k; ++i)
    {
        t[i] = thread(task, i);
    }

    for (int i = 0; i < k; ++i)
    {
        t[i].join();
    }

    return 0;
}