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


### leetcode 67 二进制两数求和

```cpp
string addBinary(string a, string b) {
        string ans;
        int i = a.size() - 1;
        int j = b.size() - 1;

        int carry = 0;

        while( i >= 0 || j >= 0 || carry > 0){
            int x = carry;
            if(i >= 0) x += a[i--] - '0';
            if(j >= 0) x += b[j--] - '0';
            
            ans.push_back(x % 2 + '0');
            carry = x / 2;
        }
        reverse(ans.begin(), ans.end());
        return ans;
    }

```

### 136 只出现一次的数字

> 需要知道亦或运算的基本性质


```cpp
int singleNumber(vector<int>& nums) {
        int ret = 0;
        // 亦或运算满足交换律、结合律以及自反
        // a ^ a = 0
        // 0 ^ a = a
        for (auto e: nums) ret ^= e;
        return ret;
    }
```

### 137 只出现一次的数字II

> 注意啊，通过这个算法，可以类推 n - 1 个数出现了 x 次，剩下的那个数只出现了1次

```cpp
int singleNumber(vector<int>& nums) {
        int ans = 0;;
        for(int i = 0; i < 32; i++){
            int x = 0;
            for(auto num : nums){
                x += (num >> i) & 1;      // 对于每个数字 在 Int 上的每一位 累加    每一位 通过  i 做基准 相加  因为  num >> i  获取的是  最低位位i 的值，比如 i = 2,然后 x = 10101  => 对应 的值为16+4+1=21 对应 >> 2 的值为 101  然后 i 位的值需要通过 & 1 运算的出结果， 101 & 001 = 1   
            }
            ans |= (x % 3) << i ;        // 这里是先通过位移得到该位的那个唯一数字的 值 然后通过 亦或得到该位的值   因为 亦或不会影响原有的值，很简单  如果原来这个位为 1， 那么 1 | x = 1  (如果x = 0)。  如果这个位原来为0  那么 0 | x  = 1  (如果 x = 1)
            
        }
        return ans;
    }

```


### leetcode 260 只出现一次的数字III

> 对于任意非零整数 x，x & (-x) 的结果是只保留 x 最低位的 1，其余位都变为 0。


```cpp
// 注意 xorAll 必须要要用 unsigned int 来表示，否则 在求  反码 的时候会由内存溢出
//  [1,1,0,-2147483648]  其中  -2147483648 对应的 unsigned int  为 2^32 + (-2147483648)  = 2^31 = 2147483648  如果用 int 表示就内存溢出了

vector<int> singleNumber(vector<int>& nums){
    unsigned int xorAll = 0;
    for(auto& num : nums){
      xorAll ^= num;    //求 a^b
    }
    // 根据 源码与反码 做 与运算的性质找到最低为为1 的位置
    unsigned int x = xorAll & (-xorAll);
    int a = 0, b = 0
    for(auto& num : nums){
      // a 在那个位置上为1
      if(num & x){
        a ^= num;
      }else{
        b ^= num;
      }
  }
  return {a, b};
}
```


### 389 找不同

```cpp
char findTheDifference(string s, string t) {
        char res = 0;       //  如果定义为 char res = ' '; 那么对应的 ASCII  值为32 ，对应求到的值是 'a' + 32 = 'A'   具体的 ' ' ^ 'a' = 'A'
        for(char c : s){
            res ^= c;
        }
        for(char c : t){
            res ^= c;
        }
        return res;
    }

```
