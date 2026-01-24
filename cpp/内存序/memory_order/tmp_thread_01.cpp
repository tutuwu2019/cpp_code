#include <thread>
#include <iostream>

int data = 42;


void thread_1(){
    std::cout<<"thread_1 func before, data: "<<data<<std::endl;
    data++;
    std::cout<<"thread_1 func end, data: "<<data<<std::endl;
}


void thread_2(){
    //std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout<<"thread_2 func before, data: "<<data<<std::endl;
    data++;
    std::cout<<"thread_2 func end, data: "<<data<<std::endl;
}

int main(){

    std::cout<<"1. main func, data: "<<data<<std::endl;
    data = 44;
    std::cout<<"2. main func, data: "<<data<<std::endl;
    std::thread t1(thread_1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout<<"3. main func, data: "<<data<<std::endl;
    
    std::thread t2(thread_2);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    data = 49;
    //std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout<<"4. main func, data: "<<data<<std::endl;
    t1.join();
    t2.join();

    std::cout<<"5. main func, data: "<<data<<std::endl;

    return 0;
}
