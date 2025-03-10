#ifndef QUEUE_H
#define QUEUE_H

#include <memory>
#include <mutex>
#include <condition_variable>

template <typename T>
struct QueueNode {
    T data;
    QueueNode* next;
    
    QueueNode(const T& value) : data(value), next(nullptr) {}
};

template <typename T>
class MyQueue {
private:
    QueueNode<T>* front_;
    QueueNode<T>* rear_;
    int size_;
    
public:
    MyQueue() : front_(nullptr), rear_(nullptr), size_(0) {}
    
    ~MyQueue() {
        clear();
    }
    
    void push(const T& item) {
        QueueNode<T>* newNode = new QueueNode<T>(item);
        
        if (isEmpty()) {
            front_ = newNode;
            rear_ = newNode;
        } else {
            rear_->next = newNode;
            rear_ = newNode;
        }
        
        size_++;
    }
    
    bool pop(T& item) {
        if (isEmpty()) {
            return false;
        }
        
        QueueNode<T>* temp = front_;
        item = temp->data;
        
        front_ = front_->next;
        delete temp;
        
        if (front_ == nullptr) {
            rear_ = nullptr;
        }
        
        size_--;
        return true;
    }
    
    bool front(T& item) const {
        if (isEmpty()) {
            return false;
        }
        
        item = front_->data;
        return true;
    }
    
    void clear() {
        while (!isEmpty()) {
            T dummy;
            pop(dummy);
        }
    }
    
    bool isEmpty() const {
        return front_ == nullptr;
    }
    
    int size() const {
        return size_;
    }
};

template <typename T>
class ThreadSafeQueue {
private:
    MyQueue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
    int maxSize_;
    bool quit_;

public:
    ThreadSafeQueue(int maxSize = 100) : maxSize_(maxSize), quit_(false) {}
    
    ~ThreadSafeQueue() {
        clear();
    }
    
    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.size() >= maxSize_ && !quit_) {
            cond_.wait(lock);
        }
        
        if (quit_) return;
        
        queue_.push(item);
        cond_.notify_one();
    }
    
    bool pop(T& item, bool block = true) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        while (queue_.isEmpty() && !quit_) {
            if (!block) return false;
            cond_.wait(lock);
        }
        
        if (queue_.isEmpty()) return false;
        
        queue_.pop(item);
        cond_.notify_one();
        return true;
    }
    
    void clear() {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.clear();
        cond_.notify_all();
    }
    
    void setQuit() {
        std::unique_lock<std::mutex> lock(mutex_);
        quit_ = true;
        cond_.notify_all();
    }
    
    int size() {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

struct Packet {
    uint8_t* data;
    int size;
    int64_t pts;
    int64_t dts;
    bool isKeyFrame;
    
    Packet() : data(nullptr), size(0), pts(0), dts(0), isKeyFrame(false) {}
    ~Packet() {
        if (data) {
            delete[] data;
            data = nullptr;
        }
    }
};

using PacketPtr = std::shared_ptr<Packet>;

using VideoPacketQueue = ThreadSafeQueue<PacketPtr>;

using AudioPacketQueue = ThreadSafeQueue<PacketPtr>;

#endif // QUEUE_H 