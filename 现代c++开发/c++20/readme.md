# 关于 c++ 20 新特性

1. ranges
> ranges = 算法 + 容器绑定 + 更少犯错

```cpp

// 模板 ranges::sort(v, {}, &T::field);

ranges::sort(meetings, {}, [](auto& a){return a[2];});

ranges::sort(meetings.begin(), meetings.end(),
            [](const auto& a, const auto&y b){
                return a[2] < b[2];
            });

```
