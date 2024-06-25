# Smart Point 智能指针

> 我想学习，总是断断续续的，刚开始晦涩，专有名词难懂，后面不断google、关键词查询，渐渐的知识树不断完善。

## 智能指针的发展历史
[智能指针的迭代历史](https://www.cnblogs.com/tp-16b/p/9033370.html)




## demo 介绍
1. tmp_0625_01 介绍了 auto_ptr 但是在我的环境跑不动，c++11已经弃用了，通过 get获取指针地址
2. tmp_0625_02 介绍了unique_ptr 注意啊 它的复制语义已经修改了，采用 std::move完成复制
3. tmp_0625_03 介绍了unique_ptr 数组，和裸指针是一样的使用、操作
4. tmp_0625_04 介绍了 unique_ptr 指向类对象，以及类的构造析构
5. tmp_0625_05 介绍了 unique_ptr 初始化 可以接入对象类型，以及decltype 函数(通过该函数对类资源的释放)
6. tmp_0625_06 介绍了 shared_ptr 可以继承 enable_shared_from_this 获取 自身shared_ptr
7. tmp_0625_07 介绍了 shared_ptr 循环引用的问题，shared_ptr<A> 对象调用方法，然后方法里面通过 shared_from_this 把shared_ptr<A>复制给类成员 shared_ptr，造成类A对象释放内存，会去调 类成员shared_ptr，而类成员shared_ptr又会去调用类对象本身，造成循环引用问题 “死锁"，解决方式就是把类成员 shared_ptr 指针类型转换成 weak_ptr
8. tmp_0625_08 介绍了 weak_ptr，它能够直接复制shared_ptr 你可以简单理解类类型转换。但是转回去就不容易了，要先验证是否声明周期结束了，然后在通过lock 获取 shared_ptr 对象
9. tmp_0625_09 介绍了 weak_ptr，它没有 operator*、operator->、operator! 通过 expire 确定 shared_ptr 是否生命周期结束，如果结束了就没办法进一步操作 shared_ptr对象，如果生命周期未结束，可以通过上锁 lock 对 shared_ptr 对象操作
10. tmp_0625_10 介绍了 shared_ptr 引用 已经释放资源的智能指针

## 总结

1. 既然使用了智能指针就不要再使用裸指针了
2. 确定哪些场所使用指针能指针。比如不需要共享的情景使用unique_ptr，如果需要管理资源生命周期使用shared_ptr，如果不需要管理生命周期的使用weak_ptr
3. 避免操作某个引用资源已经释放的智能指针。
4. 包含智能指针的类，因该前置声明。(减小编译依赖，加快编译速度和较小二进制文件大小)
