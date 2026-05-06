# SharedMutex.h 源码解读

## 源码结构


关键词：SharedMutexToken、SharedMutexImpl、SharedMutexPolicyDefault、SharedMutexImpl 锁的颗粒度(读锁、升级锁、写锁)、
32-bit的state_、slot_、doWait、WaitContext、公共接口



```cpp
struct SharedMutexToken {
  enum class State : uint16_t {
    Invalid = 0,
    LockedShared, // May be inline or deferred.
    LockedInlineShared,
    LockedDeferredShared,
  };

  State state_{};
  uint16_t slot_{};

  constexpr SharedMutexToken() = default;

  explicit operator bool() const { return state_ != State::Invalid; }
};
```

SharedMutex 保存引出来 SharedMutexToen 
> 
