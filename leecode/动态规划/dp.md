# dp

leetcode 3129 找出所有稳定的二进制数组I

> todo
```cpp
for(int i = 0; i <= min(zero, limit); i++) dp[i][0][0] = 1;
        for(int j = 0; j <= min(one, limit); j++) dp[0][j][1] = 1;
```

```cpp
int numberOfStableArrays(int zero, int one, int limit) {
        //arr 中每个长度超过 limit 的 子数组 都 同时 包含 0 和 1  这句话的涵义是， 有 limit 个 1 以后第limit + 1 必须是0

        long long mod = 1e9 + 7;
        // dp[i][j][0] 表示用i 个0,j个1，且最后一位是0
        // dp[i][j][1] 表示 用了 i 个0，j个1，且最后一个是1

        vector<vector<vector<long long>>> dp(zero + 1, vector<vector<long long >>(one + 1, vector<long long>(2, 0)));

        // 初始化 只放一种数字且不超过limit 的情况
        for(int i = 0; i <= min(zero, limit); i++) dp[i][0][0] = 1;
        for(int j = 0; j <= min(one, limit); j++) dp[0][j][1] = 1;

        for(int i = 1; i <= zero; i++){
            for(int j = 1; j <= one; j++){
                // 转移最后一位是0的情况
                dp[i][j][0] = (dp[i - 1][j][0] + dp[i - 1][j][1]) % mod;

                if(i > limit){
                    dp[i][j][0] = (dp[i][j][0] - dp[i - limit - 1][j][1] + mod) % mod;
                }

                // 转移最后一位是1的情况
                dp[i][j][1] = (dp[i][j - 1][0] + dp[i][j - 1][1]) % mod;
                if(j > limit){
                    dp[i][j][1] = (dp[i][j][1] - dp[i][j - limit - 1][0] + mod) % mod;
                }
            }
        }
        return (dp[zero][one][0] + dp[zero][one][1]) % mod;
    }

```
