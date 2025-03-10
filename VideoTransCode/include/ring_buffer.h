#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <mutex>
#include <condition_variable>
#include <vector>
#include <cstring>
#include <atomic>

class RingBuffer
{
private:
    std::vector<uint8_t> buffer_;
    size_t capacity_;
    size_t readPos_;
    size_t writePos_;
    size_t dataSize_;

    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;

    std::atomic<bool> quit_;

public:
    explicit RingBuffer(size_t capacity = 1024 * 1024)
        : capacity_(capacity), readPos_(0), writePos_(0), dataSize_(0), quit_(false)
    {
        buffer_.resize(capacity);
    }

    ~RingBuffer()
    {
        setQuit();
    }

    size_t write(const uint8_t *data, size_t size, bool blocking = true)
    {
        if (!data || size == 0 || quit_)
        {
            return 0;
        }

        std::unique_lock<std::mutex> lock(mutex_);

        if (blocking)
        {
            while (dataSize_ >= capacity_ && !quit_)
            {
                notFull_.wait(lock);
            }

            if (quit_)
            {
                return 0;
            }
        }
        else
        {
            if (dataSize_ >= capacity_)
            {
                return 0;
            }

            if (size > capacity_ - dataSize_)
            {
                size = capacity_ - dataSize_;
            }
        }

        size_t firstPart = std::min(size, capacity_ - writePos_);
        std::memcpy(buffer_.data() + writePos_, data, firstPart);

        if (firstPart < size)
        {
            std::memcpy(buffer_.data(), data + firstPart, size - firstPart);
        }

        writePos_ = (writePos_ + size) % capacity_;
        dataSize_ += size;

        notEmpty_.notify_one();

        return size;
    }

    size_t read(uint8_t *data, size_t size, bool blocking = true)
    {
        if (!data || size == 0)
        {
            return 0;
        }

        std::unique_lock<std::mutex> lock(mutex_);

        if (blocking)
        {
            while (dataSize_ == 0 && !quit_)
            {
                notEmpty_.wait(lock);
            }

            if (dataSize_ == 0)
            {
                return 0;
            }
        }
        else
        {
            if (dataSize_ == 0)
            {
                return 0;
            }
        }

        size = std::min(size, dataSize_);

        size_t firstPart = std::min(size, capacity_ - readPos_);
        std::memcpy(data, buffer_.data() + readPos_, firstPart);

        if (firstPart < size)
        {
            std::memcpy(data + firstPart, buffer_.data(), size - firstPart);
        }

        readPos_ = (readPos_ + size) % capacity_;
        dataSize_ -= size;

        notFull_.notify_one();

        return size;
    }

    size_t dataSize() const
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return dataSize_;
    }

    size_t freeSize() const
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return capacity_ - dataSize_;
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        readPos_ = 0;
        writePos_ = 0;
        dataSize_ = 0;
        notFull_.notify_all();
    }

    void setQuit()
    {
        quit_ = true;
        notEmpty_.notify_all();
        notFull_.notify_all();
    }

    void resetQuit()
    {
        quit_ = false;
    }

    bool isQuit() const
    {
        return quit_;
    }

    size_t capacity() const
    {
        return capacity_;
    }

    bool resize(size_t newCapacity)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (dataSize_ > newCapacity)
        {
            return false;
        }

        std::vector<uint8_t> newBuffer(newCapacity);

        if (dataSize_ > 0)
        {
            if (readPos_ < writePos_)
            {
                std::memcpy(newBuffer.data(), buffer_.data() + readPos_, dataSize_);
            }
            else
            {
                size_t firstPart = capacity_ - readPos_;
                std::memcpy(newBuffer.data(), buffer_.data() + readPos_, firstPart);
                std::memcpy(newBuffer.data() + firstPart, buffer_.data(), writePos_);
            }
        }

        buffer_ = std::move(newBuffer);
        capacity_ = newCapacity;
        readPos_ = 0;
        writePos_ = dataSize_;

        return true;
    }
};

#endif // RING_BUFFER_H
