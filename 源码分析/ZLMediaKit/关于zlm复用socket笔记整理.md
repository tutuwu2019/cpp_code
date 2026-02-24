# 关于zlm复用socket笔记整理


## 为什么 zlm 要复用 socket 底层源码模块

ZLToolKit 提供了一套 fd + 所属 EventPooler + 线程安全  三位一体 的 socket 抽象，这套抽象是 ZLM 多协议、多线程架构的根基

具体的从6个维度展开
1. socketNum / socketFD 的分层设计：fd 生命周期与 epoll 生命周期解耦
```text
  int rawFd()         →  SockNum（只管 fd 的 close）
      +
  EventPoller::Ptr    →  SockFD（负责 poller->delEvent(fd)）
      │
      └─ 析构顺序保证：先 delEvent，再 close(fd)
         防止 close 后 fd 号被复用，epoll 监听到错误连接的数据
```

2. cloneSocket: webRTC 线程迁移的核心能力
```cpp
// webRtcSession 里 这段代码直接以来它

// 新 Socket，绑定到 transport 所在的 poller 线程
auto sock = Socket::createSocket(transport->getPoller(), false);
// 复制 fd（不 close），切换 epoll 监听归属
auto on_complete = sock->cloneSocket(*(getSock()));
```

- 从旧 socket 拿出 socketNum (共享同一个 rawFd，不 close)
- 在新 poller 线程上 attachEvent (在fd 重新加入新线程的 epoll)
- 返回一个 shared_ptr<void> 作为 RAII guard --- 析构时才真正开始监听，保证新的 session 创建完毕后再收包

> 这是一个精妙的 fd 所有权转移 原语，自己实现需要深刻理解 epoll 的ET/LT 触发模型以及跨线程 fd 迁移的时序问题，代价极高

3. EventPoller 绑定模型:每条连接固定在一条线程

```text
Socket ──── EventPoller（一个线程一个 epoll 实例）
               │
               ├─ addEvent(fd, Event_Read, cb)
               ├─ async(task)          // 跨线程投递任务
               └─ isCurrentThread()   // 判断是否已在本线程

```

ZLM 所有 session(RtspSession、webRtcSession、HttpSession...）都遵守同一个契约: session 的读写回调之灾其绑定的EventPoller 线程上执行，因此 session 内部数据结构无需加锁。这是整个服务器高性能的基础。
如果自己写这套 poller-per-thread 模型，相当于重写libuv/libevent的核心

4. 同一的发送缓冲区(两级缓冲+ 背压)
  flushData() 理实现了：
    - 一级缓冲 _send_buf_waiting: 业务线程写入
    - 二级缓冲 _send_buf_sending: poller 线程消费，走 writev/send
    - 背压: send 返回 EAGAIN-> startWriteAbleEvent -> Evnet_Write -> 等可写再继续
    - 发送速率统计: _send_speed
  
ZLM 所有协议(RTSP/RTMP/HLS/WebRTC) 的发送走这一套，不需要每个协议各自实现，保证了行为一致性和内存安全


5. UDPbindPeerAddr 软/硬绑定：webRTC 连接迁移支持

```text
// 软绑定：只记录目标地址，不调用内核 connect()
_udp_send_dst = std::make_shared<struct sockaddr_storage>();
memcpy(_udp_send_dst.get(), dst_addr, addr_len);

```


6. 跨平台抹平: epoll/kqueue/IOCP

```cpp
// Socket.cpp 的 accept 失败处理里：
#if (defined(HAS_EPOLL) && !defined(_WIN32)) || defined(HAS_KQUEUE)
    // 边缘触发：需要手动 100ms 后重试
    _poller->doDelayTask(100, [weak_self, sock]() { ... });
#else
    // 水平触发：sleep 10ms
    this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
```

ZLToolKit 把 linux epollET 、macOS kqueue、windows IOCP 的差异全部封装在 EventPooler 内部，ZLM 所有业务代码对此无感知。ZLM 要同时支持 linux/macOS/winddows/Android/ios，这个平台抽象价值巨大。



## 为什么不使用 libevnet？

表面上 libevent 和 ZLToolKit 都能抽象 epoll/kqueue,但ZLM 的核心设计需要三个libevent 根本不能提供的能力：
1. EventPooler 继承 AnyStorage --线程本地存储(TLS)绑定到poller

```cpp
class EventPoller : public TaskExecutor, public AnyStorage, ...
```

AnyStorage 允许在 poller 线程上存放任意业务对象，并且通过 EventPoller::getCurrentPoller() 从任意位置拿到当前线程的poller

ZLM 里 RingBUffer 的 _RingReaderDispatcher 就挂在 poller 上:
```cpp
// RingBuffer::attach()
auto &ref = _dispatcher_map[poller];   // key = EventPoller::Ptr
// Dispatcher 生命周期绑定到 poller 线程
auto onDealloc = [poller](RingReaderDispatcher *ptr) {
    poller->async([ptr]() { delete ptr; });  // 必须在 poller 线程析构
};

```
libevent 的event_base 是一个被动的 fd 分发器，没有 线程本地业务对象容器 的概念，做不到这种绑定

2. RIngBUgger 的多 Poller 多播架构 --libevnet 完全没有

这是 ZLM 最核心的设计，是流媒体服务器区别于普通网络库的关键:
```text
一个推流端（生产者）
    └─ RingBuffer::write(RtpPacket)
            │
            ├─ poller_A->async(...) → Dispatcher_A::write()
            │       └─ Reader_1->onRead()   (播放器1，绑定在线程A)
            │       └─ Reader_2->onRead()   (播放器2，绑定在线程A)
            │
            ├─ poller_B->async(...) → Dispatcher_B::write()
            │       └─ Reader_3->onRead()   (播放器3，绑定在线程B)
            │
            └─ _storage->write()            (GOP 缓存，供新播放器回放)
```

一帧数据，令拷贝广播给所有订阅线程。每个poller线程有自己的Dispathcer，读者在自己线程上消费，完全无锁。

libevent 的定位是 事件驱动的IO 库，它的职责是：有数据来了-> 调你的回调。它不管 「一份数据怎么广播给 N 个消费者，并且消费者分散在不同线程」 --这是流媒体服务器的核心业务逻辑，libevent 不可能内置这个。

3. doDelayTask 返回可取消的任务 + IO 循环统一调度
```cpp
DelayTask::Ptr doDelayTask(uint64_t delay_ms, std::function<uint64_t()> task);
// task 返回值非0 → 下次执行延迟（毫秒），实现自重复定时任务
// task 返回0 → 停止
// DelayTask::Ptr 析构 → 取消任务
```
ZLM 里大量用到了：
- session keepalive 超时检测
- accept 失败后100ms 重试
- webRTC ICE 超时

libevent 有 evtimer_*，但：
1. 它是C接口，不能返回 shared_ptr 管理生命周期
2. 没有「返回值控制下次延迟」的自重复机制
3. 定时器和IO回调不在同一个C++ 对象上，生命周期管理是噩梦

其他不可忽视的工程原因
| 维度	| libevent	 | ZLToolKit | 
|:---: | :---: | :---: |
| 语言	| C，需要大量 C++ wrapper	| 原生 C++17，shared_ptr/lambda 开箱即用 | 
| 线程模型	| event_base 不是线程安全的，多线程要用 bufferevent_pair 等绕路	| EventPoller 从设计上就是 one-loop-per-thread，async() 跨线程投递是一等公民 | 
| 共享读缓冲区	| 无	| getSharedBuffer(is_udp) —— 同一线程下所有 socket 共用一块接收缓冲，减少内存分配 | 
| cloneSocket	| 无（跨线程迁移 fd 需自己实现）	| 内置，WebRtcSession 线程迁移直接调用 | 
| 依赖体积	| libevent 本身不大，但拉入后还要解决 OpenSSL 集成、Windows 兼容等问题	| ZLToolKit 与 ZLM 同一作者，零配置| 
| GOP 缓存	| 无	| _RingStorage 内置 GOP 级别的缓存管理（IDR 帧分组、LRU 淘汰）| 


总结：libevent 解决的是「如何高效监听 fd 事件」，而ZLToolKit 在此基础上还解决了：
```text
EventPoller = epoll/kqueue 封装
    + one-loop-per-thread 线程绑定
    + 跨线程任务投递
    + 线程本地业务存储（AnyStorage）
    + 共享读缓冲区

RingBuffer = 媒体帧的无锁多播
    + GOP 关键帧缓存
    + per-poller Dispatcher（新订阅者自动回放）
    + 订阅者跨线程安全析构
```
后者是流媒体服务器的业务核心，不是任何通用网络库会提供的，自然也不是libevent 能替代的





