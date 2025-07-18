#include <iostream>
#include <math.h>
#include <vector>

class Solution {
public:
    // 埃氏筛法：从2(因为已经知道2是质数)开始，逐个将每个质数的背书标记为非质数，最终留下的没有被标记的数就是质数。  O(nloglogn)
    // 说明：  0和1不是质数，其余暂定为质数

    int countPrimes2(int n){
        if(n < 2){
            return false;
        }
        
        std::vector<bool> flag(n, true);
        
        flag[0] = flag[1] = false;
        
        for(int i = 2; i * i < n; i++){
            if(flag[i]){
                for(int j = i * i; j < n; j += i){
                    flag[j] = false;
                }
            }
        }
        return count(flag.begin(), flag.end(), true);
    }
    
    bool func(int x){
        if(x % 2 == 0){
            if(x == 2){
                return true;
            }else{
                return false;
            }
        }
        int y = pow(x, 0.5);
        for(int i = 2; i <= y; i++){
            if(x % i != 0){
                continue;
            }else{
                return false;
            }
        }
        return true;
    }
    int countPrimes(int n) {
        int ans = 0;
        for(int i = 2; i < n; i++){
            if(func(i)){
                ans++;
                //std::cout<<"i: "<<i<<std::endl;
            }
        }
        return ans;
    }
};
int main()
{   
    Solution a;
    auto ans = a.countPrimes(5000000);
    std::cout<<"ans: "<<ans<<std::endl;         //348513
    
    auto ans2 = a.countPrimes2(5000000);
    std::cout<<"ans2: "<<ans2<<std::endl;       //348515
    
    return 0;
}
