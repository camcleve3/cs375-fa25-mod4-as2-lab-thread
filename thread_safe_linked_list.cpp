#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

// A simple thread-safe singly linked list with coarse-grained locking
template <typename T>
class ThreadSafeLinkedList {
private:
    struct Node {
        T value;
        Node* next;
        Node(const T& v) : value(v), next(nullptr) {}
    };
    Node* head;
    std::mutex mtx;
public:
    ThreadSafeLinkedList() : head(nullptr) {}
    ~ThreadSafeLinkedList() {
        while (head) {
            Node* n = head;
            head = head->next;
            delete n;
        }
    }
    void push_front(const T& v) {
        std::lock_guard<std::mutex> lock(mtx);
        Node* n = new Node(v);
        n->next = head;
        head = n;
    }
    bool pop_front(T& out) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!head) return false;
        Node* n = head;
        out = n->value;
        head = head->next;
        delete n;
        return true;
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return head == nullptr;
    }
    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        size_t c = 0;
        Node* cur = head;
        while (cur) { ++c; cur = cur->next; }
        return c;
    }
};

void linkedListTest() {
    std::cout << "Exercise: Thread-Safe Linked List\n";
    ThreadSafeLinkedList<int> list;
    auto pusher = [&](int id) {
        for (int i = 0; i < 10; ++i) {
            list.push_front(id * 100 + i);
        }
    };
    auto popper = [&]() {
        for (int i = 0; i < 10; ++i) {
            int v;
            if (list.pop_front(v)) std::cout << "Popped " << v << std::endl;
        }
    };

    std::vector<std::thread> threads;
    threads.emplace_back(pusher, 1);
    threads.emplace_back(pusher, 2);
    threads.emplace_back(popper);
    for (auto &t : threads) t.join();
    std::cout << "Final size: " << list.size() << std::endl;
}

int main() {
    linkedListTest();
    return 0;
}