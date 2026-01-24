#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <cstdint>

using namespace std::chrono;

long long test_atomic(unsigned int num_threads, long long increments_per_thread) {
    std::atomic<long long> counter{0};

    auto worker = [&]() {
        for (long long i = 0; i < increments_per_thread; ++i) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    std::vector<std::thread> threads;
    auto start = high_resolution_clock::now();

    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    std::cout << "原子无锁版 (std::atomic) 时间: " << elapsed.count() << " 秒" << std::endl;
    return counter.load();
}

long long test_mutex(unsigned int num_threads, long long increments_per_thread) {
    long long counter = 0;
    std::mutex mtx;

    auto worker = [&]() {
        for (long long i = 0; i < increments_per_thread; ++i) {
            std::lock_guard<std::mutex> lock(mtx);
            ++counter;
        }
    };

    std::vector<std::thread> threads;
    auto start = high_resolution_clock::now();

    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    std::cout << "互斥锁版 (std::mutex) 时间: " << elapsed.count() << " 秒" << std::endl;
    return counter;
}

int main(int argc, char* argv[]) {
    unsigned int num_threads = std::thread::hardware_concurrency();
    long long increments_per_thread = 10000000ULL;  // 10 million

    if (argc >= 2) {
        num_threads = std::stoi(argv[1]);
    }
    if (argc >= 3) {
        increments_per_thread = std::stoull(argv[2]);
    }

    long long expected = num_threads * increments_per_thread;

    std::cout << "线程数: " << num_threads
              << ", 每线程递增次数: " << increments_per_thread
              << ", 预期最终值: " << expected << std::endl << std::endl;

    std::cout << "=== 测试原子无锁版 ===" << std::endl;
    long long atomic_result = test_atomic(num_threads, increments_per_thread);
    std::cout << "最终计数: " << atomic_result << " (正确性: " << (atomic_result == expected ? "OK" : "ERROR") << ")" << std::endl << std::endl;

    std::cout << "=== 测试互斥锁版 ===" << std::endl;
    long long mutex_result = test_mutex(num_threads, increments_per_thread);
    std::cout << "最终计数: " << mutex_result << " (正确性: " << (mutex_result == expected ? "OK" : "ERROR") << ")" << std::endl;

    return 0;
}
