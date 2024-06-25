#include <iostream>
#include <memory>

/**
 *  the sp[0] is 0
    the sp[1] is 11
    the sp[2] is 22
    the sp[3] is 33
    the sp[4] is 44
    the sp[5] is 55
    the sp[6] is 66
    the sp[7] is 77
    the sp[8] is 88
    the sp[9] is 99
 */
int main(){
    std::unique_ptr<int[]> sp1(new int[10]);
    for(int i = 0; i < 10; i++){
        sp1[i] = i * 10 + i;
    }

    for(int i = 0; i < 10; i++){
        std::cout<<"the sp["<<i<<"] is "<<sp1[i]<<std::endl;
    }

    return 0;
}
