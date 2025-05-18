#include <iostream>

void func() noexcept(false){
    std::cout<<"the function of func"<<std::endl;
    throw 1024;
}

void func2() {
    std::cout<<"the function of func2"<<std::endl;
    throw 1024;
}


int main(){

    /**
     *  void (*ptr)() noexcept(true) = func;
     * 
     *  the function of func
     *  terminate called after throwing an instance of 'int'
     *  Aborted
     */
    void (*ptr)() noexcept(false) = func;
    try{
        (*ptr)();
    }catch(...){
        std::cout<<"Exception at main"<<std::endl;
    }

    void (*ptr2)() noexcept(false) = func2;

    try{
        (*ptr2)();
    }catch(...){
        std::cout<<"Exception at main"<<std::endl;
    }

    return 0;
}