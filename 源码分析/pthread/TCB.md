
# about TCB todo


1. TCB 的本体：为什么放在 struct pthread 最前面

2. header 里面放了什么：脂肪高频、ABI敏感字段

3. TCB与TLS 的一体化，不是并列关系，而是同一寻址体系

4. TCB 初始化策略：冷热分离，减少重复清零

5. TCB 字段与内核契约：把clone/exit 协议固化到结构里

