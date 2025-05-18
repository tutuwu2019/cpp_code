#include <iostream>
#include <vector>

template<typename T>
void merge(std::vector<T>& vec, int left, int mid, int right){
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<T> L(n1);
    std::vector<T> R(n2);

    for(int i = 0; i < n1; i++){
        L[i] = vec[left + i];
    }
    for(int i = 0; i < n2; i++){
        R[i] = vec[mid + 1 + i];
    }

    int i = 0, j = 0, k = left;

    while(i < n1 && j < n2){
        if(L[i] < R[j]){
            vec[k] = L[i];
            i++;
        }else{
            vec[k] = R[j];
            j++;
        }
        k++;
    }
    while(i < n1){
        vec[k++] = L[i++];
    }
    while(j < n2){
        vec[k++] = R[j++];
    }

}

template<typename T>
void MyMerge(std::vector<T>& vec, int left, int right){
    if(left < right){
        int mid = (left + right) / 2;
        MyMerge(vec, left, mid);
        MyMerge(vec, mid + 1, right);
        
        merge(vec, left, mid, right);
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
    MyMerge<int>(vec1, 0, n1 - 1);
    myPrint<int>(vec1);

    int n2 = vec2.size();
    MyMerge<double>(vec2, 0, n2 - 1);
    myPrint<double>(vec2);

    int n3 = vec3.size();
    MyMerge<char>(vec3, 0, n3 - 1);
    myPrint<char>(vec3);
}
