#include <iostream>
#include <memory>

/**
 *  shared_ptr 可以和 weak_ptr 无差别兼容
 */
int main(){
    std::shared_ptr<int> sp1(new int(123));
    std::cout<<sp1<<" the val is "<<*(sp1.get())<<std::endl;
    std::cout<<"the count is "<<sp1.use_count()<<std::endl;

    std::weak_ptr<int> sp2(sp1);
    std::cout<<"the count is "<<sp2.use_count()<<std::endl;

    std::shared_ptr<int> sp3(sp1);
    std::cout<<"the count is "<<sp3.use_count()<<std::endl;

    std::weak_ptr<int> sp4(sp1);
    std::cout<<"the count is "<<sp4.use_count()<<std::endl;
    return 0;
}
