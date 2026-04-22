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


####
### 模拟

#### 华为机试编程模拟题5
1. 密码强度等级

2. 小A的线段

3. 收集金币



