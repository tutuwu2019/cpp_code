#include <iostream>

/**
 *  noexcept 关键字
 *  true 不会抛出异常
 *  false 会抛出异常
 */

void func1() noexcept(false){
    std::cout<<" the function of func1"<<std::endl;
}

void func2(){
    std::cout<<"the function of func2"<<std::endl;
}

void func3(){
    std::cout<<"the function of func3"<<std::endl;
}

/**
 *   func2 和 func3 都不抛出异常，则 func4 就不抛出异常
 */
void func4() noexcept(noexcept(noexcept(func2()) && noexcept(func3()))){
    std::cout<<"the function of func4 start"<<std::endl;
    func2();
    func3();
    std::cout<<"the function of func4 end"<<std::endl;
}

/**
 *  如果 设定 函数不会抛出异常，那么会告诉编译器不用在这部分进行异常处理(包括栈展开)
 *      这样的话，函数设置为不抛出异常，而实际上函数内部抛出异常，这样会导致 terminate
 */

/**
 *  tmp_0518_4.cpp:38:11: warning: throw will always call terminate() [-Wterminate]
 *  throw 1024;
 */
void func5() noexcept(false){
    std::cout<<"the function of func5 start"<<std::endl;
    throw 1024;
    std::cout<<"the function of func5 end"<<std::endl;
}

void func6() noexcept{
    std::cout<<"the function of func6 start"<<std::endl;
    throw 2048;
    std::cout<<"the function of func6 end"<<std::endl;
}

int main(){
    std::cout<<"=======start========"<<std::endl;
    func4();

    try{
        func5();
    }catch(...){
        std::cout<<"Exception at main"<<std::endl;
    }

    try{
        func6();
    }catch(...){
        std::cout<<"Exception at main"<<std::endl;
    }
    std::cout<<"=======end========"<<std::endl;
}