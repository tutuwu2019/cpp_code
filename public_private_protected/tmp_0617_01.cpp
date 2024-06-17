#include <iostream>

class Base {
public:
    void publicMethod() {
        std::cout << "Base public method" << std::endl;
    }

protected:
    void protectedMethod() {
        std::cout << "Base protected method" << std::endl;
    }

private:
    void privateMethod() {
        std::cout << "Base private method" << std::endl;
    }
};

class Derived : private Base {
public:
    void accessBaseMethods() {
        // 可以访问Base的public和protected成员
        publicMethod();
        protectedMethod();
        // 不能访问Base的private成员
        //privateMethod(); // 错误：无法访问Base的private成员
    }
};

int main() {
    Derived derived;
    derived.accessBaseMethods();
    
    // 不能直接访问Base的public和protected成员
    //  derived.publicMethod(); // 错误：publicMethod在Derived中是private
    // derived.protectedMethod(); // 错误：protectedMethod在Derived中是private

    return 0;   
}
