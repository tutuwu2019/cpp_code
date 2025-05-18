#include <iostream>
#include <memory>

class A{
public:
    A(){
        std::cout<<"construct class A"<<std::endl;
    }
    void test(){
        std::cout<<"this is a class test"<<std::endl;
    }
    ~A(){
        std::cout<<"destruct class A"<<std::endl;
    }
};

/**
 *  construct class A
    this is a class test
    destruct class A
 */
int main(){
    std::unique_ptr<A> sp1(new A);
    sp1->test();

    return 0;
}
