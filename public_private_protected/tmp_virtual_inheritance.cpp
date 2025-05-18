#include <iostream>

class Base {
public:
    int value;
    Base(int v) : value(v) {
        std::cout<<"init the Base and the value: "<<value<<std::endl;
    }
    
    ~Base(){
        std::cout<<"destory Base"<<std::endl;
    }
};

class Derived1 : virtual public Base {
public:
    Derived1(int v) : Base(v) {
        std::cout<<"init the Driver1 "<<std::endl;
    }
    ~Derived1(){
        std::cout<<"destory Derived1"<<std::endl;
    }
};

class Derived2 : virtual public Base {
public:
    Derived2(int v) : Base(v) {
        std::cout<<"init the Driver2 "<<std::endl;
    }
    ~Derived2(){
        std::cout<<"destory Derived2"<<std::endl;
    }
};

class Final : public Derived1, public Derived2 {
public:
    Final(int v1) : Base(v1), Derived1(v1), Derived2(v1) {
        std::cout<<"init the Final "<<std::endl;
    }
    ~Final(){
        std::cout<<"destory Final "<<std::endl;
    }
};


int main(){
    
    Final f(1);

  /*
  运行结果
  init the Base and the value: 1
  init the Driver1 
  init the Driver2 
  init the Final 
  destory Final 
  destory Derived2
  destory Derived1
  destory Base
  
  */
}
