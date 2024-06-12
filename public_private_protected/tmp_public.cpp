#include <iostream>

class MyClass {
public:
    int x; // 公共成员变量

    void display() { // 公共成员函数
        std::cout << "x = " << x << std::endl;
    }
};

int main() {
    MyClass obj;
    obj.x = 10; // 在类外部访问公共成员变量
    obj.display(); // 在类外部调用公共成员函数
    return 0;
}
