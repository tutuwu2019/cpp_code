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


