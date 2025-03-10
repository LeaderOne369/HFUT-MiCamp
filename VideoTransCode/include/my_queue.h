#ifndef MY_QUEUE_H
#define MY_QUEUE_H

template <typename T>
struct QueueNode
{
    T data;
    QueueNode *next;

    QueueNode(const T &value) : data(value), next(nullptr) {}
};

template <typename T>
class MyQueue
{
private:
    QueueNode<T> *front_;
    QueueNode<T> *rear_;
    int size_;

public:
    MyQueue() : front_(nullptr), rear_(nullptr), size_(0) {}

    ~MyQueue()
    {
        clear();
    }

    void push(const T &item)
    {
        QueueNode<T> *newNode = new QueueNode<T>(item);

        if (isEmpty())
        {
            front_ = newNode;
            rear_ = newNode;
        }
        else
        {
            rear_->next = newNode;
            rear_ = newNode;
        }

        size_++;
    }

    bool pop(T &item)
    {
        if (isEmpty())
        {
            return false;
        }

        QueueNode<T> *temp = front_;
        item = temp->data;

        front_ = front_->next;
        delete temp;

        if (front_ == nullptr)
        {
            rear_ = nullptr;
        }

        size_--;
        return true;
    }

    bool front(T &item) const
    {
        if (isEmpty())
        {
            return false;
        }

        item = front_->data;
        return true;
    }

    void clear()
    {
        while (!isEmpty())
        {
            T dummy;
            pop(dummy);
        }
    }

    bool isEmpty() const
    {
        return front_ == nullptr;
    }

    int size() const
    {
        return size_;
    }
};

#endif // MY_QUEUE_H