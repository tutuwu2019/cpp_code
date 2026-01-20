# 线程安全 api


时间处理类

|线程不安全API | 线程安全替代方案|
|:--: | :--:|
| std::localtime(返回指向静态内存的指针；而安全版本将结果写入用户提供的缓冲区) | localtime_c(linx_)/localtime_s(win)|
|std::gmtime | gtime_r(linx)/gtime_s(win)|
|std::ctime(ctime 内部调用了 localtime,同样使用了静态缓冲区)| asctime_r 或 std::put_time|
|std::asctime|asctime_r|

" localtime 与 localtime_r "

```cpp
auto now = std::chrono::system_clock::now();

std::time_t now_c = std::chrono::system_clock::to_time(now);
std::tm noe_tm = std::localtime(&now_c);

///////////////////////////////////////////////////////

std::tm now_tm;
localtime_r(&now_tm, &now_c);

```

字符串与格式化类


