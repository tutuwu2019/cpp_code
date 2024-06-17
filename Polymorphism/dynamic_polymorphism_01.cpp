#include <iostream>

class Base{
public:
    virtual void func1(){
        std::cout<<"Base func1"<<std::endl;
    }
    virtual void func2(){
        std::cout<<"Base func2"<<std::endl;
    }
    virtual void func3(){
        std::cout<<"Base func3"<<std::endl;
    }
private:
    long long a;    // 8B
    int b;          // 4B
    char c;         // 1B
    
};
class Driver : public Base{
public:
    virtual void func1(){
        std::cout<<"Driver func1"<<std::endl;
    }
    virtual void func2(){
        std::cout<<"Driver func2"<<std::endl;
    }
    virtual void func3(){
        std::cout<<"Driver func3"<<std::endl;
    }
};

/**
 *  讲讲c++ 类的内存分布，
 *      默认会按照成员变量声明的顺序进行分配内存，也就是 
 *      [0-7]   a
 *      [8-11]  b
 *      [12]    c
 *      注意，为了按Base 的下一个对象能够按照 long long 类型对其(因为这是类Base对象过的成员变量 最大子空间 )所以采用 8B 为最小存储单位存储 类Base对象
 *      所以，最后一个Base 类的大小为 16B    前面已经有 13B 填充至 16B
 * 
 *      注意哈，还没有结束，因为还有虚函数，这个类又会有一个虚函数表，用来存储虚函数指针地址。当然哈，这个虚函数表也是通过指针去映射的，所以这个类本质上会有一个虚函数指针 8B
 * 
 *      这样，一个Base 的内存大小就是   [0-7 a][8-11 b][12 c][填充部分 13-15] [16-23 虚函数表指针]
 * 
 * 
 *      虚函数表指针，虚函数指针 找到虚函数表，虚函数表上又放了很多虚函数的地址
 * 
 *      通过定义一个 类Base对象，然后通过这个对象可以获取这个对象对应的虚函数表指针(为什么说是这个对象对应的虚函数表指针呢，因为最终的虚函数表上的虚函数指针会根据动态多态映射实际的虚函数，每个对象的虚函数可能是不一样的)
 *      
 *      还有一点要注意，这个对象的虚函数表指针是放在这个对象的头部的(起始位置)，而不是尾部，或者其他位置
 */

/**
 * 
 *      下面的这段代码，我将详细介绍怎么通过类Base对象指针找到虚函数表，再通过虚函数表找到这个对象上实际的虚函数地址
 * 
 *      首先声明一个通过 传入类Base对象指针的一个函数指针
 * 
 *      然后定义一个类Base指针对象
 * 
 *      采用 reinterpret_cast 无类型转换  获取这个对象的 虚函数表地址，这里稍微有点复杂
 *          我们要让 类Base指针对象 解释为 指向 Func*的指针  那就是  reinterpret_cast<Func**>(tmp); 
 * 
 *          注意，还要解引用，把它转为  指向虚函数指针数组的指针  *reinterpret_cast<Func**>(tmp1);
 *          最后， vtable 就是 一个指向虚函数数组的指针
 *          调用虚函数  vtable[0](tmp1)  ...
 * 
 *          虚函数表，其实是一个指针数组， 然后我们又要获取这个数组的地址 
 *              
 */

/**
 *  the sizeof of Base is 24
    Driver func1
    Driver func2
    Driver func3
    test once
    Base func1
    Base func2
    Base func3
 */
int main(){
    typedef void(*Func)(Base*);
    std::cout<<"the sizeof of Base is "<<sizeof(Base)<<std::endl;


    Base* tmp1 = new Driver();

    Func* vtable = *reinterpret_cast<Func**>(tmp1);
    
    vtable[0](tmp1);
    vtable[1](tmp1);
    vtable[2](tmp1);

    std::cout<<"test once"<<std::endl;
    Base* tmp2 = new Base();

    Func* vtable2 = *reinterpret_cast<Func**>(tmp2);
    
    vtable2[0](tmp2);
    vtable2[1](tmp2);
    vtable2[2](tmp2);

    delete tmp1;
    tmp1 = nullptr;
    delete tmp2;
    tmp2 = nullptr;

    return 0;
}
