#include <iostream>

// 定义第一个虚拟基类
class Base1 {
public:
    int value1;
    Base1(int v1) : value1(v1) {
        std::cout << "Base1 constructor called with value: " << value1 << std::endl;
    }
};

// 定义第二个虚拟基类
class Base2 {
public:
    int value2;
    Base2(int v2) : value2(v2) {
        std::cout << "Base2 constructor called with value: " << value2 << std::endl;
    }
};

// 定义一个中间类，继承自两个虚拟基类
class Intermediate : virtual public Base1, virtual public Base2 {
public:
    Intermediate(int v1, int v2) : Base1(v1), Base2(v2) {
        std::cout << "Intermediate constructor called" << std::endl;
    }
};

// 定义最终派生类，继承自中间类
class Final : public Intermediate {
public:
    Final(int v1, int v2) : Base1(v1), Base2(v2), Intermediate(v1, v2) {
        std::cout << "Final constructor called" << std::endl;
    }
};

int main() {
    Final obj(10, 20);
    std::cout << "Final object value1: " << obj.value1 << std::endl;
    std::cout << "Final object value2: " << obj.value2 << std::endl;


  /*
    运行结果
    Base1 constructor called with value: 10
    Base2 constructor called with value: 20
    Intermediate constructor called
    Final constructor called
    Final object value1: 10
    Final object value2: 20
  
  */
    return 0;
}
