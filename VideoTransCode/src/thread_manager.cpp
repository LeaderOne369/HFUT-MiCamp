#include "../include/thread_manager.h"
#include <iostream>
#include <functional>
#include <string>
#include <thread>
#include <exception>

using namespace std;

ThreadManager::ThreadManager(const string &name)
    : running_(false), name_(name)
{
}

ThreadManager::~ThreadManager()
{
    stop();
}

void ThreadManager::setThreadFunction(function<void()> func)
{
    threadFunc_ = func;
}

bool ThreadManager::start()
{
    if (running_ || !threadFunc_)
    {
        return false;
    }

    running_ = true;
    thread_ = thread([this]()
                     {
        try {
            threadFunc_();
        } catch (const exception& e) {
            cerr << "线程 " << name_ << " 异常: " << e.what() << endl;
        } catch (...) {
            cerr << "线程 " << name_ << " 未知异常" << endl;
        }
        
        running_ = false; });

    return true;
}

void ThreadManager::stop()
{
    running_ = false;

    if (thread_.joinable())
    {
        thread_.join();
    }
}

bool ThreadManager::isRunning() const
{
    return running_;
}

string ThreadManager::getName() const
{
    return name_;
}