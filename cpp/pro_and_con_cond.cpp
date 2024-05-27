#include <iostream>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include "locker.h"

std::queue<int> buffer;
const int MAX_BUFFER_SIZE = 10;
locker mtx;
cond cond_var;

void* producer(void* arg){
    int item = 0;
    for(; ;){
        mtx.lock();
        //如果缓冲区满，则等待
        while(buffer.size() == MAX_BUFFER_SIZE){
                cond_var.wait(mtx.get());
            }

         //生产一个项目并放入缓冲区
        buffer.push(item);
        std::cout<<"Producer: "<<item++<<std::endl;
        //通知消费者有项目更新可消费
         cond_var.broadcast();
         mtx.unlock();
        sleep(1);
    }
     return NULL;
}

void* consumer(void* arg){
    while(true){
        mtx.lock();
        //如果缓冲区为空，则等待
        while(buffer.empty()){
            cond_var.wait(mtx.get());
        }

        //消费一个项目并从缓冲区移除
        int item = buffer.front();
        buffer.pop();
        std::cout<<"Consumer: "<<item<<std::endl;
        //通知生产者有新的空槽
        cond_var.broadcast();
        mtx.unlock();
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



