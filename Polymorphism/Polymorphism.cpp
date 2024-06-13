#include <iostream>

class Base{
public:
    Base(){
        std::cout<<"Base construct func"<<std::endl;
    }
    virtual void func(){
        std::cout<<"Base func"<<std::endl;
    }
    virtual ~Base(){
        std::cout<<"Base destruct func"<<std::endl;
    }
};

class Driver: public Base{
public:
    Driver(){
        std::cout<<"Driver construct func"<<std::endl;
    }
    void func(){
        std::cout<<"Driver func"<<std::endl;
    }
    ~Driver(){
        std::cout<<"Drive destruct func"<<std::endl;
    }
};

/**
 *  Base construct func
    Driver construct func
    Driver func
    Drive destruct func
    Base destruct func

    多态性和虚函数
        在 C++ 中，多态性允许通过基类指针或引用操作派生类对象。当一个基类指针指向一个派生类对象时，如果基类中有虚函数，调用该虚函数会执行派生类中的覆盖版本。
        这种行为是通过虚函数表（vtable）实现的。虚函数表是一个包含指向虚函数的指针的数组，每个多态性类对象都有一个指向虚函数表的指针。

    如果用 派生类区执行 func 会优先调用 派生类的 func 如果派生类有这个函数，这就是调用这个函数，基类的同名函数全都会被隐藏，如果派生类没有这个函数，则会通过继承的方式获取这个函数。但这不是运行多态的精髓
    定义一个基类指针对象，指向派生类的对象，那么可以通过这个指针对象指向派生类的成员(包括成员方法、成员变量)
    注意啊，这里还有一个问题，就是如果基类的析构不是虚函数，那么基类指针对象在清空内存的时候之后执行基类的析构函数而派生类的析构不会执行，这会造成内存泄漏。应该把基类的析构改成虚函数。
            这样在 delete p； 的时候会先析构派生类再析构基类
 */
int main(){
    Base* p = new Driver();
    p->func();
    delete p;
    p = nullptr;

    return 0;
}
/**
 * 创建对象:

    Derived 对象被创建时，首先调用 Base 类的构造函数，然后调用 Derived 类的构造函数。
    Derived 对象中包含一个指向 Derived 类的虚函数表的指针（vptr）。
    删除对象:

    当执行 delete ptr 时，编译器首先通过基类指针 ptr 调用基类的析构函数。然而，由于基类的析构函数是虚函数，因此会根据对象的实际类型（Derived）通过虚表指针找到 Derived 类的析构函数。
    Derived 类的析构函数被调用，释放 Derived 类特有的资源。
    Derived 类析构函数执行完毕后，调用 Base 类的析构函数，释放基类部分的资源。
    调用顺序
    调用 Derived 类的析构函数。
    调用 Base 类的析构函数。
    虚函数表（vtable）的作用
    虚函数表使得 C++ 可以实现运行时的多态性。每个类都有一个虚函数表，表中记录了该类的虚函数的地址。通过虚函数表指针（vptr），程序在运行时可以确定实际调用的函数。

    当基类的析构函数被声明为虚函数时，派生类对象的虚函数表中会有派生类析构函数的指针。当通过基类指针删除对象时，程序会先调用派生类的析构函数，然后再调用基类的析构函数，从而确保对象的正确析构。

    总结
    基类的析构函数是虚函数时，通过基类指针删除派生类对象，能够首先调用派生类的析构函数，然后再调用基类的析构函数。这是通过虚函数表机制实现的，确保了正确的析构顺序，避免资源泄漏和内存泄漏。虚函数表在运行时确定实际调用的函数，实现了多态性。
 */
