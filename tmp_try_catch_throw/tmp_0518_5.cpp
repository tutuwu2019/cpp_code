#include <iostream>

void func(){
    std::cout<<"the function of func"<<std::endl;
}

void func() noexcept(false){
    std::cout<<"the function of func noexcept(false)"<<std::endl;
}

void func() noexcept(true){
    std::cout<<"the function of func noexcept(true)"<<std::endl;
}

int main(){

    std::cout<<"=======start========"<<std::endl;
    func();
    std::cout<<"=======end========"<<std::endl;
}