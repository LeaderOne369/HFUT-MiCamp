#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
using namespace std;

template <typename T>
class Ring
{
public:
    Ring(size_t size)
        : buffer(size), capacity(size), head(0), end(0), count(0) {}

    void produce(int id, const T &item)
    {
        unique_lock<mutex> lock(mtx);
        full.wait(lock, [this]()
                  { return count < capacity; });
        buffer[end] = item;
        end = (end + 1) % capacity;
        ++count;
        cout << "生产者" << id << "生产了 " << item << " 号商品。" << endl;
        empty.notify_all();
    }
    T consume(int id)
    {
        unique_lock<mutex> lock(mtx);
        empty.wait(lock, [this]()
                   { return count > 0; });
        T item = buffer[head];
        head = (head + 1) % capacity;
        --count;
        cout << "消费者" << id << "消费了 " << item << " 号商品。" << endl;
        full.notify_all();
        return item;
    }

    bool isEmpty() const
    {
        return count == 0;
    }
    bool isFull() const
    {
        return count == capacity;
    }

private:
    vector<T> buffer;
    size_t capacity;
    size_t head;
    size_t end;
    size_t count;

    mutable mutex mtx;
    condition_variable empty;
    condition_variable full;
};

void producer(Ring<int> &buffer, int id)
{
    for (int i = 0; i < 5; ++i)
    {
        buffer.produce(id, i + id * 100);
    }
}

void consumer(Ring<int> &buffer, int id)
{
    for (int i = 0; i < 5; ++i)
    {
        buffer.consume(id);
    }
}

int main()
{
    const size_t buffer_size = 10;
    Ring<int> buffer(buffer_size);
    vector<thread> t1;
    vector<thread> t2;

    for (int i = 0; i < 3; ++i)
    {
        t1.push_back(thread(producer, ref(buffer), i));
    }
    for (int i = 0; i < 3; ++i)
    {
        t2.push_back(thread(consumer, ref(buffer), i));
    }
    for (auto &p : t1)
    {
        p.join();
    }
    for (auto &c : t2)
    {
        c.join();
    }

    return 0;
}