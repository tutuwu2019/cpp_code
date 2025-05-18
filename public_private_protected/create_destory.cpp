#include <iostream>

class Base{
public:
    int i;
public:
    Base(int x):i(x){
        std::cout<<"start Base..."<<std::endl;
    }
    ~Base(){
        std::cout<<"destory Base"<<std::endl;
    }
};

class Intermediate : public Base{
public:
    int j;
public:
    Intermediate(int x, int y):Base(x), j(y){
        std::cout<<"start Intermediate..."<<std::endl;
    }
    ~Intermediate(){
        std::cout<<"destory Intermediate "<<std::endl;
    }
};

class Driver: public Intermediate{
public:
    int k;
public:
    Driver(int x, int y, int z):Intermediate(x, y),k(z){
        std::cout<<"start Driver..."<<std::endl;
    }
    ~Driver(){
        std::cout<<"destory Driver "<<std::endl;
    }
};


int main(){
    
    Driver a(1, 2, 3);
    std::cout<<"a.i(Base::i): "<<a.i<<std::endl;
    std::cout<<"a.j(Intermediate::j): "<<a.j<<std::endl;
    std::cout<<"a.k(Driver::k): "<<a.k<<std::endl;
    
    return 0;

  /*
    运行结果

    start Base...
    start Intermediate...
    start Driver...
    a.i(Base::i): 1
    a.j(Intermediate::j): 2
    a.k(Driver::k): 3
    destory Driver 
    destory Intermediate 
    destory Base
  
  */
    
}
