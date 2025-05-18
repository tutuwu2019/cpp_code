#include <iostream>
#include <vector>

template<typename T>
int partition(std::vector<T>& vec, int low, int high){
    auto pivot = vec[high];
    int i = low - 1;
    for(int j = low; j < high; j++){
        if(vec[j] < pivot){
            i++;
            {
                auto tmp = vec[j];
                vec[j] = vec[i];
                vec[i] = tmp;
            }
        }
    }
    {
        auto tmp = vec[i + 1];
        vec[i + 1] = vec[high];
        vec[high] = tmp;
    }
    return (i + 1);
}

template<typename T>
void quick(std::vector<T>& vec, int low, int high){
    if(low < high){
        int mid = partition(vec, low, high);
        
        quick(vec, low, mid - 1);
        quick(vec, mid + 1, high);
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

    int n1 = vec1.size();
    quick<int>(vec1, 0, n1 - 1);
    myPrint<int>(vec1);

    int n2 = vec2.size();
    quick<double>(vec2, 0, n2 - 1);
    myPrint<double>(vec2);

    int n3 = vec3.size();
    quick<char>(vec3, 0, n3 - 1);
    myPrint<char>(vec3);
}
