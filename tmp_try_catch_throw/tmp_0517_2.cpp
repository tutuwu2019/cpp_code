#include <iostream>

class a{
public:
    a(){
        std::cout<<"create class a obj\n"<<std::endl;
    }
    ~a(){
        std::cout<<"destory class a obj\n"<<std::endl;
    }
};

class b{
public:
    b(){
        std::cout<<"create class b obj\n"<<std::endl;
    }
    ~b(){
        std::cout<<"destory class b obj\n"<<std::endl;
    }
};
void f1(){
    a a1;
    std::cout<<"===\n"<<std::endl;
    b b1;
    std::cout<<"this is f1 func\n"<<std::endl;
    throw 1;
    std::cout<<"this is after throw\n"<<std::endl;
}

void f2(){
    std::cout<<"this is f2 func\n"<<std::endl;
    try{
        f1();
    }catch(double& e){
        std::cout<<"exception is occurred in f2 func,and the throw is "<<e<<std::endl;
        throw;
    }catch(int& e){
        std::cout<<"exception is occurred in f2 func,and the throw is "<<e<<std::endl;
        throw;
    }
    std::cout<<"this is end of f2 func"<<std::endl;
}
void f3(){
    std::cout<<"this is f3 func\n"<<std::endl;
    try{
        f2();
    }catch(int& e){
        std::cout<<"exception is occurred in f3 func,and the throw is "<<e<<std::endl;
        //throw;
    }
    std::cout<<"this is end of f3 func"<<std::endl;
}

int main(){
    
    f3();
    
    std::cout<<"this is func will go on and then the func will end.\n"<<std::endl;
    return 0;
}
