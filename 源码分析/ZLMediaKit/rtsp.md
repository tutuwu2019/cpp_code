# rtsp


```mermaid

sequenceDiagram
    autonumber
    participant Client as RTSP客户端
    participant Server as ZLMediaKit(RtspSession)
    participant Media as RtspMediaSource

    rect rgb(245, 245, 245)
    note over Client,Server: 播放流程（Pull）
    Client->>Server: OPTIONS
    Server-->>Client: 200 OK (Public)

    Client->>Server: DESCRIBE rtsp://host/app/stream
    Server->>Media: 查找/创建 MediaSource
    Server-->>Client: 200 OK + SDP

    Client->>Server: SETUP (Transport)
    Server-->>Client: 200 OK (Session/Transport)

    Client->>Server: PLAY
    Server-->>Client: 200 OK
    Server->>Media: 订阅RTP Ring
    Media-->>Client: RTP/RTCP（UDP或TCP interleaved）
    end

    rect rgb(240, 248, 255)
    note over Client,Server: 推流流程（Publish）
    Client->>Server: OPTIONS
    Server-->>Client: 200 OK

    Client->>Server: ANNOUNCE + SDP
    Server->>Media: 创建 RtspMediaSourceImp
    Server-->>Client: 200 OK

    Client->>Server: SETUP (Transport)
    Server-->>Client: 200 OK

    Client->>Server: RECORD
    Server-->>Client: 200 OK

    Client-->>Server: RTP/RTCP（UDP或TCP interleaved）
    Server->>Media: onWrite(RTP) 写入
    Media-->>Media: 解封装/生成Track/多协议复用
    end


    ```
