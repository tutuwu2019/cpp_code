# 线程安全 api


时间类

" localtime 与 localtime_r "

```cpp
auto now = std::chrono::system_clock::now();

std::time_t now_c = std::chrono::system_clock::to_time(now);
std::tm noe_tm = std::localtime(&now_c);

///////////////////////////////////////////////////////

std::tm now_tm;
localtime_r(&now_tm, &now_c);

```
