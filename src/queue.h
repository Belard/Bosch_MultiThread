#ifndef QUEUE_H
#define QUEUE_H

#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <memory>
#include <chrono>

/**
 * @class Queue
 * @brief A thread-safe, fixed-size queue that supports multi-threaded communication.
 *
 * This queue operates in a circular buffer fashion, where the oldest element is
 * dropped if the queue reaches capacity. It provides blocking and timed retrieval
 * methods for efficient multi-threaded use.
 *
 * @tparam T The type of elements stored in the queue.
 */
template <typename T>
class Queue {
public:
    /**
     * @brief Constructs a Queue with a given capacity.
     * @param size The maximum number of elements the queue can hold.
     */
    explicit Queue(int size)
        : capacity_(size), size_(0), head_(0), tail_(0) {
        buffer_ = std::make_unique<T[]>(capacity_);
    }

    /**
     * @brief Pushes an element into the queue.
     *
     * If the queue is full, the oldest element is removed to make room.
     *
     * @param element The element to be added.
     */
    void Push(T element) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (size_ == capacity_) {
            head_ = (head_ + 1) % capacity_; ///< Drop the oldest element
            --size_;
        }
        buffer_[tail_] = element;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
        cond_var_.notify_one();
    }

    /**
     * @brief Removes and returns an element from the queue.
     *
     * This function blocks if the queue is empty until an element is available.
     *
     * @return The element removed from the queue.
     */
    T Pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this] { return size_ > 0; });
        return popInternal();
    }

    /**
     * @brief Removes and returns an element from the queue with a timeout.
     *
     * If the queue is empty, this function waits for up to `milliseconds` time
     * before throwing an exception.
     *
     * @param milliseconds The maximum time to wait before throwing an exception.
     * @return The element removed from the queue.
     * @throws std::runtime_error if the timeout expires before an element is available.
     */
    T PopWithTimeout(int milliseconds) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cond_var_.wait_for(lock, std::chrono::milliseconds(milliseconds), [this] { return size_ > 0; })) {
            throw std::runtime_error("Timeout while waiting for an element");
        }
        return popInternal();
    }

    /**
     * @brief Returns the current number of elements in the queue.
     * @return The number of elements currently stored.
     */
    int Count() {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }

    /**
     * @brief Returns the maximum size of the queue.
     * @return The maximum number of elements the queue can hold.
     */
    int Size() {
        return capacity_;
    }

private:
    /**
     * @brief Removes and returns the front element of the queue.
     *
     * This function is used internally by Pop() and PopWithTimeout().
     *
     * @return The removed element.
     */
    T popInternal() {
        T value = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        --size_;
        return value;
    }

    int capacity_;                  ///< Maximum number of elements the queue can hold
    int size_;                       ///< Current number of elements in the queue
    int head_, tail_;                ///< Indices for the front and rear of the queue
    std::unique_ptr<T[]> buffer_;    ///< Buffer storing queue elements
    std::mutex mutex_;               ///< Mutex for thread safety
    std::condition_variable cond_var_; ///< Condition variable for blocking pop operations
};

#endif // QUEUE_H
