# ZLMediaKit 源码分析 note


## 网络层

### 常见端口

端口 | 协议 | 默认监听 | 主要用途 | 是否默认开启 | 备注 / 常见修改场景
:--: | :--: | :--: | :--:     | :--:       | :--: 
554 | TCP (RTSP) | 是 | RTSP 服务器（推流 / 拉流） | 是 | RTSP 标准端口，几乎所有 RTSP 客户端/摄像头默认使用 554
1935 | TCP (RTMP) | 是 | RTMP 推流 / 拉流（最经典的直播推流端口） | 是 | Adobe RTMP 标准端口，推流端（如 OBS、ffmpeg）最常用
80 | TCP (HTTP) | 是 | HTTP API + HTTP-FLV + HLS(m3u8) + Web管理页面 | 是 | 明文 HTTP 服务端口，绝大多数 Web 播放器默认走 80
443 | TCP (HTTPS) | 否 | HTTPS API + 加密 HLS/HTTP-FLV/WebRTC signaling | 通常手动开 | 生产环境强烈建议开启，需配置证书；很多 Docker 镜像映射 8443→443
10000 | TCP/UDP | 是 | RTP over TCP/UDP（主要是 WebRTC + 部分 RTSP） | 是 | WebRTC 默认音视频 RTP 端口范围起点（通常 10000-65535 范围动态分配）
9000 | UDP | 是 | SRT 协议推拉流 | 是 | SRT 默认监听端口（从 v4.x 开始默认开启）
19355 | TCP | 否 | 极少见，通常是自定义 RTMP 端口或误写 | 基本不开 | 官方默认无此端口，可能是用户改了 1935→19355 或某些旧教程/脚本遗留的写法

### 网络协议

1. tcp

```cpp
/**
    * @brief 开始tcp server
    * @param port 本机端口，0则随机
    * @param host 监听网卡ip
    * @param backlog tcp listen backlog
     * @brief Starts the TCP server
     * @param port Local port, 0 for random
     * @param host Listening network card IP
     * @param backlog TCP listen backlog
     
     * [AUTO-TRANSLATED:9bab69b6]
    */
    template <typename SessionType>
    void start(uint16_t port, const std::string &host = "::", uint32_t backlog = 1024, const std::function<void(std::shared_ptr<SessionType> &)> &cb = nullptr) {
        static std::string cls_name = toolkit::demangle(typeid(SessionType).name());
        // Session创建器，通过它创建不同类型的服务器  [AUTO-TRANSLATED:f5585e1e]
        //Session creator, creates different types of servers through it
        _session_alloc = [cb](const TcpServer::Ptr &server, const Socket::Ptr &sock) {
            auto session = std::shared_ptr<SessionType>(new SessionType(sock), [](SessionType *ptr) {
                TraceP(static_cast<Session *>(ptr)) << "~" << cls_name;
                delete ptr;
            });
            if (cb) {
                cb(session);
            }
            TraceP(static_cast<Session *>(session.get())) << cls_name;
            session->setOnCreateSocket(server->_on_create_socket);
            return std::make_shared<SessionHelper>(server, std::move(session), cls_name);
        };
        start_l(port, host, backlog);
    }

```


## 事件机制

emitEvent
```cpp
// src/Common/MediaSource.cpp:495
void MediaSource::emitEvent(bool regist) {
    // 1. 通知局部 listener（如 MultiMediaSourceMuxer）
    auto listener = _listener.lock();
    if (listener) {
        listener->onRegist(*this, regist);
    }
    // 2. 全局广播到 NoticeCenter 总线
    NOTICE_EMIT(BroadcastMediaChangedArgs, Broadcast::kBroadcastMediaChanged, regist, *this);
    InfoL << (regist ? "媒体注册:" : "媒体注销:") << getUrl();
}
```
<img width="826" height="226" alt="image" src="https://github.com/user-attachments/assets/7c6409ab-4e1b-4e8d-a2b9-09a72ce5b25e" />

```text
MediaSource::regist()   → emitEvent(true)
MediaSource::unregist() → emitEvent(false)
         │
         ├─ ① listener->onRegist(*this, regist)
         │       └── _listener 是 weak_ptr<MediaSourceEvent>
         │           → 例如 MultiMediaSourceMuxer（HLS/RTMP/TS 转封装器）
         │               收到注册通知后，关联或释放复用资源
         │
         └─ ② NOTICE_EMIT(kBroadcastMediaChanged, regist, sender)
                 └── NoticeCenter（全局发布/订阅总线）
                     将事件广播给所有订阅者

## 关键场景---播放器等流
播放器请求 rtsp://127.0.0.1/live/stream（流不存在）
    │
    ├─ findAsync_l() 找不到流
    ├─ addListener(kBroadcastMediaChanged, on_register)  ← 挂起等待
    └─ NOTICE_EMIT(kBroadcastNotFoundStream, ...)        ← 触发按需拉流

推流端推流进来
    │
    └─ MediaSource::regist() → emitEvent(true)
            └─ NoticeCenter 广播 → on_register 被唤醒
                    └─ 切回播放器所在线程 → findAsync_l() 再找一次 → 回复播放器 ✅


onRecv_L

toolkit::Session         HttpRequestSplitter
      │                          │
      └──────── WebRtcSession ───┘
                     │
                 onRecv()          onRecvHeader()
                 (override)        (override)
                     │                  ↑
                     └──→ onRecv_l() ───┘ (被 Splitter 回调)



### udp 模式（直接模式）
[内核 epoll/kqueue]
    └─ Socket::setOnRead() 回调触发
        └─ Session::onRecv(Buffer::Ptr)          ← ZLToolKit 框架调用
            └─ WebRtcSession::onRecv()
                if (!_over_tcp):
                    └─ onRecv_l(data, len)        ← 直接进入业务逻辑
                        ├─ getUserName() → 找 WebRtcTransport
                        ├─ 如果 transport 在别的线程 → cloneSocket + async → 新 WebRtcSession
                        └─ _transport->inputSockData(data, len, self)


### tcp 模式（经过 Splitter 的间接路径）
[内核 epoll/kqueue]
    └─ Socket::setOnRead() 回调触发
        └─ Session::onRecv(Buffer::Ptr)          ← ZLToolKit 框架调用
            └─ WebRtcSession::onRecv()
                if (_over_tcp):
                    └─ HttpRequestSplitter::input(data, len)   ← 进入拆包状态机
                        │
                        │  【while 循环】
                        ├─ onSearchPacketTail(ptr, len)        ← WebRtcSession 实现
                        │       读前2字节大端 uint16 = length
                        │       检查剩余数据 >= length+2
                        │       返回 ptr + 2 + length （包尾指针）
                        │
                        │  找到完整帧 → 触发:
                        └─ onRecvHeader(header_ptr, header_size)  ← WebRtcSession 实现
                                剥掉2字节长度头:
                                onRecv_l(data + 2, len - 2)   ← 进入业务逻辑（同上）
                                return 0  ← 告诉 Splitter "后面还是 header 模式"

HttpRequestSplitter::input() 状态机解析
_content_len == 0  →  "请求头模式"（Header Mode）
                       循环调用 onSearchPacketTail() 定位包尾
                       找到包尾 → 调用 onRecvHeader()
                       onRecvHeader 返回值含义：
                         == 0  → 继续 Header 模式（WebRtcSession 就返回 0）
                         > 0   → 切换到固定长度 Content 模式，等 N 字节后调 onRecvContent()
                         < 0   → Content 模式，后续所有数据全部分段 onRecvContent()

```

```text
网络层          TcpServer/Socket       WebRtcSession          HttpRequestSplitter      WebRtcTransport
  │                    │                    │                         │                      │
  │──epoll 事件───────→│                    │                         │                      │
  │              setOnRead cb              │                         │                      │
  │                    │──onRecv(buf)──────→│                         │                      │
  │                    │               [_over_tcp?]                   │                      │
  │                    │           yes │          no                  │                      │
  │                    │               │──input()──→│                 │                      │
  │                    │               │            │─onSearchPacketTail()                   │
  │                    │               │            │  找2字节帧边界   │                      │
  │                    │               │            │─onRecvHeader()──→│                     │
  │                    │               │            │                 │                      │
  │                    │               │←─────────────────────────── │                      │
  │                    │               │onRecv_l(payload)            │                      │
  │                    │               │[同线程? 直接调用 : 迁移线程]  │                      │
  │                    │               │──────────────────────────────────inputSockData()──→│
  │                    │               │                             │                      │

```

| 问题	| 设计决策 | 
|:---: | :---:|
|为什么 UDP 不用 Splitter？	| UDP 本身是数据报，每次 recvfrom 就是完整一帧，无需拆包 | 
| 为什么 TCP 要用 Splitter？	| TCP 是字节流，需要"2字节长度前缀"定界，Splitter 封装了这个拆包状态机 | 
| onRecvHeader 为何返回 0？	| WebRTC 帧是定长前缀协议，不需要 Content 模式，每帧即是一个独立"header" | 
| _find_transport 只允许一次？	| 第一帧 STUN BindingRequest 才含 username，一旦找到 transport 就绑定，后续帧无需再查找 | 
| 线程迁移为何用 throw？	| 抛异常是销毁旧 Session 的最简洁方式：沿调用栈展开，让 shared_ptr 析构接管清理 | 

---


```text

using onReadCB = std::function<void(Buffer::Ptr &buf, struct sockaddr *addr, int addr_len)>;



```



为什么 ZLM 要复用 Socket 底层源码模块？
这个问题的本质是：为什么 ZLMediaKit 不自己写 socket，而要依赖 ZLToolKit 这个子模块？

答案是一句话：ZLToolKit 提供了一套「fd + 所属 EventPoller + 线程安全」三位一体的 Socket 抽象，这套抽象是 ZLM 多协议、多线程架构的根基，自己从头造的成本远超收益。

1. SockNum / SockFD 的分层设计：fd 生命周期与 epoll 生命周期解耦
```text
 int rawFd()         →  SockNum（只管 fd 的 close）
      +
  EventPoller::Ptr    →  SockFD（负责 poller->delEvent(fd)）
      │
      └─ 析构顺序保证：先 delEvent，再 close(fd)
         防止 close 后 fd 号被复用，epoll 监听到错误连接的数据
```
2. cloneSocket：WebRTC 线程迁移的核心能力
```text
WebRtcSession 里这段代码直接依赖它：
// 新 Socket，绑定到 transport 所在的 poller 线程
auto sock = Socket::createSocket(transport->getPoller(), false);
// 复制 fd（不 close），切换 epoll 监听归属
auto on_complete = sock->cloneSocket(*(getSock()));

cloneSocket 的实现是：

从旧 Socket 拿出 SockNum（共享同一个 rawFd，不 close）
在新 poller 线程上 attachEvent（把 fd 重新加入新线程的 epoll）
返回一个 shared_ptr<void> 作为 RAII guard —— 析构时才真正开始监听，保证新 Session 创建完毕后再收包
这是一个精妙的「fd 所有权转移」原语，自己实现需要深刻理解 epoll 的 ET/LT 触发模型以及跨线程 fd 迁移的时序问题，代价极高。

```

3. EventPoller 绑定模型：每条连接固定在一条线程
```text

Socket ──── EventPoller（一个线程一个 epoll 实例）
               │
               ├─ addEvent(fd, Event_Read, cb)
               ├─ async(task)          // 跨线程投递任务
               └─ isCurrentThread()   // 判断是否已在本线程
```

ZLM 所有 Session（RtspSession、WebRtcSession、HttpSession…）都遵守同一个契约：Session 的读/写回调只在其绑定的 EventPoller 线程上执行，因此 Session 内部数据结构无需加锁。这是整个服务器高性能的基础。如果自己实现这套 poller-per-thread 模型，相当于重写 libuv/libevent 的核心。

4. 统一的发送缓冲区（两级缓冲 + 背压）

flushData() 里实现了：

一级缓冲 _send_buf_waiting：业务线程写入
二级缓冲 _send_buf_sending：poller 线程消费，走 writev/send
背压：send 返回 EAGAIN → startWriteAbleEvent → 监听 Event_Write → 等可写再继续
发送速率统计：_send_speed
ZLM 所有协议（RTSP/RTMP/HLS/WebRTC）的发送都走这一套，不需要每个协议各自实现，保证了行为一致性和内存安全。

5. UDP bindPeerAddr 软/硬绑定：WebRTC 连接迁移支持
```text
// 软绑定：只记录目标地址，不调用内核 connect()
_udp_send_dst = std::make_shared<struct sockaddr_storage>();
memcpy(_udp_send_dst.get(), dst_addr, addr_len);
```
WebRTC 的 ICE 连接迁移（客户端切换网络时源地址变更）需要 UDP socket 在不重建 fd 的前提下改变发送目标。软绑定就是这个能力的基础设施，ZLM 直接复用，零实现代价。

6. 跨平台抹平：epoll / kqueue / IOCP
```text
// Socket.cpp 的 accept 失败处理里：
#if (defined(HAS_EPOLL) && !defined(_WIN32)) || defined(HAS_KQUEUE)
    // 边缘触发：需要手动 100ms 后重试
    _poller->doDelayTask(100, [weak_self, sock]() { ... });
#else
    // 水平触发：sleep 10ms
    this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
```

ZLToolKit 把 Linux epoll ET、macOS kqueue、Windows IOCP 的差异全部封装在 EventPoller 内部，ZLM 所有业务代码对此无感知。ZLM 要同时支持 Linux/macOS/Windows/Android/iOS，这个平台抽象价值巨大。

总结
| 能力	| 如果 ZLM 自己造 | 
|:--: | :--: |
| fd 生命周期管理	| 极易出现 close 后 fd 复用、epoll 残留事件 bug | 
| 线程迁移（cloneSocket）	| 需要深入理解 ET 触发时序，实现难度极高 | 
| 两级发送缓冲+背压	| 每个协议重复实现，代码量巨大且行为不一致 | 
| epoll/kqueue/IOCP 抽象	| 相当于重写 libuv，不现实 | 
| poller-per-thread 模型	| Session 内部需要大量加锁，性能倒退 | 


### 偶合

webrtc、session、poller以及应用协议层(包括ICE、DTLS、SRTP)


- ZLM中的每个poller 代表一个事件循环线程，处理网络IO和定时器
- webRtcTransport 是一个处理webRTC 传输层的对象 (包括ICE协商、DTLS握手、SRTP加密、数据收发)，它与某个poller 绑定，其所有的操作都应该在poller线程中执行，以避免锁竞争和线程安全问题
- webRtcSession 代表一个webrtc 会话，它管理 socket，并持有 webRtcTransport 指针
- 为什么需要切换线程判断？因为当第一个数据包到达时，webRtcSession 可能位于监听线程(如主线程或接收连接的线程),而对应的webRtcTransport 可能位于另一个工作线程(根据负载分配或其他策略)。如果直接在当前线程处理数据，会违反线程关联性，可能导致数据竞争、锁竞争、活着某些非线程安全的操作。所以需要将后续处理切换到webrtcTracsport 所在的线程
- 切换过程：通过克隆 socket 到目标线程，并在目标线程创建新的webRtcSession，重新处理数据，然后销毁原对象，确保所有后续操作都在正确线程执行
- 这种设计充分利用多核CPU,同时保持单线程编程的简洁性，避免复杂的锁

> poller 是事件驱动的核心抽象，每个poller 对象运行在一个独立的线程中，负责管理该线程上的所有套接字的IO 事件和定时器任务。webRtcTracsport 和 webRtcSession 都与特定的Poller 线程绑定，他们的成员函数只能绑定线程上使用，否则可能导致数据竞争和为定义行为。这种单线程绑定的设计简化了并发控制，避免使用锁，提高了性能

线程匹配
1. 对象线程关联性
   - webRtcTransport 在创建时被分配到一个poller 线程，该线程负责处理它的所有逻辑
   - webRtcTransport 同样绑定到一个poller 线程，它负责底层 socket 事件监听和数据接收
   - 这两个对象可能不在同一个线程(例如：webRtcTransport 可能根据负载均衡被分配到某个工作线程，而初始的tcp 连接可能由监听线程接收)
  
2. 数据处理的线程安全性
   - 如果webRtcSession 在接收数据后直接调用 webRtcTransport 的方法，而后者位于另一个线程，就会发生垮线程调用，而破坏线程关联性
   - 这可能导致 webRtcSport 内部状态被并发修改(例如同时处理网络数据和超时定时器)，引发崩溃或数据损坏
   - 虽然可以用锁保护，但多线程加锁会降低性能并增加复杂性，ZLM 更倾向于无锁设计，每个对象都只在一个线程上操作
  
3. 性能考量
   - 将webRtcTransport 独立到一个线程，可以让多个会话在不同的CPU核心上并行处理，提高吞吐量
   - 数据从socket 读入后，立即切换到目标 poller 线程，后续所有协议栈处理都在同一线程完成，避免了线程切换和锁开销

线程切换判断的时机与作用

在webRtcSession::onRecv_l 中，当第一次收到数据(_find_transport == true)时，需要根据数据中用户名找到对应的 webRtcTransport。此时必须检查 transport->getPoller()->isCurrentThread():
   - 如果相同：直接绑定 _transport，后续数据在当前线程处理即可  
   - 如果不相同：说明 webRtcSession 和 webRtcTransport 不在同一线程，不能直接在当前线程使用 transport 
   此时需要通过一下步骤切换：
      1. 克隆当前 socket 到目标 poller (使用 cloneSocket, fd 不变但事件监听由新线程负责)
      2. 在目标线程异步创建一个新的webRtcSession，将克隆的 socket 交给它
      3. 在新 session 中重新调用 onRecv_l，处理同一份数据，从而将整个会话迁移到正确的线程
      4. 通过抛弃异常销毁原 webRtcSession 和旧 socket，完成线程切换
    
> 这样保证了所有后续数据都在 webRtcTransport 所在的线程处理，维持了单线程的简洁性，并充分利用多核能力
