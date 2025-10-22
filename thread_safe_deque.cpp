#include <iostream>
#include <deque>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

template <typename T>
class ThreadSafeDeque {
private:
    std::deque<T> dq;
    std::mutex mtx;
public:
    void push_front(const T& v) {
        std::lock_guard<std::mutex> lock(mtx);
        dq.push_front(v);
    }
    void push_back(const T& v) {
        std::lock_guard<std::mutex> lock(mtx);
        dq.push_back(v);
    }
    bool pop_front(T& out) {
        std::lock_guard<std::mutex> lock(mtx);
        if (dq.empty()) return false;
        out = dq.front();
        dq.pop_front();
        return true;
    }
    bool pop_back(T& out) {
        std::lock_guard<std::mutex> lock(mtx);
        if (dq.empty()) return false;
        out = dq.back();
        dq.pop_back();
        return true;
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return dq.empty();
    }
    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return dq.size();
    }
};

void dequeTest() {
    std::cout << "Exercise: Thread-Safe Deque\n";
    ThreadSafeDeque<int> dq;
    auto worker1 = [&]() {
        for (int i = 0; i < 10; ++i) { dq.push_back(i); std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
    };
    auto worker2 = [&]() {
        for (int i = 100; i < 110; ++i) { dq.push_front(i); std::this_thread::sleep_for(std::chrono::milliseconds(15)); }
    };
    auto popper = [&]() {
        for (int i = 0; i < 10; ++i) {
            int v;
            if (dq.pop_front(v)) std::cout << "Pop front " << v << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    };

    std::thread t1(worker1), t2(worker2), t3(popper);
    t1.join(); t2.join(); t3.join();
}

int main() {
    dequeTest();
    return 0;
}