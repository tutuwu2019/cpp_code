# 线程安全 api

> 其实这里面涉及一个很有意思的东西，就是 多线程技术的发展晚于c/c++ 所以会有一些 std 原生 api 在设计之初没有支持 对线程场景

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

| 线程不安全 api | 线程安全 替代方案 |
|:--:|:--:|
| strtok (使用静态全局指针记录分割位置，多线程同时分割字符串会发生崩溃)| strtok_r(linx)/strtok_s(win) |
|strerror (获取错误码描述，不安全版本返回静态指针)| strrerror_r |
|sprintf(虽然 sprintf 本身不涉及静态缓冲区，但它不检查长度，容易导致缓冲区溢出，间接影响线程安全)|snprintf|


随机数生成类

| 线程不安全 api | 线程安全 替代方案 |
|:--:|:--:|
|std::rand (内部维护一个全局种子状态) | std::mt19937(每个线程独立的生成器) |
| std::srand (修改全局种子) | std::random_device | 


信号处理与环境类

这些 api 操作的是进程级别的全局资源

|  函数名 | 不安全原因 |
|:--:|:--:|
| std::getent | 返回指向环境列表的指针，若其他线程修改环境则失效|
|std::putenv/ setenv | 修改全局环境列表 |
| std::signal | 设置全局信号量处理函数 |
