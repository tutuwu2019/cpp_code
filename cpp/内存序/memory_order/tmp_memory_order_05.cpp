#include <atomic>
#include <cassert>
#include <thread>
#include <string>
#include <iostream>

std::atomic<std::string*> ptr;
int data;

void thread_1(){
    data = 43;
    std::cout<<"thread_1 func, data: "<<data<<std::endl;
}
void producer(){
    std::string* p = new std::string("hello");
    data = 42;
    ptr.store(p, std::memory_order_release);
}

// sonsume 模式下，CPU可能会任务，既然 data  与 p2 没有关系，我可以先读 p2 之前，先从 缓存中把 data旧值读出来。
// 结果就是 虽然读到了最新的指针p2，但是 data可能拿到的是旧值
// 为什么要设计这么奇怪的 consumer 呢？
// 为了极致性能。在很多硬件架构中(尤其是 ARM和 PowerPC)上， 
// Acquire 需要插入一条特殊的内存屏障(Memory Barrier)，这会稍微拖慢速度。
// Consumer 则不需要任何额外的指令。因为硬件本身就无法在不知道地址的情况下读取地址里的内容(即硬件天然支持数据依赖同步，注意可能获取的是过时的数据/内容)
// 理想与现实：目前的尴尬处境
// 虽然 consume 在理论上很美，但是在实际开发中：
//  1.编译器很难搞定它，编译器优化时经常会无意中破坏数据依赖链(例如：x = a -a; 结果为 0 ，但是编译器可能觉得 x 不在依赖 a 了)
//  2. 便准委员会的建议：由于实现太复杂，目前的C++标准实际上不鼓励直接使用 connsume。大多数编译器会直接把 consume 提升为 acquire来处理，以确保安全性            



// Note that currently (2 / 2015) no known production compilers track dependency chains: consume operations are lifted to acquire operations.
// 没有生产级编译器能追踪依赖链。本质上是因为编译器在追求”运行速度“时，会无意识地破坏掉 consume 赖以生存地”逻辑线“
// 1. 什么是依赖链？ 在 consume 地理想世界里，编译器必须像侦探一样盯着每一个变量。
// 2. 为什么编译器“追踪”不了？ 编译器最核心的工作是优化。而优化往往会“切断”依赖链，让编译器变瞎
void consumer(){
    std::string* p2;

    while(!(p2 = ptr.load(std::memory_order_consume)));

    assert(*p2 == "hello"); // CPU 层面，要读取 *p2 的内容，必须先知道p2 的地址。这种“先有地址，后有内容”的物理限制，被称为 Carries Dependency (也叫携带依赖)
    assert(data == 42);     // 可能会被触发也可能不会被出发。但是如果采用memory_order_acquire 必定不会触发
    std::cout<<"p2 : "<<*p2<<", data : "<<data<<std::endl;
    delete p2;
}

// 发布--消费顺序
//  一旦原子加载操作完成，线程B中所有使用该加载值的操作符和函数都必然能够看到线程A写入内存的内容
// 同步仅在释放和使用同一原子变量的线程之间建立联系。其他线程看到的内存访问顺序可能与同步线程中的一个或两个都不同
// 除了 DEC Aloha 之外，所有主流CPU的依赖关系排序都是自动的，这种同步模式不会发出额外的CPU指令，只会影响某些编译器优化(例如，禁止编译器对依赖链中的对象执行推测性加载)
// 这种排序的典型用力包括对很少写入的并发数据结构(路由表、配置、安全策略、防火墙规则等)的读取访问，以及采取指针介导发布的发布者--订阅者场景，即生产者发布一个指针，消费者可以通过该指针访问信息：无需将生产者写入内存的所有其他内容都对消费者可见。（这在弱有序架构上可能是一项开销很大的操作）。rcu_dereference 就是这种场景的一个例子。
int main(){
    
    std::thread t1(producer);
    //std::this_thread::sleep_for(std::chrono::seconds(2));
    std::thread t(thread_1);
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    std::thread t2(consumer);

    std::cout<<"the end of ptr: "<<*ptr<<", the data: "<<data<<std::endl;
    t1.join();
    t2.join();

    t.join();
    return 0;
    
}
