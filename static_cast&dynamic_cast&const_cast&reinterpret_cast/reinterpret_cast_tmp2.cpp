#include <iostream>

typedef int (*FunctionPtr)(int);

int MyFunc(int v){

    return v * 2;
}

unsigned long Hash(void* p){
    unsigned long val = reinterpret_cast<unsigned long>(p);
    return (unsigned long)(val ^ (val >> 16));
}

int main(){
    FunctionPtr tmp_func;
    void* tmp_ptr = reinterpret_cast<void*>(&MyFunc);
    int x = 22;
    tmp_func = reinterpret_cast<FunctionPtr>(tmp_ptr);
    int ans = tmp_func(x);
    std::cout<<"the ans is: "<<ans<<std::endl;

    std::cout<<"test once again"<<std::endl;

    FunctionPtr tmp_func2;
    tmp_func2 = reinterpret_cast<FunctionPtr>(&MyFunc);
    std::cout<<"the ans is: "<<tmp_func2(x)<<std::endl;

    std::cout<<"another test"<<std::endl;

    int a[20];
    for(int i = 0; i < 20; i++){
        std::cout<<"(a + i): "<< (a + i)<<std::endl;
        std::cout<<"Hash( a + i): "<<Hash( a + i)<<std::endl;
    }

    return 0;
}
