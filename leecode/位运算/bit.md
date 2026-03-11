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

### leetcode 29 两数相除

```cpp
int divide(int dividend, int divisor) {
        // 边界处理
        
        if(dividend == INT_MIN && divisor == -1){
            return INT_MAX;
        }
        

        //  int 中最小的数  -2^32=-2147483648  如果直接用 abs((-2147483648)-2147483648) 会抛出内存溢出
        //  int 中最大的数  2^32-1=2147483647
        //  所以牢记，负数取反，一定要确定原有的类型能够包住新的值
        /*
            其实这个算法，采用的是多项式拆解的思路
        */
        bool flag = (dividend > 0) ^ (divisor > 0);
        long long a = abs((long long)dividend);
        long long b = abs((long long )divisor);
        long long ans = 0;
        while( a >= b){
            long long tmp = b, mul = 1;
            while( a >= (tmp << 1)){
                tmp = tmp << 1;
                mul = mul << 1;
            }
            a -= tmp;
            ans += mul;
        }
        return flag ? (-ans) : ans;
    }
```
