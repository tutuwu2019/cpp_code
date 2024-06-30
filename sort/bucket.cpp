#include <iostream>
#include <vector>

template<typename T>
void insertSort(std::vector<T>& vec){
    int n = vec.size();
    for(int i = 1; i < n; i++){
        T tmp = vec[i];
        auto j = i - 1;
        while(j >= 0 && vec[j] > tmp){
            vec[j + 1] = vec[j];
            j--;
        }
        vec[j + 1] = tmp;
    }
}


void bucketSortInt(std::vector<int>& vec){
    int n = vec.size();
    if(n <= 0){
        return;
    }
    int Max_ = vec[0], Min_ = vec[0];
    for(auto& it : vec){
        if(it > Max_){
            Max_ = it;
        }
        if(it < Min_){
            Min_ = it;
        }
    }

    int bucketCount = Max_ - Min_ + 1;

    std::vector<std::vector<int>> buckets(bucketCount);

    for(int i = 0; i < n; i++){
        int idx = vec[i] - Min_;
        buckets[idx].push_back(vec[i]);
    }
    for(int i = 0; i< bucketCount; i++){
        if(!buckets[i].empty()){
            insertSort(buckets[i]);
        }
    }
    int idx = 0;
    for(int i = 0; i < bucketCount; i++){
        for(int j = 0; j < buckets[i].size(); j++){
            vec[idx++] = buckets[i][j];
        }
    }
}

void bucketSortDouble(std::vector<double>&vec){
    int n = vec.size();
    if(n <= 0){
        return;
    }
    double  Max_ = vec[0], Min_ = vec[0];
    for(int i = 0; i < n; i++){
        if(vec[i] > Max_){
            Max_ = vec[i];
        }
        if(vec[i] < Min_){
            Min_ = vec[i];
        }
    }
    std::vector<std::vector<double>> buckets(n);

    for(int i = 0; i < n; i++){
        int idx = static_cast<int>((vec[i] - Min_) / (Max_ - Min_));
        if(idx == n){
            idx--;
        }
        buckets[idx].push_back(vec[i]);
    }
    for(int i = 0; i < n; i++){
        if(!buckets[i].empty()){
            insertSort(buckets[i]);
        }
    }

    int idx = 0;
    for(int i = 0; i < n; i++){
        for(int j = 0; j < buckets[i].size(); j++){
            vec[idx++] = buckets[i][j];
        }
    }
}

void bucketSortChar(std::vector<char>& vec){
    int n = vec.size();
    if(n <= 0){
        return;
    }

    std::vector<std::vector<char>> buckets(256);

    for(int i = 0; i < n; i++){
        int idx = vec[i];
        buckets[idx].push_back(vec[i]);
    }

    for(int i = 0; i < 256; i++){
        if(!buckets[i].empty()){
            insertSort(buckets[i]);
        }
    }

    int idx = 0; 
    for(int i = 0; i < 256; i++){
        for(int j = 0; j < buckets[i].size(); j++){
            vec[idx++] = buckets[i][j];
        }
    }

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
    std::vector<double> vec2 = {9.2, 8.0, 7.1, 6.2, 5.5, 4.2, 3.5, 2.8, 1.9};
    std::vector<char> vec3 = {'i', 'h', 'g', 'f', 'e', 'd', 'c', 'b', 'a'};

    std::cout<<"before the vec1, ";
    myPrint(vec1);
    
    bucketSortInt(vec1);
    std::cout<<"after the vec1, ";
    myPrint(vec1);

    std::cout<<"double test"<<std::endl;
    std::cout<<"before the vec2, ";
    myPrint<double>(vec2);
    bucketSortDouble(vec2);
    std::cout<<"after the vec2, ";
    myPrint<double>(vec2);

    std::cout<<"char test"<<std::endl;
    std::cout<<"before the vec3, ";
    myPrint<char>(vec3);
    bucketSortChar(vec3);
    std::cout<<"after the vec3, ";
    myPrint<char>(vec3);
    return 0;
}
