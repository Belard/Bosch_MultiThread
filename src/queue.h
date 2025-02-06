#ifndef QUEUE_H
#define QUEUE_H

#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <memory>
#include <chrono>

/**
 * @class Queue
 * @brief A thread-safe queue supporting multi-threaded communication.
 *
 * @tparam T The type of elements stored in the queue.
 */
template <typename T>
class Queue {
public:
    explicit Queue(int size)
        : capacity_(size), size_(0), head_(0), tail_(0) {
        buffer_ = std::make_unique<T[]>(capacity_);
    }

    void Push(T element) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (size_ == capacity_) {
            head_ = (head_ + 1) % capacity_; // Drop the oldest element
            --size_;
        }
        buffer_[tail_] = element;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
        cond_var_.notify_one();
    }

    T Pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this] { return size_ > 0; });
        return popInternal();
    }

    T PopWithTimeout(int milliseconds) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cond_var_.wait_for(lock, std::chrono::milliseconds(milliseconds), [this] { return size_ > 0; })) {
            throw std::runtime_error("Timeout while waiting for an element");
        }
        return popInternal();
    }

    int Count() {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }

    int Size() {
        return capacity_;
    }

private:
    T popInternal() {
        T value = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        --size_;
        return value;
    }

    int capacity_;
    int size_;
    int head_, tail_;
    std::unique_ptr<T[]> buffer_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
};

#endif // QUEUE_H
