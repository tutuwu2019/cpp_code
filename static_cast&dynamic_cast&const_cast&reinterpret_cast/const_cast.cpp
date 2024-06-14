#include <iostream>

/*
    通过 const_cast 把 char* 对象转换成为 const char*
*/
void test(std::string str){
    std::cout<<"test func is printf the str :"<<str<<std::endl;
}
/*
  有一点要注意，在c++里头 原来的常变量指针是没有发生变化的，还是原来的常变量 但是可以通过 const_cast 获取 常变量对象的地址  然后这个获取的变量的类型就是  去掉了 const 的类型

*/
int main(){
    char* str = "hello, world";
    auto tmp = const_cast<const char*>(str);
    std::cout<<tmp<<std::endl;
    
    std::cout<<str<<std::endl;
    
    test(const_cast<char*>(str));
    
    return 0;
}
