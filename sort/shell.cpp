#include <iostream>
#include <vector>

template<typename T>
void shell(std::vector<T>& vec){
    int n = vec.size();

    for(int gap = n / 2; gap > 0; gap /=2){

        for(int i = 0, j; i < n; i++){
            auto tmp = vec[i];
            j = i - gap;
            while(j >= 0 && vec[j] > tmp){
                vec[j + gap] = vec[j];
                j = j - gap;
            }
            vec[j + gap] = tmp;
        }
    }
}

template<typename T>
void myPrint(std::vector<T>& vec){
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

    shell<int>(vec1);
    myPrint<int>(vec1);

    shell<double>(vec2);
    myPrint<double>(vec2);

    shell<char>(vec3);
    myPrint<char>(vec3);

    return 0;
}
