#include <iostream>

class Base{
public:
    virtual void func() {

    }
};

class Driver : public Base{
public:
    void func() noexcept(true) override{

    }
};

/**
 *  泛型 func 不允许抛出异常，而实际上派生类的func 允许抛出异常。 基类对象 ref 实际上指向的是派生类的对象d ，那么这会导致最后 ref 在调用func 可能会抛出异常
 *  这与 基类的func 不允许 抛出异常 矛盾。所以 不允许派生类的重写 函数 的 noexcept 与基类的 noexcept 不一致
 * 
 *  注意 基类没有关键词 noexcept 允许抛出异常，派生类重写 noexcept(true)不允许抛出异常  这是可以的。
 *  而，基类如果已经有了旨意，比如 noexcept(false) 派生类只能 noexcept(false)，而不能 noexcept(true)。反之 基类 noexcept(true)，派生类只能 noexcept(true)
 * 
 *  需要注意：再重写的时候，编译器发现 行动不一致，会直接编译报错。而函数指针 行动不一致，编译不会报错，但是会有terminate 
 * 
 */
int main(){
    
    Driver d;
    Base& ref = d;
    ref.func();
    return 0;
}