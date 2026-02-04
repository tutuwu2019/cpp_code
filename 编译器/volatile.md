# volatile 

> volatile tells the compiler not to optimize anything that has to do with the volatile variable.

可以简单理解为， volatile 是在编译器层面禁用涉及 volatile 变量的代码。

使用情况有三
- When you interface with hardware that changes the value itself
- when there's another thread running that also uses the variable
- when there's a signal handler that might change the value of the variable.
