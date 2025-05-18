#include <iostream>
#include <cstring>

/*
4     4
8     4 | 8  ==>   8 | 8
1     8 | 8 | 1 ==>   8 | 8 | 8
8     8 | 8 | 8 | 8

所以一共32B
*/
struct MyStruct {
    
    int a;
    double b;
    char c;
    long long d;
    
};
class MyClass {
public:
    int a;
    double b;
    char c;

    void myFunction() {
        // Some function
    }
};

enum MyEnum {
    VALUE1,
    VALUE2,
    VALUE3
};


double test(){
    printf("hello, world!\n");
    return 0.0;
}
void test2(){
    std::cout<<"hello, world!\n"<<std::endl;
    
    return;
}
/*
    the test sizeof: 8
    the test2 sizeof: 8
    the express sizeof: 8
    the strcut sizeof: 32
    the class sizeof: 24
    the enum sizeof: 4

*/ 
int main(){
    std::cout<<"the test sizeof: "<<sizeof(test())<<std::endl;
    std::cout<<"the test2 sizeof: "<<sizeof(&test2)<<std::endl;
    
    double a = 10.0, b = 20.0;
    std::cout<<"the express sizeof: "<<sizeof(a > b ? 50.0 : 5.0)<<std::endl;
    
    std::cout<<"the strcut sizeof: "<<sizeof(MyStruct)<<std::endl;
    
    std::cout<<"the class sizeof: "<<sizeof(MyClass)<<std::endl;
    
    std::cout<<"the enum sizeof: "<<sizeof(MyEnum)<<std::endl;
    
    return 0;
}
