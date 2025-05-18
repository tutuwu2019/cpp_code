#include <iostream>

struct Str{
    Str(){
        throw 1024;
    }
};
void func1()
try{
    std::cout<<"this is a func1 try catch block"<<std::endl;
    throw 365;
}catch(...){
    std::cout<<"Exception at func1"<<std::endl;
    throw;  // 如果要在调用函数的地方显示，这块不能注释掉
}

void func2(Str x)
try{
    std::cout<<"this is a func2 try catch block"<<std::endl;
}catch(...){
    std::cout<<"Exception at func2"<<std::endl;
}

void func3(){
    try{
        std::cout<<"this is try block"<<std::endl;
    }catch(...){
        std::cout<<"Exception at func3"<<std::endl;
    }
    std::cout<<"the remain func block"<<std::endl;
}

/**
 *  注意啊  function-try-catch 不仅可以在 构造函数初始化使用，也可以在普通函数上使用
 *  但是注意啊，在普通函数上，catch(){}  模块中的 throw 是不会隐式，如果需要调用函数的时候抛出异常 需要显式 throw       365
 * 
 *  在cppreference 中原话是这样的，函数try 块不捕捉从按值传递的函数形参的复制/移动构造函数和析构函数中抛出的异常：这些异常是在调用放的语境中抛出的  1024
 * 
 *  线程的顶层函数的函数try块不捕捉从线程局部对象的构造函数和析构函数(但除了函数作用域的线程局部对象的构造函数)中抛出的异常。  c++11起
 *  
 *  还有 catch 不能 使用 auto 替代对象类型
 */

int main(){
    
    std::cout<<"=====================start==========================="<<std::endl;

    try{
        func1();
    }catch(int& e){
        std::cout<<"Exception at main func, the error is "<<e<<std::endl;
    }

    std::cout<<"======================================================"<<std::endl;

    try{
        func2(Str{});
    }catch(int& e){
        std::cout<<"Exception at main func, the error is "<<e<<std::endl;
    }

    std::cout<<"======================================================"<<std::endl;

    try{
        func3();
    }catch(...){
        std::cout<<"Exception at main func"<<std::endl;
    }

    std::cout<<"=====================end==========================="<<std::endl;

    return 0;
}