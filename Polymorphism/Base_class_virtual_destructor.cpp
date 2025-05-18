#include <iostream>

class Base{
public:
    Base(){
        std::cout<<"Base construct func"<<std::endl;
    }
    virtual ~Base(){
        std::cout<<"Base destruct func"<<std::endl;
    }
};

class Driver : public Base{
public:
    Driver(){
        std::cout<<"Driver construct func"<<std::endl;
    }
    ~Driver(){
        std::cout<<"Driver destruct func"<<std::endl;
    }
};

/**
 *  before 
 *  ~Base(){
        std::cout<<"Base destruct func"<<std::endl;
    }
 * 
    Base construct func
    Driver construct func
    Base destruct func
=======================================================

    after 
    virtual ~Base(){
        std::cout<<"Base destruct func"<<std::endl;
    }

    Base construct func
    Driver construct func
    Driver destruct func
    Base destruct func
 */
int main(){
    Base* p = new Driver();
    delete p;
    p =nullptr;
    
    return 0;
}
