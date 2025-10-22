#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <chrono>

// Thread-safe queue
template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> q;
    std::mutex mtx;
public:
    ThreadSafeQueue() = default;
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(value);
    }
    bool pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (q.empty()) return false;
        value = q.front();
        q.pop();
        return true;
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return q.empty();
    }
    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return q.size();
    }
};

// Thread-safe stack
template <typename T>
class ThreadSafeStack {
private:
    std::stack<T> s;
    std::mutex mtx;
public:
    ThreadSafeStack() = default;
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        s.push(value);
    }
    bool pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (s.empty()) return false;
        value = s.top();
        s.pop();
        return true;
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return s.empty();
    }
    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return s.size();
    }
};

// Producer-consumer test
void producerConsumerTest() {
    std::cout << "Problem 1: Producer-Consumer Simulation\n";
    ThreadSafeQueue<std::string> mq;
    const int NUM_PRODUCERS = 3;
    const int NUM_CONSUMERS = 2;
    const int MESSAGES_PER_PRODUCER = 5;
    std::atomic<int> producedCount{0};

    auto producer = [&](int id) {
        for (int i = 0; i < MESSAGES_PER_PRODUCER; ++i) {
            std::string msg = "Producer " + std::to_string(id) + " - Message " + std::to_string(i);
            mq.push(msg);
            producedCount.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    auto consumer = [&](int id) {
        while (true) {
            std::string msg;
            if (mq.pop(msg)) {
                std::cout << "Consumer " << id << " processed: " << msg << std::endl;
            } else {
                if (producedCount.load() >= NUM_PRODUCERS * MESSAGES_PER_PRODUCER && mq.empty()) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
    };

    std::vector<std::thread> producers, consumers;
    for (int i = 0; i < NUM_PRODUCERS; ++i) producers.emplace_back(producer, i);
    for (int i = 0; i < NUM_CONSUMERS; ++i) consumers.emplace_back(consumer, i);

    for (auto &t : producers) t.join();
    for (auto &t : consumers) t.join();
}

// Undo-redo test using thread-safe stacks
void undoRedoTest() {
    std::cout << "\nProblem 2: Undo-Redo System\n";
    ThreadSafeStack<std::string> undoStack;
    ThreadSafeStack<std::string> redoStack;
    std::string current = "Initial";

    auto edit = [&](const std::string& newText) {
        undoStack.push(current);
        current = newText;
        // clear redo stack (simple destructive loop)
        std::string tmp;
        while (redoStack.pop(tmp)) {}
        std::cout << "Edit: " << current << std::endl;
    };

    auto undo = [&]() {
        std::string prev;
        if (undoStack.pop(prev)) {
            redoStack.push(current);
            current = prev;
            std::cout << "Undo -> " << current << std::endl;
        } else {
            std::cout << "Undo stack empty\n";
        }
    };

    auto redo = [&]() {
        std::string next;
        if (redoStack.pop(next)) {
            undoStack.push(current);
            current = next;
            std::cout << "Redo -> " << current << std::endl;
        } else {
            std::cout << "Redo stack empty\n";
        }
    };

    edit("Hello");
    edit("Hello World");
    undo();
    undo();
    redo();
    edit("Hello Galaxy");
    undo();
}

int main() {
    producerConsumerTest();
    undoRedoTest();
    return 0;
}