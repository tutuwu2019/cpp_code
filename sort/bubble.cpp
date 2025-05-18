#include <iostream>
#include <vector>

template<typename T>
void func(std::vector<T>& vec){
    int n = vec.size();
    for(int i = 0; i < n - 1; i++){     //循环的次数 只需要循环  n - 1 轮
        for(int j = 0; j < n - 1 - i; j++){     // 比较的次数  n - 1 -i 次   因为前面排好顺序了就不用再排序了
            if(vec[j] > vec[j + 1]){
                auto tmp = vec[j];
                vec[j] = vec[j + 1];
                vec[j + 1] = tmp;
            }
        }
    }
}
template<typename T>
void func2(std::vector<T>& vec){
    std::cout<<"the sort vec: ";
    for(auto& it : vec){
        std::cout<<"\t"<<it;
    }
    std::cout<<std::endl;
}

int main(){
    std::vector<int> vec1 = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::vector<double> vec2 = {9.2, 8.0, 7.1, 6.2, 5.5, 4.2, 3.5, 2.8, 1.9};
    std::vector<char> vec3 = {'i', 'h', 'g', 'f', 'e', 'd', 'c', 'b', 'a'};
    func<int>(vec1);
    func2<int>(vec1);
    func<double>(vec2);
    func2<double>(vec2);
    func<char>(vec3);
    func2<char>(vec3);


}
