#include <iostream>
#include <vector>


template<typename T>
std::vector<T> count(std::vector<T>& vec){
    int n = vec.size();
    if (n == 0) return {};

    // 找到 vec 中的最小值和最大值
    T min = vec[0], max = vec[0];
    for (auto& it : vec) {
        if (it > max) {
            max = it;
        }
        if (it < min) {
            min = it;
        }
    }

    // 创建计数数组并统计每个元素的出现次数
    std::vector<int> count(max + 1, 0);     // 或者用  std::vector<int> count(max -min + 1, 0);
    for (auto& it : vec) {
        count[it ]++;                       // 相应的 这里也要修改 count[it - min]++;
    }

    // 计算前缀和
    for (int i = 1; i < count.size(); i++) {
        count[i] += count[i - 1];
    }

    // 构建输出数组
    std::vector<T> output(n);
    for (int i = n - 1; i >= 0; i--) {
        output[count[vec[i] ] - 1] = vec[i];            // 相应的这里也要修改   output[count[vec[i] - min] - 1] = vec[i];
        count[vec[i] ]--;                               // 相应的这里也要修改   count[vec[i] - min]--;
    }

    return output;
}

template<typename T>
void myPrint(std::vector<T> vec){
    std::cout<<"the sort vec: ";
    for(auto& it : vec){
        std::cout<<"\t"<<it;
    }
    std::cout<<std::endl;
}


int main(){
    std::vector<int> vec1 = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    

    std::cout<<"before the vec1, ";
    myPrint(vec1);
    std::cout<<"after the vec1, ";
    myPrint(count(vec1));

    
    return 0;
}
