#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "locker.h"


sem empty_slots(10);    //设置缓冲区大小为10
sem full_slots(0);

locker mutex;


void* producer(void* arg){
    while(true){  
        //等待是否有资源  -1 
        empty_slots.wait();
        //有资源就上锁
        mutex.lock();

        //生产一个项目
        std::cout<<"Producer an item..."<<std::endl;
        //生产就解锁
        mutex.unlock();
        //释放信号  +1
        full_slots.post();

        sleep(1);
    }
        return NULL;
}


void* consumer(void* arg){
    while(true){
        full_slots.wait();
        mutex.lock();
        //消费一个项目
        std::cout<<"Consumer an item..."<<std::endl;
        mutex.unlock();
        empty_slots.post();
        sleep(1);
    }
    return NULL;
}

int main(){
    pthread_t prod_thread, cons_thread;
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    return 0;
}



