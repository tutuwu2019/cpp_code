#include <iostream>

/*
int main(){
    const int int_const_val = 22;
    int& int_ref_val = const_cast<int&>(int_const_val);
    
    std::cout<<int_ref_val<<std::endl;
    
    int int_val = 77;
    
    float& long_ref = reinterpret_cast<float&>(int_val);
    std::cout<<long_ref<<std::endl;
    
    return 0;
}
*/


/*
    interpret_cast 
        1.用于指针类型转换
            char*  --->   void*
        2. 指针类型转换成整数类型
            int*    ---> size_t
        3. 指针类型转换成指针类型
            int     ---> int*
        
    Address of a: 0x7ffe633487d8
    Value at address: 42
    Address: 140730562807772
    Value at address: 42
*/

/*
int main() {
    int a = 42;
    int* p = &a;
    char* c = reinterpret_cast<char*>(p);

    std::cout << "Address of a: " << static_cast<void*>(c) << std::endl;
    std::cout << "Value at address: " << *reinterpret_cast<int*>(c) << std::endl;
    
    
    /////////////////////
    int tmp_val = 42;
    int* tmp_p = &tmp_val;
    size_t address = reinterpret_cast<size_t>(tmp_p); // 将指针转换为整数类型
    std::cout << "Address: " << address << std::endl;

    int* new_p = reinterpret_cast<int*>(address); // 将整数转换回指针
    std::cout << "Value at address: " << *new_p << std::endl;

    return 0;
}

*/




class Base {
public:
    virtual void show() {
        std::cout << "Base class" << std::endl;
    }
    void tmp(){
        std::cout<<"Base tmp func"<<std::endl;
    }
};

class Derived : public Base {
public:
    void show() override {
        std::cout << "Derived class" << std::endl;
    }
};

/*
    基类与派生类对象  互换

        Derived class
        Base tmp func
        Derived class
*/
int main() {
    Derived d;
    Base* basePtr = reinterpret_cast<Base*>(&d); // 基类和派生类之间的转换
    Derived* d2 = reinterpret_cast<Derived*>(basePtr);

    basePtr->show();
    
    basePtr->tmp();
    
    d2->show();
    
    return 0;
}

