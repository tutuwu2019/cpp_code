#include <atomic>
#include <cassert>
#include <string>
#include <thread>
#include <iostream>

/*
什么是 Acquire 语义？
简单来说，memory_order_acquire 是一条“禁止向后越界”的指令。
对编译器 / 硬件的约束：在 acquire 加载操作之后的所有读写操作，绝对不能被重排序到该 acquire 操作之前。
视觉化理解：想象一扇单向门，acquire 之后的操作可以被关在门后，但门后的操作（后续代码）绝对不能跑到门前去执行。
2. 核心：Acquire-Release 的 “握手” 机制
这是多线程同步中最常用的模式。它们像是在两个线程间建立了一个因果通道：
线程 A (Release)
线程 B (Acquire)
1. 准备数据（写普通变量 data）
2. 循环检查原子变量 flag
3. 发布信号：flag.store(true, release)
4. 获取信号：flag.load(acquire)
结果：信号发出前，数据一定写好了。
结果：一旦读到信号，之前 A 准备的数据一定可见。
关键点：如果线程 B 的 acquire 读到了线程 A 的 release 写入的值，那么线程 A 在 release 之前所做的所有内存写入，对线程 B 在 acquire 之后的操作都是可见的。
*/
std::atomic<std::string*> ptr;
int data;

void producer(){
    std::cout<<"producer..."<<std::endl;
    std::string* p = new std::string("Hello");
    data = 42;
    ptr.store(p, std::memory_order_release);
}

void consumer(){
    std::cout<<"consumer..."<<std::endl;
    std::string* p2;
    while(!(p2 = ptr.load(std::memory_order_acquire)));

    assert(*p2 == "Hello");
    assert(data == 42);
    std::cout<<"consumer end..."<<std::endl;
    delete p2;
}

// 发布获取顺序

int main(){
    
    std::thread t2(consumer);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::thread t1(producer);
    
    
    
    
    t1.join();
    t2.join();


    return 0;
}
