# volatile 

> volatile tells the compiler not to optimize anything that has to do with the volatile variable.

可以简单理解为， volatile 是在编译器层面禁用涉及 volatile 变量的代码。

使用情况有三
- When you interface with hardware that changes the value itself
- when there's another thread running that also uses the variable
- when there's a signal handler that might change the value of the variable.


volatile 内存语义

1. 可见性保证
```cpp
// 线程A
shared_data = 42;  // 普通变量，可能被缓存在寄存器

// 线程B
// 可能看到旧值，因为修改可能还在CPU缓存中

// 使用 volatile
volatile int shared_data = 0;
// 写入会立即刷新到主内存
// 读取会直接从主内存获取最新值
```
2. 顺序性保证(有限，在CPU硬件上仍可能重排，需要加上内存屏障)
```cpp
volatile int* p = (volatile int*)0x1234;
volatile int* q = (volatile int*)0x5678;

*p = 1;     // 操作1
*q = 2;     // 操作2
// 编译器保证：操作1在操作2之前执行
// 但CPU硬件仍可能重排，需要内存屏障

```
