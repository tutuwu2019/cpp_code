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


```
