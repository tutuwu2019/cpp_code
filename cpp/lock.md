# 锁

除了上面介绍的互斥锁，还有读写锁、自旋锁、公平锁、乐观锁-悲观锁


## 读写锁

读写锁，允许多个线程同时读取共享资源，但是在写入的时候只允许一个线程独占访问。适用于读多写少的场景，提高了并发性能

特点：
    允许多个读者同时反问
    写者独占访问，禁止其他读写操作

```c++
// 伪代码
pthread_rwlock rwlock;

void* reader(void* arg){

    pthread_rwlock_rdlock(&rwlock);

    // 读  共享资源


    pthread_rwlock_unlock(&rwlock);
    return NULL;
}
void write(void* arg){
    pthread_rwlock_wrlock(&rwlock);
    // 写 共享资源
    pthread_rwlock_unlock(&rwlock);

    return NULL:
}
```

## 自旋锁  skynet 项目采用的就是自旋锁处理的

自旋锁事轻量级的锁，线程在等待的时候会忙等待（不断检查锁的状态），而不是被阻塞。这种锁适用于锁持有时间短的场景，以减少上下文切换的开销


特点：

- 适合锁持有时间短的情困过
- 忙等待消耗cpu

```c++
//伪代码
pthread_spinlock_t spinlock;

void critical_section(){
    pthread_spin_lock(&spinlock);

    //自旋 区

    pthread_spin_unlock(&spinlock);

}
```

## 乐观锁&悲观锁


乐观锁：假设冲突发生的少，先执行操作再检查冲突。

悲观锁：假设冲突经常发生，先锁定资源再执行操作


特点：

乐观锁：高并发下性能较好
悲观锁：适用于高冲突场景


## 分布式锁

分布式锁用于分布式系统（貌似一句废话），确保在多个节点间维持资源访问互斥。

常见方式包裹zk、redis

---





