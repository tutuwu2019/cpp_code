#include <iostream>
#include <memory>

class A : public std::enable_shared_from_this<A>{
public:
    A(){
        std::cout<<"construct class A"<<std::endl;
    }
    ~A(){
        std::cout<<"destruct class A"<<std::endl;
    }
    void Func(){
        m_SelfPtr = shared_from_this();
        std::cout<<"line 14 the count is "<<m_SelfPtr.use_count()<<std::endl;
    }
private:
    std::shared_ptr<A> m_SelfPtr;
    //std::weak_ptr<A> m_SelfPtr;       这是正确的修改方案
};
/**
 *  必须销毁 A 才能销毁其成员变量 m_SelfPtr，而销毁 m_SelfPtr 必须先销毁 A。这就是所谓的 std::enable_shared_from_this 的循环引用问题。
 */
int main(){
    std::cout<<"test start"<<std::endl;
    std::shared_ptr<A> sp(new A);
    std::cout<<"line 23 the count is "<<sp.use_count()<<std::endl;
    sp->Func();
    std::cout<<"line 25 the count is "<<sp.use_count()<<std::endl;
    std::cout<<"test end"<<std::endl;
    return 0;
}
