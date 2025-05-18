#include <iostream>
#include <string>

class a{
public:
    a(){
        std::cout<<"create class a obj\n"<<std::endl;
    }
    ~a(){
        std::cout<<"destroy class a obj\n"<<std::endl;
    }
};

class b{
public:
    b(){
        std::cout<<"create class b obj\n"<<std::endl;
    }
    b(const b& other) {
        std::cout << "copy constructor of class b\n";
    }

    friend std::ostream& operator<<(std::ostream& os, const b& obj);

    ~b(){
        std::cout<<"destroy class b obj\n"<<std::endl;
        throw 365;
    }
};

std::ostream& operator<<(std::ostream& os, const b& obj) {
    os << "[class b object]";
    return os;
}

void f1(){
    a a1;
    std::cout<<"===\n"<<std::endl;
    b b1;
    std::cout<<"this is f1 func\n"<<std::endl;
    throw (b&)b1;      
    std::cout<<"this is after throw\n"<<std::endl;
}

void f2(){
    std::cout<<"this is f2 func\n"<<std::endl;
    try{
        f1();
    }catch(int e){
        std::cout<<"exception  in f2 func,the throw is "<<e<<std::endl;
    }catch(double e ){
        std::cout<<"exception in f2 func,the throw is "<<e<<std::endl;
    }catch(std::string e ){
        std::cout<<"exception in f2 func,the throw is "<<e<<std::endl;
    }catch(b& e){
        std::cout<<"exception in f2 func,the throw is "<<e<<std::endl;
    }catch(...){
        std::cout<<"exception in f2 func,the throw is Generics.\n";
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
        std::cout<<"exception  in main func\n";
    }
    std::cout<<"this is func will go on and then the func will end.\n"<<std::endl;
    return 0;
}
