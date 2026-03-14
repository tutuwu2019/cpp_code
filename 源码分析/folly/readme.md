# 锁

锁是序列化访问共享资源的一种机制。核心目的是维护不变量，防止数据竞争。  
使用锁是有代价的，包括：上下文切换、缓存一致性、串行化瓶颈

> “锁是并发编程中最后的手段，而不是首选。” 能无锁则无锁，能减少共享则减少共享。

原子变量与锁的差异

```cpp

std::atomic<int> balance{100};

void withdraw() {

    // 步骤 1: 读取

    if (balance.load() > 0) { 

        // 步骤 2: 写入

        balance.fetch_sub(10); 

    }

}

// ..........
std::mutex mtx;

int balance{100}; // 普通 int 即可

void withdraw() {

    std::lock_guard<std::mutex> lock(mtx); // 锁住整个临界区

    if (balance > 0) {

        balance -= 10;

    }

}
```

第一段代码 可能会这样触发
```text
Thread A                Thread B

load -> 10
                        load -> 10
fetch_sub(10) -> 0
                        fetch_sub(10) -> -10
```
而第二段代码会这样触发
```text
Thread A

lock
balance = 10
withdraw -> 0
unlock

Thread B

lock
balance = 0
if false
unlock
```
细微的差别是，前者金额的修改是原子操作，而后者是金额修改的过程是原子操作



## SharedMutex 读写锁
## RWSpinLock 纯自旋锁

RWSpinLock 默认优先权是读锁，而SharedMutex 默认优先权是写锁，但是其读写均可。

## Upgradeable 语义
