#include <iostream>
#include <typeinfo>

// 基类异常
class Base {
public:
    Base(const std::string& msg = "Base") : msg_(msg) {}
    virtual ~Base() = default;

    // 虚函数，用于输出类型标识
    virtual const char* name() const { return "Base"; }
    const std::string& message() const { return msg_; }

private:
    std::string msg_;
};

// 派生类异常
class Derived : public Base {
public:
    Derived(const std::string& detail)
      : Base("Derived: " + detail), detail_(detail) {}

    const char* name() const override { return "Derived"; }
    const std::string& detail() const { return detail_; }

private:
    std::string detail_;
};

int main() {
    std::cout << "=== 按值捕获 (slicing 演示) ===\n";
    try {
        throw Derived("extra-info");
    }
    catch (Base e) {  // 按值捕获：发生切片，派生部分被丢弃
        std::cout << "Caught as Base by value\n";
        std::cout << "typeid: " << typeid(e).name() << "\n";
        std::cout << "e.name(): " << e.name() << "\n";
        std::cout << "e.message(): " << e.message() << "\n";
    }

    std::cout << "\n=== 按引用捕获 (保留多态) ===\n";
    try {
        throw Derived("extra-info");
    }
    catch (const Base& e) {  // 按引用捕获：完整保留派生对象
        std::cout << "Caught as Base by const reference\n";
        std::cout << "typeid: " << typeid(e).name() << "\n";
        std::cout << "e.name(): " << e.name() << "\n";
        std::cout << "e.message(): " << e.message() << "\n";
        // 还可以尝试 dynamic_cast 回派生类型
        if (auto pd = dynamic_cast<const Derived*>(&e)) {
            std::cout << "dynamic_cast succeeded, detail(): "
                      << pd->detail() << "\n";
        }
    }

    return 0;
}
