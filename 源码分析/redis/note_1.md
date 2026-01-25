# note_1

> 老实说，跟着源码敲一遍，真的可以学些到很多东西，很多有意思的 tips

## 数据结构
1. 字典
src/dict.h
```cpp
// 字典哈希 func

static unsigned int dictGenHashFunction(const unsigned char* buf, int len){
  unsigned int hash = 5381;

while(len--)
  hash = ((hash << 5) + hash) + (*buf++);    /* hash * 33 + c      注意， (*buf++)  这里 *buf  先加上，然后 buf 再往后偏移一位*/
return hash


```

2. 跳表
src/server.h


3. 简单动态字符串
src/sds.h

4. 双链表
src/adlist.h


5. 整数集合
src/intset.h


6. 压缩列表
src/ziplist.h


7. 快速列表
src/quicklist.h


8. 基数树
src/rax.h


9. 流
src/stream.h

10. 地图索引
src/geo.h
