#include <iostream>

class Base{
public:
    Base(){
        std::cout<<"construct Base "<<std::endl;
    }
    virtual void test(){
        std::cout<<"this is Base test() "<<std::endl;
    }
    virtual ~Base(){
        std::cout<<"destruct Base "<<std::endl;
    }
};

class Driver: public Base{
public:
    Driver(){
        std::cout<<"Driver construct "<<std::endl;
    }
    void test() override{
        std::cout<<"this is Driver test() "<<std::endl;
    }

    ~Driver(){
        std::cout<<"Driver destruct "<<std::endl;
    }    
};

/**
    test start
    construct Base 
    Driver construct 
    this is Driver test() 
    this is Base test() 
    test once
    this is Driver test() 
    this is Base test() 
    Driver destruct 
    destruct Base 
 * 
 */
int main(){

    std::cout << "test start" << std::endl;

    Driver* driver = new Driver();
    Base* base = dynamic_cast<Base*>(driver);
    
    if(base){
        base->test();           // 调用 Driver::test()
        base->Base::test();     // 调用 Base::test()
    } else {
        std::cout << "dynamic_cast failed" << std::endl;
    }

    std::cout << "test once" << std::endl;

    base = static_cast<Base*>(driver);
    if(base){
        base->test();           // 调用 Driver::test()
        base->Base::test();     // 调用 Base::test()
    } else {
        std::cout << "static_cast failed" << std::endl;
    }

    delete driver;

    return 0;

}
