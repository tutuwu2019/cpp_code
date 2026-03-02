# HLS 切片逻辑

```text

_hls->inputFrame(frame)
  └─ HlsRecorder（TsMuxer 子类）处理帧
       └─ TsMuxer::inputFrame(frame)        ← 把 H264/AAC 封装成 TS 格式字节流
            └─ HlsMaker::inputData(data, len, timestamp, is_idr)

HlsMaker::inputData(data, len, ts, is_idr_fast_packet)
  ├─ 时间戳回退检测
  ├─ if (is_idr_fast_packet)
  │    └─ addNewSegment(timestamp)          ← 遇到关键帧，尝试切新分片
  └─ onWriteSegment(data, len)              ← 写 TS 数据到当前切片文件




addNewSegment(stamp)
  ├─ 检查距上次切片时间是否 >= seg_duration（默认10s）
  ├─ flushLastSegment(false)               ← 关闭上一个分片
  │    ├─ 计算 seg_dur
  │    ├─ onFlushLastSegment(seg_dur)      ← _file=nullptr，关闭 ts 文件（fclose）
  │    ├─ makeIndexFile(false, eof)        ← 生成/更新 hls.m3u8
  │    │    └─ onWriteHls(index_str, false)
  │    │         ├─ fwrite(...) → hls.m3u8 落盘
  │    │         └─ _media_src->setIndexFile(data)  ← 内存里也更新一份
  │    └─ [可选] makeIndexFile(true, eof)  ← 生成 _delay.m3u8（延迟版本）
  └─ onOpenSegment(_file_index++)          ← 打开新 ts 文件
       ├─ 按时间计算路径：YYYY-MM-DD/HH/MM-SS_index.ts
       ├─ makeFile(segment_path)           ← fopen 创建 .ts 文件
       └─ 返回 segment_name（写入 m3u8 用）


onWriteSegment(data, len)                   ← 每次 TS 封包后立即追加写文件
  ├─ fwrite(data, len, 1, _file.get())
  └─ _media_src->onSegmentSize(len)         ← 更新字节统计（bytesSpeed 等）

```


浏览器请求 m3u8 -> ZLM 返回

> 重点 NoticeCenter 语义

> kBroadcastHttpAccess 事件触发机制


```text
播放器 GET /live/test/hls.m3u8
  └─ HttpSession::onRecv(buf)
       └─ HttpRequestSplitter::input(...)
            └─ HttpSession 解析 URL

  └─ NoticeCenter emit kBroadcastHttpRequest
       └─ 未命中 s_map_api（非 /index/api/）
            └─ emit kBroadcastHttpAccess / HttpFileManager::process(...)
                 ├─ 鉴权（on_http_access hook，可选）
                 └─ 定位文件：www/live/test/hls.m3u8
                      └─ HttpSession 发送文件内容
                           └─ Socket::send(...)    ← 发回浏览器
```

> HlsMediaSource 还维护了内存版 m3u8（setIndexFile），部分实现下可以直接走内存返回，不用磁盘读：

```text
HlsMediaSource::setIndexFile(data)
  └─ 更新内存 m3u8 字符串
       └─ 后续 HTTP 请求命中时直接返回内存数据（避免磁盘 IO）

```
浏览器请求 .ts 分片

```text
播放器 GET /live/test/2026-03-02/03/10-00_5.ts
  └─ HttpSession → HttpFileManager::process(...)
       └─ 找到 ts 文件路径（已在磁盘）
            └─ Socket::send(文件数据流) → 浏览器
```


服务器关闭--清理HLS 文件
```text
ZLM 关闭 / 流断开
  └─ RtspSession 析构 → RtspMediaSourceImp 析构
       └─ MultiMediaSourceMuxer 析构
            └─ _hls.reset()               ← HlsRecorder 析构
                 └─ HlsMakerImp::~HlsMakerImp()
                      └─ clearCache(false, true)  ← eof=true
                           ├─ flushLastSegment(true)  ← 关闭最后一个 ts 文件
                           │    └─ makeIndexFile(..., eof=true)
                           │         └─ onWriteHls(...) 写含 #EXT-X-ENDLIST 的 m3u8
                           └─ [直播模式] 延迟删除 ts 文件 + m3u8
                                └─ _poller->doDelayTask(delay*1000, clearHls)

```
