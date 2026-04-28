# 华为机试

机考3道题，包括编程语言基础、算法、数据结构。线上笔试，3道算法，总分400分，考试时间150分钟，150及格，考试设备需要有摄像头。机考题目分数：100、100、200，按照用例通过计算，每道题基本12个基本测试用例，8个边缘测试用例。若考试没有通过，有半年冷却期，期间无法再次投递。

## 分析

观点1：华为机考[200多道编程题](https://www.nowcoder.com/exam/oj/ta?tpId=37&channelPut=w25post)，发现华为机考常考点，排序、动态规划、双指针、DFS、贪心、模拟、BFS。且编程模式为ACM模式

观点2：2025年新增AI方向，20道选择题(150分)、2道AI编程(150分、200分)，达到180分通过。

观点3：从力扣hot100开始练，刷题过程中注意总结方法。若遇到归纳困难，可以参考代码随想录同类题目解析。

观点4：不会写也要先拿分，能暴力就暴力，别留空。实在不行可以尝试打印样例。[严格卡输出格式，尤其是行末空格等细节](https://www.nowcoder.com/discuss/790918305140711424?sourceSSR=search)。

## 机考题
> 题源牛客网


### 题源

#### HJ1 字符串最后一个单词的长度

从后往前遍历，但是可能最后一个字符的位子就是 空字符， 还有ACM 的刷题方式，会把测试用例用多行的方式测试。还有，如果使用 cin>>str 会以空白字符(空格、换行)作为分隔符

```cpp
std::string str;

while(getline(cin, str)){
  int n = str.size();
  int i = n - 1;
  int len = 0;
  while(i >= 0 && str[i] == ' '){
    i--;
  }
  while(i>= 0 && str[i] != ' '){
    len++;
    i--;
  }
  std::cout<<len;
}
```

#### HJ2 计算某字符出现的次数

注意，如果采用+-32 去找同字符(大小写)，会把数字也加上去，但是这出问题，比如 1+ 32  就是Q
正确的做法，应该是统一转换成大写或者小写  tolower

```cpp
#include <cctype>
#include <iostrewam>

using namespace std;

int main(){
  string str;
  char c;
  getline(sin, str);
  cin>>c;
  int cnt = 0;

  for(int i = 0; i < str.size(); i++){
    // 这种方法有漏洞
    /*
    if(c == str[i] || c + 32 == str[i] || c - 32 == str[i]){
      cnt++;
    }
    */
    // cctype
    if(tolower(c) == tolower(str[i]){
      cnt++;
    }
  }
  cout<<cnt;

return 0;
}
```

#### HJ4 字符串

字符串函数 substr、append

```cpp
int main(){
  string str;
  cin>>str;

  int fullBlock = str.size() / 8;
  int remainder = str.size() % 8;

  // 注意：这里不是 i+= 8 这是一共有多少块
  for(int i = 0; i < fullBlock; i++){
    string outStr = str.substr(i, i + 8);
    cout<<out<<std::endl;
  }
  if(remainder > 0){
    string last = str.substr(fullmainder * 8, remainder);
    last.append(8 - remainder, '0');
    std::cout<<last<<std::endl;
  }
return 0;
}
```

#### HJ5 进制转换

**十六进制转10进制**


#### HJ6 质数因子

[质数的一些性质](https://github.com/tutuwu2019/cpp_code/blob/9d1ef18221933c5be75d76c2a193605b921339c2/%E4%BD%8D%E8%BF%90%E7%AE%97_%E8%B4%A8%E6%95%B0_%E7%BB%84%E5%90%88/readme.md)
给定一个整数，求出从小到大的所有质因子。从2开始，然后允许重复输出。

核心思想是，从2 到 sqrt(n) 遍历，找到符合要求的质数，但是可能最后只有质数1 和它本身。



#### HJ7 
浮点数+0.5 后向下取整(强制转换为int)

5.5	5.5+0.5=6.0 → 6	6  
5.4	5.4+0.5=5.9 → 5	5  
5.0	5.0+0.5=5.5 → 5	5  


#### HJ9 提取不重复的整数

unordered_set、stdoi

比如，输入一个整数 9876673 需要输出为：37689


#### HJ11 数字颠倒

还是常规的 求余、求商

**想想对数字颠倒的封装**


#### Hj12 反转字符串

左右指针，原地反转，零内存开销

```cpp

int main(){
  string str;
  cin>>str;

  int n = str.size();
  int left = 0, right = n - 1;
  while(left < right){
    swap(str[left], str[right]);
    left++;
    right--;
  }
  std::cout<<str<<std::endl;

  return 0;
}



```
#### 句子逆序

```cpp
include <sstream>
#include <iostreawm>
#include <vector>

using namespace std;

int main(){
  string line;
  getline(cin, line);

  vector<string> words;
  stringstrewam ss(line);
  string word;
  while(ss >> word){
    words.push_back(word);
  }

  for(int i = words.size() - 1; i >= 0; i--){
    std::cout<<words[i];
    if(i > 0){
      cout<<" ";
    }
  }
  std::cout<<std<<endl;
  return 0;
}
```

思考方法二：手动分割+双指针
```cpp
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int main() {
    string s;
    getline(cin, s);
    vector<string> words;
    int start = 0;
    for (int i = 0; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == ' ') {
            if (start < i) {
                words.push_back(s.substr(start, i - start));
            }
            start = i + 1;
        }
    }
    for (int i = words.size() - 1; i >= 0; --i) {
        cout << words[i];
        if (i > 0) cout << " ";
    }
    cout << endl;
    return 0;
}

```
####  HJ 14 字符串排序

```cpp
#include <algorithm>
#include <vector>
#include <iostreaw>

using namespace std;

int main(){
  int n = 0;
  cin>>n;
  vector<words> words(n);
  for(int i = 0; i < n; i++){
    cin>>words[i];
  }
  sort(words.begin(), words.end());
  for(int i = 0; i < n; i ++){
    cout<<words[i]<<std::endl;
  }
  return 0;
}
```

#### HJ 15 求 int 型正整数在内存中存储时1的个数


注意 : n &(n - 1) 会消除n 的最右边的那个1  然后通过 循环，每轮消除，直到 n 变成了0

```cpp

int main(){
  int n = 0;
  cin>>n;
  int count = 0;
  while(n){
    n &= (n - 1);
    count++;
  }
  cout<<count<<std::endl;

  return 0;
}
```


#### HJ16  购物单
> 原来在2016年，就已经有人在牛客网上刷题、[面试](https://www.nowcoder.com/discuss/353153935596789760?sourceSSR=users)，但是注意哈，大部分都是知名高校，而且当时他们的水平就是刚毕业的水平。可是那个时候他们明明知道这条路会很难，而且会越走越难，事实证明也是如此。也就是明知山有虎，偏向虎山行！这才是这个世界真实的样子。 2026.04.29 00:34 有感(行动得趁早！)
>> 事实上，薪资与能力挂钩。就算开始蒙混过关，后面也迟早会出尽洋相。当然职场情商、应变能力也非常重要。


<img width="1350" height="2653" alt="image" src="https://github.com/user-attachments/assets/ab4ed3b8-7ef6-49ad-8427-48962ebd7dab" />



#### HJ18 识别有效的IP地址和掩码并进行分类

> todo 

---

####
### 模拟

#### 华为机试编程模拟题5
1. 密码强度等级

2. 小A的线段

3. 收集金币



