#include <iostream>

struct Str1{
    Str1(){
        std::cout<<"create a struct Str1 obj start"<<std::endl;
        throw 512;
    }
    ~Str1(){
        std::cout<<"destroy a struct Str1 obj"<<std::endl;
    }
};

struct Str2{
    Str2(){
        std::cout<<"create a struct Str2 obj start"<<std::endl;
        std::cout<<"create a struct Str2 obj success "<<std::endl;
    }
    ~Str2(){
        std::cout<<"destroy a struct Str2 obj"<<std::endl;
    }
};

class A{
public:
    A()
    try :m_1(Str1{}), m_2(Str2{}){
        std::cout<<"init A::A() start"<<std::endl;
        std::cout<<"init A::A() success"<<std::endl;
    }catch(int& e){
        std::cout<<"Exception at A::A(),the throw ans is "<<e<<std::endl;
    }
    ~A(){
        std::cout<<"destroy a class A obj start"<<std::endl;
        std::cout<<"destroy a class A obj success"<<std::endl;
    }
private:
    /**
     *  注意 m_2 和 m_1 定义的前后顺序会导致，m1、m2 析构和构造的先后顺序
     *  先定义 m_2  m_2 会成功构造，当 m_1 构造的时候 throw 异常，m_1 构造失败，然后紧接着 会把 m_2 析构 在A 对象构造抛出异常，紧接着在 catch 继续隐式 throw 在 main 继续 catch 异常，注意这两次 catch  的结果是一样的
     *  而先定义 m_1 那必然是在 m_1 构造的时候直接 throw 异常，会进行栈展开，会往前释放已经分配的资源(比如通过析构函数)，而后面的对象或者成员要进行分配资源都不会继续。 
     * 
     *  相应的，如果在A 对象初始化失败，要对其他成员资源的清理，需要在构造函数对应的 catch{} 块做资源的清理。
     * 
     *  注意在 抛出异常是在构造对象 A之前，初始化成员 m_1,m_2 的时候，还没有进入 A对象的构造域(也可以理解为 A对象构造失败)，这样必然不用析构 A对象
     * 
     *  还有需要注意，在构造 A对象的时候，初始化列表顺序并不会改变 成员对象m1,m_2 的初始化先后顺序。这两个成员对象初始化的先后顺序由其定义的先后顺序决定的
     */
    Str2 m_2;       
    Str1 m_1;
    
};


int main(){

    try{
        A a;
    }catch(int& e){
        std::cout<<"Exception at main func, the throw ans is "<<e<<std::endl;
    }

    return 0;
}