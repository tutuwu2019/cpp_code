#include <iostream>
#include <vector>


template<typename T>
void insert(std::vector<T>& vec){
    int n = vec.size();
    
    for(int i = 0, j; i < n - 1; i++){
        auto tmp = vec[i + 1];
        j = i;
        while(j >= 0 && vec[j] > tmp){
            vec[j + 1] = vec[j];
            j--;
        }
        vec[j + 1] = tmp;
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

    insert<int>(vec1);
    myPrint<int>(vec1);

    insert<double>(vec2);
    myPrint<double>(vec2);

    insert<char>(vec3);
    myPrint<char>(vec3);

    return 0;
}
