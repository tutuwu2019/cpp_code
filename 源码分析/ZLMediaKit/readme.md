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

```
