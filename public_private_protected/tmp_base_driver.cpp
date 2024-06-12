#include <iostream>

class Base {
public:
    int publicVar;
protected:
    int protectedVar;
private:
    int privateVar;
};

class PublicDerived : public Base {
    // publicVar 是 public
    // protectedVar 是 protected
    // privateVar 不可访问
public:
  void test(){
    std::cout<<publicVar<<std::endl;    //这是对的
    std::cout<<protectedVar<<std::endl;  //这也是可以访问的
    std::cout<<privateVar<<std::endl;    //这是错误的，这个只能在 Base 类 内访问
  }
};

class ProtectedDerived : protected Base {
    // publicVar 是 protected
    // protectedVar 是 protected
    // privateVar 不可访问

public:
  void test(){
    std::cout<<publicVar<<std::endl;    //这是对的 保护继承  把基类的public 成员继承为 派生类的保护成员  所以可以在派生类的类内访问，而在派生类 类外无法访问。
    std::cout<<protectedVar<<std::endl;  //这也是可以访问的
    std::cout<<privateVar<<std::endl;    //这是错误的，这个只能在 Base 类 内访问
  }

};

class PrivateDerived : private Base {
    // publicVar 是 private
    // protectedVar 是 private
    // privateVar 不可访问

public:
  void test(){
    std::cout<<publicVar<<std::endl;    //这是对的，变成了 派生类的私有成员，派生类的私有成员可以在派生类 类内 使用
    std::cout<<protectedVar<<std::endl;  //这个也是对的，变成了派生类的私有成员，派生类是可以获取基类的受保护的成员，但是不能在类外调用 这样也就变成了派生类的保护成员
    std::cout<<privateVar<<std::endl;    //这是错误的，基类的私有成员，无论什么继承方式都是获取不到的，更别提是类内获取还是类外获取
  }

};

int main() {
    PublicDerived pub;
    // pub.publicVar = 1; // 可以访问
    // pub.protectedVar = 2; // 错误，不能在类外部访问 protected 成员
    // pub.privateVar = 3; // 错误，不能在类外部访问 private 成员

    ProtectedDerived prot;
    // prot.publicVar = 1; // 错误，不能在类外部访问 protected 成员
    // prot.protectedVar = 2; // 错误，不能在类外部访问 protected 成员

    PrivateDerived priv;
    // priv.publicVar = 1; // 错误，不能在类外部访问 private 成员
    // priv.protectedVar = 2; // 错误，不能在类外部访问 private 成员

    return 0;
}
