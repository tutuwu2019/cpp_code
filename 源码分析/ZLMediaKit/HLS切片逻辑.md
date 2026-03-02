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
