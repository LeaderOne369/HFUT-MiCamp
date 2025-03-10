#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

using namespace std;

class Pool
{
public:
    Pool(size_t threads);
    ~Pool();
    template <typename F>
    void add(F &&f);
    void stop();
    int getCount() const { return Count.load(); }

private:
    void work();
    vector<thread> t;
    queue<function<void()>> tasks;
    mutex mtx;
    condition_variable cv;
    atomic<bool> flag;
    mutex printMutex;
    atomic<int> Count;
};

Pool::Pool(size_t threads) : flag(false), Count(0)
{
    for (size_t i = 0; i < threads; ++i)
    {
        t.push_back(thread(&Pool::work, this));
    }
}
Pool::~Pool()
{
    stop();
    for (auto &t : t)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

template <typename F>
void Pool::add(F &&f)
{
    {
        lock_guard<mutex> lock(mtx);
        tasks.push(std::forward<F>(f));
    }
    cv.notify_one();
}
void Pool::stop()
{
    flag = true;
    cv.notify_all();
}

void Pool::work()
{
    while (true)
    {
        function<void()> task;
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [this]
                    { return !tasks.empty() || flag; });
            if (flag && tasks.empty())
            {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        {
            lock_guard<mutex> printLock(printMutex);
            Count++;
            task();
            Count--;
        }
    }
}

void task()
{
    cout << "任务执行" << endl;
    this_thread::sleep_for(chrono::seconds(1));
    cout << "任务完毕" << endl;
}

int main()
{
    cout << "程序开始" << endl;
    Pool pool(4);
    pool.add(task);
    pool.add([]()
             {
        cout << "神秘任务执行" << endl;
        this_thread::sleep_for(chrono::seconds(1));
        cout << "神秘任务完毕" << endl; });
    while (pool.getCount() > 0)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    pool.stop();
    cout << "程序结束" << endl;
    return 0;
}