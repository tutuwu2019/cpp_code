#include <atomic>
#include <cassert>
#include <thread>
#include <vector>
#include <iostream>

std::vector<int> data;
std::atomic<int> flag = {0};

void thread_1(){
    data.push_back(42);
    flag.store(1, std::memory_order_release);
    std::cout<<"func thread_1 data push 42, flag store 1"<<std::endl;
}

void thread_2(){
    int expected = 1;
    // compare_exchange_strong 检查 flag 是否为1，如果是，则改成2
    // 虽然这里用了 relaxed ，但是根据 std，原子操作的 读-改-写 能够延续之前的释放序列。
    // 线程2 实际扮演了“接力者”，它读到了线程1的信号，并将其升级为新的信号(从1变成2)
    while(!flag.compare_exchange_strong(expected, 2, std::memory_order_relaxed)){
        expected = 1;
        std::cout<<"expected: "<<expected<<std::endl;
    }
    std::cout<<"the flag had edit, and the val : "<<flag.load()<<std::endl;
}

void thread_3(){
    int i = 1;
    while(flag.load(std::memory_order_acquire) < 2){
        std::cout<<"i: "<<i++<<std::endl;
    }

    assert(data.at(0) == 42);
    std::cout<<"data : "<<data.at(0)<<", flag : "<<flag.load()<<std::endl;
}

// 发布序列，跨三个线程的释放-获取顺序

/**
 *  1. 传递性：线程3并没有直接读取线程1写入的值，但是由于线程2的中间操作，线程1对 data 的修改对线程3 依然可见
 *  2. 极高的性能效率，线程2的优化：线程2在修改时，甚至可以使用 relaxed(在某些复杂场景下)，只要它属于通过一个释放序列。非阻塞：整个过程没有使用 std::mutex。在 x86架构上，这些原子操作几乎等同于普通指令，没有额外的锁开销
 *  3. 逻辑的严密性：尽管 data 是一个非原子性的 std::vector 但是在 thread_3 中调用 data.at(0) 是绝对安全的。。
 */
int main(){
    std::thread c(thread_3);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::thread a(thread_1);
    std::thread b(thread_2);
    

    a.join();
    b.join();
    c.join();

    return 0;
}
