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
        std::cout<<"do some thing "<<std::endl;
    }
};
/**
 *      construct A
        do some thing 
        do some again
        do some thing 
        destruct A
 * 

        这是注释部分的结果
 *      construct A
        do some thing 
        destruct A
        sharedPtr is expire 
 */
int main(){
    std::weak_ptr<A> sp2;
    std::shared_ptr<A> sp1(new A);
    sp1->doSomeThing();
    sp2 = sp1;
    // {
    //     std::shared_ptr<A> sp1(new A);
    
    //     sp1->doSomeThing();
    //     sp2 = sp1;
    // }

    // 通过 expire 函数判断 shared_ptr 是否已经释放
    // 如果还没有释放可以通过 wead_ptr lock 获取 shared_ptr 对象再进行 对 shared_ptr 操作
    // 为什么要这么做呢？因为  shared_ptr 没有 -> 操作   也没有 * 操作 也没有 ！ 操作  (operator->、 operator*、operator! 都被禁用了)

    if(sp2.expired()){
        std::cout<<"sharedPtr is expire "<<std::endl;
        return -1;
    }else{
        std::shared_ptr<A> sp3 = sp2.lock();
        std::cout<<"do some again"<<std::endl;
        sp3->doSomeThing();
    }
    std::cout<<"the auto_ptr size is " <<sizeof(std::auto_ptr<int>)<<" ,the unique_ptr size is "<<sizeof(std::unique_ptr<int>)<<" ,the shared_ptr size is "<<sizeof(std::shared_ptr<int>)<<" ,the weak_ptr size is "<<sizeof(std::weak_ptr<int>)<<std::endl;

    return 0;
}
