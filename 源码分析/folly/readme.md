# 锁

锁是序列化访问共享资源的一种机制

## SharedMutex 读写锁
## RWSpinLock 纯自旋锁

RWSpinLock 默认优先权是读锁，而SharedMutex 默认优先权是写锁，但是其读写均可。

## Upgradeable 语义
