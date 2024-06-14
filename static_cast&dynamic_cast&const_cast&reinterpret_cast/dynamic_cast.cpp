/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>

class Base{
public:
    Base(){
        std::cout<<"Base construct"<<std::endl;
    }
    virtual void func(){
        std::cout<<"Base func"<<std::endl;
    }
    virtual ~Base(){
        std::cout<<"Base destruct"<<std::endl;
    }
};

class Driver : public Base{
public:
    Driver(){
        std::cout<<"Driver construct"<<std::endl;
    }
    void func(){
        std::cout<<"Driver func"<<std::endl;
    }
    ~Driver(){
        std::cout<<"Driver destruct"<<std::endl;
    }
};

/*
  通过 dynami_cast  基类指针对象获取派生类指针对象地址  或者说叫，把派生类指针对象转换为基类指针对象  再通过基类指针对象调用派生类继承的虚函数重载，实现了动态多态
test start
Base construct
Driver construct
Driver func
once test
Driver func
Driver destruct
Base destruct
*/
int main()
{
    std::cout<<"test start"<<std::endl;
    Driver tmp;
    tmp.func();
    std::cout<<"once test"<<std::endl;
    
    Base* p = dynamic_cast<Driver*>(&tmp);
    p->func();
    //delete p;
    p = nullptr;
    
    //std::cout<<"test end"<<std::endl;
    return 0;
}
