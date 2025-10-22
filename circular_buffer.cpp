#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

class ThreadSafeCircularBuffer {
private:
    std::vector<int> buf;
    size_t capacity;
    size_t head = 0;
    size_t tail = 0;
    size_t count = 0;
    std::mutex mtx;
    std::condition_variable not_full;
    std::condition_variable not_empty;
public:
    ThreadSafeCircularBuffer(size_t n = 8) : buf(n), capacity(n) {}
    void push(int v) {
        std::unique_lock<std::mutex> lock(mtx);
        not_full.wait(lock, [&]{ return count < capacity; });
        buf[tail] = v;
        tail = (tail + 1) % capacity;
        ++count;
        not_empty.notify_one();
    }
    bool pop(int &out) {
        std::unique_lock<std::mutex> lock(mtx);
        if (count == 0) return false;
        out = buf[head];
        head = (head + 1) % capacity;
        --count;
        not_full.notify_one();
        return true;
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return count == 0;
    }
    bool full() {
        std::lock_guard<std::mutex> lock(mtx);
        return count == capacity;
    }
    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return count;
    }
};

void circularBufferTest() {
    std::cout << "Exercise: Thread-Safe Circular Buffer\n";
    ThreadSafeCircularBuffer cb(6);
    const int NUM_PRODUCERS = 2;
    const int NUM_CONSUMERS = 2;
    const int ITEMS = 12;

    auto producer = [&](int id) {
        for (int i = 0; i < ITEMS/NUM_PRODUCERS; ++i) {
            int v = id * 100 + i;
            cb.push(v);
            std::cout << "Produced " << v << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    };

    auto consumer = [&](int id) {
        for (;;) {
            int v;
            if (cb.pop(v)) {
                std::cout << "Consumed " << v << std::endl;
            } else {
                if (cb.empty()) break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    std::vector<std::thread> producers, consumers;
    for (int i = 0; i < NUM_PRODUCERS; ++i) producers.emplace_back(producer, i);
    for (int i = 0; i < NUM_CONSUMERS; ++i) consumers.emplace_back(consumer, i);
    for (auto &t : producers) t.join();
    // give consumers time to drain
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    for (auto &t : consumers) t.join();
}

int main() {
    circularBufferTest();
    return 0;
}