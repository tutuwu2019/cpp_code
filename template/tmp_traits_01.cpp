#include <iostream>
template<typename T>
struct traits;


template<>
struct traits<int>{
    static bool print_test(){
        std::cout<<" traits<int> return false "<<std::endl;
        return false;
    }
};
template<>
struct traits<double>{
    static bool print_test(){
        std::cout<<" traits<double> return false "<<std::endl;
        return false;
    }
};

template<>
struct traits<void>{
    static bool print_test(){
        std::cout<<" traits<void> return true "<<std::endl;
        return true;
    }
};

template<typename T>
void test(){
    std::cout<<(traits<T>::print_test() ? "true" : "false")<<std::endl;
}
int main()
{
    test<int>();
    test<double>();
    test<void>();

    return 0;
}
