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


3. 资源绑定: session 绑定 MediaSource

RtspSession 会把连接和媒体资源“绑定起来"
  - 推流时: 创建 RtspMediaSourceImp -> _push_src
  - 拉流时: 绑定 RtspMediaSOurce -> _play_src、_play_reader
  - MediaSOurceEvent 让 session 可以收到媒体事件(关闭你/人数统计等)

好处：
  - session 生命周期 = 连接声明周期
  - 媒体源生命周期可独立延迟回收(_continue_push_ms)


4. 状态驱动 + 超时管理

RtspSession 用 _alive_ticker + _sessionid 控制状态

  - onManager() 周期检测：握手超时/推流超市/播放超时
  - _rtp_type 记录传输方式(UDP/TCP/HTTP)

session 自带“状态机”，不依赖外部调度

5. 高可用: 统一的 session 基类
session 只负责通用能力：
  - 连接读写
  - attachServer
  - 基础统计

结论
ZLMediaKit 的 session 设计核心是"一个连接 = 一个协议组合对象"，通过继承吧协议栈拼装起来，并将媒体资源绑定到连接生命周期中。
