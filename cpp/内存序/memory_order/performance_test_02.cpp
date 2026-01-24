/*
 * 无锁原子操作 vs 有锁同步性能对比测试
 * 编译命令: g++ -std=c++17 -pthread -O2 performance_test_02.cpp -o performance_test_02
 * 运行命令: ./performance_test_02
 */

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <functional>

// ==================== 测试配置 ====================
const int NUM_THREADS = 8;           // 线程数量
const int ITERATIONS = 1000000;      // 每个线程的迭代次数

// ==================== 无锁原子操作计数器 ====================
class AtomicCounter {
private:
    std::atomic<long long> counter{0};
    
public:
    void increment() {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
    
    long long get() const {
        return counter.load(std::memory_order_relaxed);
    }
    
    void reset() {
        counter.store(0, std::memory_order_relaxed);
    }
};

// ==================== 互斥锁计数器 ====================
class MutexCounter {
private:
    long long counter = 0;
    mutable std::mutex mtx;
    
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mtx);
        ++counter;
    }
    
    long long get() const {
        std::lock_guard<std::mutex> lock(mtx);
        return counter;
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(mtx);
        counter = 0;
    }
};

// ==================== 无锁栈（使用 CAS 操作 + 安全内存管理）====================
template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        // 使用延迟删除代替引用计数来管理节点生命周期，避免并发删除导致的双重释放
        Node(const T& val) : data(val), next(nullptr) {}
    };
    
    std::atomic<Node*> head{nullptr};
    std::atomic<int> size{0};
    
    // 延迟删除队列（被移除的节点会统一放到这个链表，在析构时释放）
    std::atomic<Node*> toDelete{nullptr};

    // 将节点加入到延迟删除链表（无锁 push）
    void addToDelete(Node* node) {
        Node* old = toDelete.load(std::memory_order_acquire);
        do {
            node->next.store(old, std::memory_order_relaxed);
        } while (!toDelete.compare_exchange_weak(old, node,
                                               std::memory_order_release,
                                               std::memory_order_acquire));
    }
    
public:
    void push(const T& val) {
        Node* newNode = new Node(val);
        Node* oldHead = head.load(std::memory_order_acquire);

        do {
            newNode->next.store(oldHead, std::memory_order_relaxed);
        } while (!head.compare_exchange_weak(oldHead, newNode,
                                             std::memory_order_release,
                                             std::memory_order_acquire));
        
        size.fetch_add(1, std::memory_order_relaxed);
    }
    
    bool pop(T& val) {
        Node* oldHead = head.load(std::memory_order_acquire);

        while (oldHead) {
            Node* next = oldHead->next.load(std::memory_order_acquire);

            if (head.compare_exchange_weak(oldHead, next,
                                          std::memory_order_release,
                                          std::memory_order_acquire)) {
                val = oldHead->data;
                size.fetch_sub(1, std::memory_order_relaxed);

                // 不直接 delete，改为加入延迟删除链表，统一在析构时释放
                addToDelete(oldHead);
                return true;
            }
            // CAS 失败，oldHead 已由 compare_exchange_weak 更新为新的 head，继续重试
        }
        return false;
    }
    
    int getSize() const {
        return size.load(std::memory_order_relaxed);
    }
    
    ~LockFreeStack() {
        // 先把栈中剩余节点移到延迟删除链表中
        T val;
        while (pop(val)) {}

        // 释放延迟删除链表中的节点
        Node* node = toDelete.exchange(nullptr, std::memory_order_acq_rel);
        while (node) {
            Node* next = node->next.load(std::memory_order_acquire);
            delete node;
            node = next;
        }
    }
};

// ==================== 有锁栈 ====================
template<typename T>
class MutexStack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& val) : data(val), next(nullptr) {}
    };
    
    Node* head = nullptr;
    int stackSize = 0;
    mutable std::mutex mtx;
    
public:
    void push(const T& val) {
        std::lock_guard<std::mutex> lock(mtx);
        Node* newNode = new Node(val);
        newNode->next = head;
        head = newNode;
        ++stackSize;
    }
    
    bool pop(T& val) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!head) return false;
        
        Node* oldHead = head;
        val = oldHead->data;
        head = head->next;
        delete oldHead;
        --stackSize;
        return true;
    }
    
    int getSize() const {
        std::lock_guard<std::mutex> lock(mtx);
        return stackSize;
    }
    
    ~MutexStack() {
        std::lock_guard<std::mutex> lock(mtx);
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

// ==================== 性能测试框架 ====================
template<typename Func>
double measureTime(Func func, const std::string& name) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> elapsed = end - start;
    double ms = elapsed.count();
    
    std::cout << std::left << std::setw(40) << name 
              << std::right << std::setw(12) << std::fixed 
              << std::setprecision(2) << ms << " ms" << std::endl;
    
    return ms;
}

// ==================== 测试用例 ====================
void testAtomicCounter() {
    AtomicCounter counter;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < ITERATIONS; ++j) {
                counter.increment();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

void testMutexCounter() {
    MutexCounter counter;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < ITERATIONS; ++j) {
                counter.increment();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

void testLockFreeStack() {
    LockFreeStack<int> stack;
    std::vector<std::thread> threads;
    
    // 一半线程 push，一半线程 pop
    for (int i = 0; i < NUM_THREADS / 2; ++i) {
        threads.emplace_back([&stack]() {
            for (int j = 0; j < ITERATIONS; ++j) {
                stack.push(j);
            }
        });
    }
    
    for (int i = 0; i < NUM_THREADS / 2; ++i) {
        threads.emplace_back([&stack]() {
            int val;
            for (int j = 0; j < ITERATIONS; ++j) {
                stack.pop(val);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

void testMutexStack() {
    MutexStack<int> stack;
    std::vector<std::thread> threads;
    
    // 一半线程 push，一半线程 pop
    for (int i = 0; i < NUM_THREADS / 2; ++i) {
        threads.emplace_back([&stack]() {
            for (int j = 0; j < ITERATIONS; ++j) {
                stack.push(j);
            }
        });
    }
    
    for (int i = 0; i < NUM_THREADS / 2; ++i) {
        threads.emplace_back([&stack]() {
            int val;
            for (int j = 0; j < ITERATIONS; ++j) {
                stack.pop(val);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

// ==================== 主函数 ====================
int main() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  无锁原子操作 vs 互斥锁性能对比测试" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "线程数量: " << NUM_THREADS << std::endl;
    std::cout << "每线程迭代次数: " << ITERATIONS << std::endl;
    std::cout << "总操作次数: " << (NUM_THREADS * ITERATIONS) << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 测试 1: 计数器性能对比
    std::cout << "【测试 1: 简单计数器递增】" << std::endl;
    double atomicCounterTime = measureTime(testAtomicCounter, "无锁原子计数器 (atomic)");
    double mutexCounterTime = measureTime(testMutexCounter, "互斥锁计数器 (mutex)");
    
    std::cout << "\n性能提升: " << std::fixed << std::setprecision(2) 
              << (mutexCounterTime / atomicCounterTime) << "x" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 测试 2: 栈操作性能对比
    std::cout << "【测试 2: 并发栈 Push/Pop 操作】" << std::endl;
    double lockFreeStackTime = measureTime(testLockFreeStack, "无锁栈 (lock-free stack)");
    double mutexStackTime = measureTime(testMutexStack, "互斥锁栈 (mutex stack)");
    
    std::cout << "\n性能提升: " << std::fixed << std::setprecision(2) 
              << (mutexStackTime / lockFreeStackTime) << "x" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // 总结
    std::cout << "【测试总结】" << std::endl;
    std::cout << "1. 原子操作避免了线程阻塞，减少了上下文切换开销" << std::endl;
    std::cout << "2. 在高并发场景下，无锁结构通常有更好的性能" << std::endl;
    std::cout << "3. 无锁编程复杂度更高，需要仔细处理内存序问题" << std::endl;
    std::cout << "4. 选择方案需要根据实际场景权衡性能和复杂度" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    return 0;
}
