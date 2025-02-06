#include "queue.h"
#include <iostream>
#include <thread>

void writer(Queue<int>& q) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Push(1)" << std::endl;
    q.Push(1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Push(2)" << std::endl;
    q.Push(2);
    
    std::cout << "Push(3)" << std::endl;
    q.Push(3);
    
    std::cout << "Push(4) // Element 2 dropped!" << std::endl;
    q.Push(4);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Push(5)" << std::endl;
    q.Push(5);
}

void reader(Queue<int>& q) {
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    std::cout << "Pop() -> " << q.Pop() << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Pop() -> " << q.Pop() << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Pop() -> " << q.Pop() << std::endl;
    
    std::cout << "Pop() // blocks" << std::endl;
    std::cout << "-> " << q.Pop() << " // is released" << std::endl;
}

int main() {
    Queue<int> q(2);
    
    std::thread writerThread(writer, std::ref(q));
    std::thread readerThread(reader, std::ref(q));
    
    writerThread.join();
    readerThread.join();
    
    return 0;
}
