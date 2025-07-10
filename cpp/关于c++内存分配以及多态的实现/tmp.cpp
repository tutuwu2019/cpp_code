#include <iostream>

double pi = 3.1415926;

int inx;

class Basic{
public:
    static float alias;
    static int x1 = 10;      // 此处会在编译的时候报错
    static void show1(){
        std::cout<<"alias: "<<alias<<std::endl;
    }
    void show2(){
        std::cout<<"alias*2:"<<alias*2<<std::endl;
    }

};

float Basic::alias = 0.23333;
// int Basic::x1 = 10;  //  不在类外定义 会报定义为使用错误  undefined reference to `Basic::x1'  如果没有通过类对象或者类通过作用域符调用 便不会报这个错误。简单来说，就是声明不分配空间

int main(){
    Basic tmp;
    
    
    Basic::show1();
    tmp.show1();
    tmp.show2();
    std::cout<<"为初始化的静态成员变量 tmp.x1: "<<tmp.x1<<" Basic::x1: "<<Basic::x1<<std::endl;

    std::cout<<"已经初始化的全局变量pi: "<<pi<<std::endl;
    std::cout<<"未初始化的全局变量inx: "<<inx<<std::endl;
    return 0;

}
