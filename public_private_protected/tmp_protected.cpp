#include <iostream>

class Base {
protected:
    int x; // 受保护成员变量

public:
    void setX(int val) {
        x = val;
    }
};

class Derived : public Base {
public:
    void display() {
        std::cout << "x = " << x << std::endl; // 在派生类中访问受保护成员变量
    }
};

int main() {
    Derived obj;
    obj.setX(10);
    obj.display(); // 输出 "x = 10"
    // obj.x = 20; // 错误：不能在类外部访问受保护成员变量      // 破坏类的封装性
    return 0;
}
