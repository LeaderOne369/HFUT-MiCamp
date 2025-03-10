#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <thread>
#include <atomic>
#include <functional>
#include <string>

using namespace std;

class ThreadManager
{
private:
    thread thread_;
    atomic<bool> running_;
    string name_;

    function<void()> threadFunc_;

public:
    ThreadManager(const string &name = "");
    virtual ~ThreadManager();

    void setThreadFunction(function<void()> func);

    bool start();

    void stop();

    bool isRunning() const;

    string getName() const;
};

#endif // THREAD_MANAGER_H