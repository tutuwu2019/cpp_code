#include <iostream>

class Base{
public:
    int val = 10;
    
    int func(int x){
        std::cout<<"Base func(1) and the return val is "<<x<<std::endl;
        return 0;
    }
    int func(int x, int y){
        std::cout<<"Base func(x, y) and the x is "<<x<<" and the y is "<<y<<std::endl;

        return 0;
    }
    int func2(std::string& str){
        std::cout<<"Base func(str) and the str is "<<str<<std::endl;
        return 0;
    }
};

class Driver : public Base{
public:
    int val = 20;
    /**
     *  Base 中的同名函数 int func(int x) 将被隐藏
     */
    // void func(double y){
    //     std::cout<<"Driver func() and the val y is "<<y<<std::endl;
    // }
};

/**
 *  隐藏
 *      会隐藏基类所有同名函数，包括重载函数，用一句话来讲，就是如果派生类出现了基类同名函数，在调用的时候会优先派生类，而基类的同名函数(包括重载)会被隐藏
 */
int main(){
    //覆盖了基类的成员变量
    Driver t;
    std::cout<<"Driver val is "<<t.val<<std::endl;
    std::cout<<"Base val is "<<t.Base::val<<std::endl;

    std::cout<<"test once.."<<std::endl;

    Driver t2;
    std::string str = "hello, world";
    //t2.func(str);
    t2.func(2);
    t2.func(2, 4);
    //t2.Base::func(2);
    //t2.Base::func(2);

    return 0;
}
