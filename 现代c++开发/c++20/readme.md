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


//c++20 支持模板化this
operator()<Self&& self, ...)

//引入显示对象参数语法
（this auto&& self, ...)

//this 参与模板推导
self(self, x);

```

> 为什么之前的 Y-combinator 很丑 ? 
