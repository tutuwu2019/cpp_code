#include <iostream>
#include <memory>

/**
 *  注意 unique_ptr 把 复制语义给修改了，怎么说呢就是拷贝构造和赋值运算符重载均被delete
 *  只能通过移动构造，把 unique_ptr持有的对内存 移动构造给另一个 unique_ptr 对象
    sp1 is nullptr
    the sp2 is 8
 */
int main(){
    std::unique_ptr<int> sp1(new int(8));
    //std::unique_ptr<int>sp2(sp1);
    std::unique_ptr<int>sp2;
    //sp2 = sp1;
    sp2 = std::move(sp1);
    if(sp1.get() == nullptr){
        std::cout<<"sp1 is nullptr"<<std::endl;
    }else{
        std::cout<<"sp1 is not nullptr"<<std::endl;
    }

    std::cout<<"the sp2 is "<<*(sp2.get())<<std::endl;

    return 0;
}
