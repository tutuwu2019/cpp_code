#include <iostream>

class MyClass {
private:
    int x; // 私有成员变量

public:
    void setX(int val) {
        x = val;
    }

    void display() {
        std::cout << "x = " << x << std::endl;
    }
};

int main() {
    MyClass obj;
    // obj.x = 10; // 错误：不能在类外部访问私有成员变量
    obj.setX(10); // 在类外部通过公共成员函数修改私有成员变量
    obj.display(); // 输出 "x = 10"
    return 0;
}
