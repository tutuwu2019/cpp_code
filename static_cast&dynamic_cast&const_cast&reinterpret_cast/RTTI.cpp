#include <iostream>
#include <typeinfo>

class Base {
public:
    virtual ~Base() {} // 基类必须有虚函数
};

class Derived : public Base {
public:
    void show() { std::cout << "Derived class method" << std::endl; }
};

class AnotherClass : public Base {
public:
    void display() { std::cout << "Another class method" << std::endl; }
};

void testDynamicCast(Base* b) {
    // 尝试将 Base* 转换为 Derived*
    Derived* d = dynamic_cast<Derived*>(b);
    if (d) {
        d->show(); // 转换成功
    } else {
        std::cout << "Conversion to Derived* failed" << std::endl;
    }

    // 尝试将 Base* 转换为 AnotherClass*
    AnotherClass* a = dynamic_cast<AnotherClass*>(b);
    if (a) {
        a->display(); // 转换成功
    } else {
        std::cout << "Conversion to AnotherClass* failed" << std::endl;
    }
}

/*
    Testing dynamic_cast:
    b1 (Base):
    Conversion to Derived* failed
    Conversion to AnotherClass* failed
    b2 (Derived):
    Derived class method
    Conversion to AnotherClass* failed
    b3 (AnotherClass):
    Conversion to Derived* failed
    Another class method
*/
int main() {
    Base* b1 = new Base();
    Base* b2 = new Derived();
    Base* b3 = new AnotherClass();

    std::cout << "Testing dynamic_cast:" << std::endl;

    std::cout << "b1 (Base):" << std::endl;
    testDynamicCast(b1); // b1 实际上是 Base*

    std::cout << "b2 (Derived):" << std::endl;
    testDynamicCast(b2); // b2 实际上是 Derived*

    std::cout << "b3 (AnotherClass):" << std::endl;
    testDynamicCast(b3); // b3 实际上是 AnotherClass*

    delete b1;
    delete b2;
    delete b3;

    return 0;
}
