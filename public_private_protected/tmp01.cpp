#include <iostream>

class Base{
public:
    static int i;
    int y = 1;
    static void test(){
        std::cout<<"this is a static test func ..."<<std::endl;
        std::cout<<"i: "<<i++<<std::endl;
        //std::cout<<"y: "<<y<<std::endl;         //报错  error: invalid use of member ‘Base::y’ in static member function
    }
    void test2(){
        test();
        std::cout<<"test once "<<std::endl;
        test3();
        std::cout<<"test once "<<std::endl;
        test4();
    }
private:
    void test3(){
        std::cout<<"this is a private test3..."<<std::endl;
    }
protected:
    void test4(){
        std::cout<<"this is a protected test4..."<<std::endl;
    }
};

class Driver : public Base{
public:
    void test(){
        /*
          protected 受保护的访问控制，允许在类中的方法访问、特定派生方式的派生类在类中中访问
        */
        {
            std::cout<<"test with Driver test the base test4 function"<<std::endl;
            test4();
            std::cout<<"the test end..."<<std::endl;
        }
        /*
          private 访问控制的 只允许在类类中的方法访问，类外以及派生类中都不能访问
        {
            std::cout<<"the test is Driver test with the Base private test3 function"<<std::endl;
            test3();
            std::cout<<"the test end..."<<std::endl;
        }
        */
    }
};

int Base::i = 0;

void testCount(){
    static int i = 0;
    std::cout<<"i: "<<i++<<std::endl;
    
}
int main(){
    
    Base::test();
    Base a;
    std::cout<<"once test2..."<<std::endl;
    a.test2();
    //a.test3();
    Driver tmp;
    tmp.test();

    
    
    return 0;
}
/*
  运行结果
  this is a static test func ...
  i: 1
  test once 
  this is a private test3...
  test once 
  this is a protected test4...
  test with Driver test the base test4 function
  this is a protected test4...
  the test end...

*/
