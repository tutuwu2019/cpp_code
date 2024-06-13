#include <iostream>
#include <cstring>


int main(){
    /*
        sizeof tmp1: 8 strlen tmp1: 12                      sizeof 是运算符，在编译的时候发生作用，计算的是 变量、数据类型、函数、表达式的大小  指针的大小为8字节
        sizeof tmp2: 13 strlen tmp1: 12                                     tmp2[] 这个数组的大小，编译器分配 12 + 1 = 13 个字节 最后1个用于保存截断的 '\0'
        sizeof tmp3: 100 strlen tmp1: 12                                    tmp3   这个数组分配了 100 字节
                                                            strlen 是函数，计算字符串的实际长度，以'\0' 为结束标记
    */
    const char* tmp1 = "hello, world";
    char tmp2[] = "hello, world";
    char tmp3[100] = "hello, world";
    
    std::cout<<"sizeof tmp1: "<<sizeof(tmp1)<<" strlen tmp1: "<<strlen(tmp1)<<std::endl;
    std::cout<<"sizeof tmp2: "<<sizeof(tmp2)<<" strlen tmp1: "<<strlen(tmp2)<<std::endl;
    std::cout<<"sizeof tmp3: "<<sizeof(tmp3)<<" strlen tmp1: "<<strlen(tmp3)<<std::endl;
    
    return 0;
}
