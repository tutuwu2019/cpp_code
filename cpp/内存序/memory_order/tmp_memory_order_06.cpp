#include <atomic>
#include <cassert>
#include <thread>
#include <iostream>
 
std::atomic<bool> x = {false};
std::atomic<bool> y = {false};
std::atomic<int> z = {0};
 
void write_x()
{
    x.store(true, std::memory_order_seq_cst);
}
 
void write_y()
{
    y.store(true, std::memory_order_seq_cst);
}
 
void read_x_then_y()
{
    while (!x.load(std::memory_order_seq_cst))
        ;
    if (y.load(std::memory_order_seq_cst))
        ++z;
}
 
void read_y_then_x()
{
    while (!y.load(std::memory_order_seq_cst))
        ;
    if (x.load(std::memory_order_seq_cst))
        ++z;
}

// memory_order_seq_cst，所有标记为 seq_cst 的操作都发生在“全球统一的时间轴上”。就像一场直播，全世界(所有线程)看到的事件发生顺序必须是完全一致的。
int main()
{   
    std::thread b(write_y);
    std::thread a(write_x);
    

    std::thread d(read_y_then_x);
    std::thread c(read_x_then_y);
    
    a.join(); b.join(); 
    c.join(); d.join();
    assert(z.load() != 0); // will never happen
    std::cout<<"z : "<<z.load()<<std::endl;
}
