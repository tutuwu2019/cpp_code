#include <iostream>
#include <memory>


class A{
public:
    A(){
        std::cout<<"construct A"<<std::endl;
    }
    ~A(){
        std::cout<<"destruct A"<<std::endl;
    }
    void doSomeThing(){
        std::cout<<" do some thing"<<std::endl;
    }
};

/**
 * 事实上出现了内存泄漏，然后sp1 置为空，在 sp2调用个 doSomeThing 出现报错
 *  construct A
1
 do some thing
1
destruct A
0
flag
 do some thing
0
 */
int main(){
    std::shared_ptr<A> sp1(new A);
    std::cout<<sp1.use_count()<<std::endl;
    const auto& sp2 = sp1;

    sp1->doSomeThing();
    std::cout<<sp1.use_count()<<std::endl;
    sp1.reset();
    std::cout<<sp1.use_count()<<std::endl;
    std::cout<<"flag"<<std::endl;
    sp2->doSomeThing();

    std::cout<<sp1.use_count()<<std::endl;

    return 0;
}
