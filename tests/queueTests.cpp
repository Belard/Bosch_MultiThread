/**
 * @file queue_test.cpp
 * @brief Test suite for the Queue class.
 * 
 * This file contains multiple test cases for the Queue class that test various
 * aspects such as basic operations (push, pop), handling overflow, timeout behavior, 
 * thread-safety, and blocking behavior on empty queues.
 */

 #include "gtest/gtest.h"
 #include "queue.h"
 #include <thread>
 #include <chrono>
 #include <stdexcept>
 
 /**
  * @brief Test basic push and pop functionality of the queue.
  * 
  * This test verifies that elements are pushed and popped correctly in the order 
  * they were inserted. It pushes three elements into the queue and checks that 
  * they are popped in the correct order.
  */
 TEST(queueTest, PushPopTest) {
     Queue<int> q(3);  ///< Create a queue of size 3
 
     // Push elements into the queue
     q.Push(1);
     q.Push(2);
     q.Push(3);
 
     // Pop elements and verify the order
     EXPECT_EQ(q.Pop(), 1);  ///< Should be 1, as it's the first element
     EXPECT_EQ(q.Pop(), 2);  ///< Should be 2
     EXPECT_EQ(q.Pop(), 3);  ///< Should be 3
 }
 
 /**
  * @brief Test overflow behavior of the queue.
  * 
  * This test pushes more elements into the queue than its capacity. It verifies 
  * that the oldest element is removed when the queue overflows and that the 
  * queue operates correctly after the overflow.
  */
 TEST(queueTest, OverflowTest) {
     Queue<int> q(3);  ///< Create a queue of size 3
 
     // Push elements into the queue
     q.Push(1);
     q.Push(2);
     q.Push(3);
 
     // Push another element, which should remove the oldest (1)
     q.Push(4);
 
     // Pop and check that the first element is now 2 (since 1 was removed)
     EXPECT_EQ(q.Pop(), 2);
     EXPECT_EQ(q.Pop(), 3);
     EXPECT_EQ(q.Pop(), 4);
 }
 
 /**
  * @brief Test the count and size of the queue.
  * 
  * This test ensures that the count reflects the number of elements in the 
  * queue after pushing elements, and that the size remains constant according 
  * to the queue's initial capacity.
  */
 TEST(queueTest, CountAndSizeTest) {
     Queue<int> q(3);  ///< Create a queue of size 3
 
     // Push elements into the queue
     q.Push(1);
     q.Push(2);
 
     // Verify the count and size
     EXPECT_EQ(q.Count(), 2);  ///< Should be 2 after 2 pushes
     EXPECT_EQ(q.Size(), 3);   ///< Size of the queue should still be 3
 }
 
 /**
  * @brief Test pop with timeout behavior.
  * 
  * This test verifies the behavior of the `PopWithTimeout` function. It ensures 
  * that the function throws a runtime error if no element is available within 
  * the specified timeout and that it returns the correct value when an element 
  * is available.
  */
 TEST(queueTest, PopWithTimeoutTest) {
     Queue<int> q(3);  ///< Create a queue of size 3
 
     // Case 1: Pop with timeout before any push
     EXPECT_THROW(q.PopWithTimeout(100), std::runtime_error);  ///< Should throw due to timeout
 
     // Case 2: Push an element and pop with timeout
     q.Push(10);
     EXPECT_EQ(q.PopWithTimeout(100), 10);  ///< Should return 10
 }
 
 /**
  * @brief Test blocking behavior when popping from an empty queue.
  * 
  * This test ensures that when the queue is empty, a pop operation blocks 
  * until an element is pushed into the queue.
  */
 TEST(queueTest, EmptyQueuePopBlockTest) {
     Queue<int> q(3);
 
     // Create a thread that will block on pop until an element is pushed
     std::thread pop_thread([&]() {
         EXPECT_EQ(q.Pop(), 1);  ///< This will block until pushed
     });
 
     std::this_thread::sleep_for(std::chrono::milliseconds(100));  ///< Let pop start blocking
     q.Push(1);  ///< Push an element, which should unblock pop
     pop_thread.join();
 }
 
 /**
  * @brief Test the thread-safety of the queue.
  * 
  * This test ensures that the queue works correctly in a multi-threaded environment.
  * It uses two threads: one for pushing elements into the queue and another for 
  * popping elements. It verifies that the elements are pushed and popped in the correct order.
  */
 TEST(QueueTest, ThreadSafetyTest) {
     Queue<int> q(3);  ///< Create a queue of size 3
 
     constexpr auto elements_size = 100;
 
     // Shared variables for synchronization
     std::condition_variable cv_push;   ///< Condition variable for push thread
     std::condition_variable cv_pop;    ///< Condition variable for pop thread
     std::mutex mtx_push;               ///< Mutex for push thread synchronization
     std::mutex mtx_pop;                ///< Mutex for pop thread synchronization
     std::atomic<int> push_count(0);    ///< Tracks how many pushes have occurred
     std::atomic<int> pop_count(0);     ///< Tracks how many pops have occurred
     
     // Function to push elements in a thread
     auto push_func = [&q, &cv_push, &mtx_push, &push_count, &pop_count, &cv_pop, &mtx_pop]() {
         for (int i = 0; i < elements_size; ++i) {
             q.Push(i);  ///< Push element into the queue
             {
                 std::lock_guard<std::mutex> lock(mtx_push);
                 ++push_count;  ///< Increment push count
             }
             cv_push.notify_one();  ///< Notify the pop thread that an element has been pushed
             
             // Wait for the pop thread to finish popping before continuing to push
             std::unique_lock<std::mutex> lock(mtx_pop);
             cv_pop.wait(lock, [&pop_count, i] { return pop_count > i; });
         }
     };
 
     // Function to pop elements in a thread
     auto pop_func = [&q, &cv_push, &mtx_push, &cv_pop, &mtx_pop, &push_count, &pop_count]() {
         for (int i = 0; i < elements_size; ++i) {
             // Wait for the push thread to push the element
             std::unique_lock<std::mutex> lock(mtx_push);
             cv_push.wait(lock, [&push_count, i] { return push_count > i; });
             
             // Once notified, pop the element and verify the order
             EXPECT_EQ(q.Pop(), i);  ///< Expect the popped value to match the pushed value
 
             {
                 std::lock_guard<std::mutex> lock(mtx_pop);
                 ++pop_count;  ///< Increment pop count
             }
             cv_pop.notify_one();  ///< Notify the push thread that the pop is complete
         }
     };
 
     // Launch multiple threads
     std::thread push_thread(push_func);
     std::thread pop_thread(pop_func);
 
     // Join the threads
     push_thread.join();
     pop_thread.join();
 }
 