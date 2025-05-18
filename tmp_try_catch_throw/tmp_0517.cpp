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
    }catch(int){
        std::cout<<"exception is occured in f2 func\n";
    }
}
void f3(){
    std::cout<<"this is f3 func\n"<<std::endl;
    f2();
}

int main(){
    try{
        f3();
    }catch(int){
        std::cout<<"exception is occured in mian func\n";
    }
    //std::cout<<"this is main func, and the main had throw.\n"<<std::endl;
    std::cout<<"this is func will go on and then the func will end.\n"<<std::endl;
    return 0;
}
