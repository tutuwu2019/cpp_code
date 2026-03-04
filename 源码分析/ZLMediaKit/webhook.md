# webhook


> webhook 是一种基于 http 的反向回调机制。传统的 api 是 拉取 模式(主动请求服务器)，而webhook  是一种"推送模式"(事件发生时服务器主动通知你)
> > 比如你在网购下单，快递到达本地触发 快递小哥主动联系你，给你送快递


工作流
```text
事件发生  -> 服务端构造 HTTP POST 请求 -> 推送到预先配置的 URL(服务器) -> f服务器响应

```
> 工作流可以简单理解为，正常的 web 之上封装了一层事件触发机制

所以可以很快发现其特点：
1. 事件驱动，可以支持异步，不需要轮询
2. 实时，事件发生即通知
3. 轻量，通常是简单的http post + json body
4. 解耦：发送方和接收方单独部署(这也是异步的特点)


## zlm 中的 web hook

> 具体的，比如点击拉流，zlm 中就开始触发拉流播放

```shell

# 在 config.ini 中配置

[hook]
enable=1
on_publish=http://127.0.0.1:8080/index/hook/on_publish
on_play=http://127.0.0.1:8080/index/hook/on_play
on_stream_changed=http://127.0.0.1:8080/index/hook/on_stream_changed
on_stream_not_found=http://127.0.0.1:8080/index/hook/on_stream_not_found
on_record_mp4=http://127.0.0.1:8080/index/hook/on_record_mp4
on_shell_login=http://127.0.0.1:8080/index/hook/on_shell_login
on_flow_report=http://127.0.0.1:8080/index/hook/on_flow_report
on_http_access=http://127.0.0.1:8080/index/hook/on_http_access
on_server_started=http://127.0.0.1:8080/index/hook/on_server_started

```

主要的 hook 事件

1. on_publish  推流鉴权
推流端(视频源) 向 ZLM  推流时触发

```json
// ZLMediaKit → 你的服务器
POST /index/hook/on_publish
{
  "mediaServerId": "your_server_id",
  "app": "live",
  "stream": "test_stream",
  "ip": "192.168.1.100",
  "params": "token=abc123",   // 推流 URL 携带的参数
  "schema": "rtmp",
  "vhost": "__defaultVhost__"
}

```

```json
// 你的服务器 → ZLMediaKit（允许推流）
{ "code": 0, "msg": "success" }

// 拒绝推流
{ "code": -1, "msg": "invalid token" }
```

2. on_play 播放鉴权

用户拉流播放时触发


```json
{
  "app": "live",
  "stream": "test_stream",
  "ip": "10.0.0.5",
  "params": "sign=xyz",
  "schema": "http",   // http-flv / hls / rtsp 等
  "vhost": "__defaultVhost__"
}


```

3. on_streawm_changed 流状态变化

流注册(上线)、注销(下线)时触发

```json
{
  "app": "live",
  "stream": "test_stream",
  "regist": true,    // true=上线，false=下线
  "schema": "rtmp",
  "vhost": "__defaultVhost__",
  "tracks": [
    { "codec_id": 0, "codec_type": 0 },  // 视频轨道
    { "codec_id": 2, "codec_type": 1 }   // 音频轨道
  ]
}

```

4. on_streawm_not_found 流不存在

 拉流时发现流不存在触发，用于按需拉流(懒加载)，收到通知后去源站拉流再转推
```json
{
  "app": "live",
  "stream": "camera_001",
  "ip": "10.0.0.5",
  "schema": "http"
}

```

5. on_record_mp4 录制完成

mp4 文件录制完成后触发，可做归档、转码、上传OS等处理

```json
{
  "app": "live",
  "stream": "test_stream",
  "file_path": "/opt/media/record/live/test_stream/2024-01-01/10-00-00.mp4",
  "file_size": 10485760,
  "time_len": 60.5,
  "start_time": 1704067200,
  "url": "record/live/test_stream/2024-01-01/10-00-00.mp4"
}
```

6. on_flow_report 流量统计

断流开时上报该路流的流量消耗，用于计费

```json

{
  "app": "live",
  "stream": "test_stream",
  "ip": "10.0.0.5",
  "duration": 3600,         // 播放秒数
  "total_bytes": 524288000, // 总字节数
  "is_player": true         // true=播放端，false=推流端
}

```

7. on_server_started 服务启动


ZLMediaKit 服务启动时触发，携带完整的当前配置信息，可做服务发现或配置同步。
