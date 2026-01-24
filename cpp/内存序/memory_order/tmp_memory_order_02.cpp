#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <condition_variable>


std::mutex mt;
std::atomic<int> cnt = {0};

void f(){
    std::cout<<"start of f func"<<std::endl;
    
    for(int i = 0; i < 1000; i++){
        cnt.fetch_add(1, std::memory_order_release);
    }
    
    std::cout<<"end of f func"<<std::endl;
}

int main(){

    std::vector<std::thread> v;

    for(int n = 0; n < 10; n++){
        
        v.emplace_back(f);
        
    }
    int i = 1;
    for(auto& t : v){
        t.join();
        std::cout<<"index: "<<i++<<" , Final counter value is "<<cnt<<"\n";
    }

    return 0;
}

