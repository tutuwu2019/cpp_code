#include <iostream>

int main(){
    const int int_const_val = 22;
    int& int_ref_val = const_cast<int&>(int_const_val);
    
    std::cout<<int_ref_val<<std::endl;
    
    int int_val = 77;
    
    float& long_ref = reinterpret_cast<float&>(int_val);
    std::cout<<long_ref<<std::endl;
    
    return 0;
}
