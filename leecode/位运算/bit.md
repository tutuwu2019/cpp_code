# 位运算

###  leetcode 1009 十进制整数的反码

```cpp

  int bitwiseComplement(int n){
    if(n == 0){
      return 1;
    }
  // 找到掩码，位数与n 一样，且所有位都是1
  unsigned int mask = 1;
  while(mask < n){
    // 或运算就是  有1即为1
    // 不断地由 1 10 100 1000
    //         1 11 111  ...
    max = ( maxk << 1) | 1;
  }
// 最后亦或 亦或的意思是，相同为0不同为1
  return mask ^ n;
```
