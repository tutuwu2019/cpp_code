#include <iostream>
#include <stdexcept>

void func(){
    
    throw std::runtime_error("Invalid input");

}

int main(){
    try{
        func();
    }catch(std::runtime_error& e){
        std::cout<<e.what()<<std::endl;
    }

    return 0;
}