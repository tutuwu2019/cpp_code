# session 会话思想

> ZLMediaKit 中的 session 设计思想，采用“分层职责 + 状态驱动、资源绑定“逻辑实现

1. session 是“连接的协议容器”
ZLMediaKit 中，session 表示一次 TCP 连接的业务具体实现：
  - 底层由TcpServer 创建
  - 每个连接绑定一个 session 实例
  - session 负责协议解析 + 业务流程 + 资源生命周期

RtspSession 举例：
(协议栈组合)
  - 继承 toolkit::Session 连接声明周期
  - 继承 RtspSplitter RTSP 协议解析
  - 继承 RtspReceiver RTP 乱序/排序
  - 基层 MediaSOurceEvent 与媒体源事件交互

2. 分层职责：控制流 vs 数据流

控制流(RTSP 指令)
  - onWholeRtspPacket() 解析请求方法
  - handleReq_Options/Describe/Setup/PlayRecord
  - 认证相关: onAuthBasic/onAuthDigest

数据流(RTP/RTCP)
  - onRtpPacket()/onRtcpPacket()
  - onRtpSOrted() 写入媒体源
  - sendRtpPacket() 从 RtspMediaSOurce 发给客户端

设计思想：控制面(RTSP) 和数据面(RTP) 解耦，但在同一个 session 内协同


3. 资源绑定: session 绑定 MediaSOurce

RtspSession 会把连接和媒体资源“绑定起来"
  - 推流时: 创建 RtspMediaSourceImp -> _push_src
  - 拉流时: 绑定 RtspMediaSOurce -> _play_src、_play_reader
  - MediaSOurceEvent 让 session 可以收到媒体事件(
