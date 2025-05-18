#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

class SingletonEager{

public:
    static SingletonEager& getInstance(){
        return instance;
    }
    //删除拷贝构造函数和赋值运算符
    SingletonEager(const SingletonEager&) = delete;
    SingletonEager& operator=(const SingletonEager&) = delete;

    void push(int value){
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        cond_var_.notify_one();
    }

    int pop(){
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]{return !queue_.empty();});

        int value = queue_.front();
        queue_.pop();

        return value;
    }

private:
    SingletonEager(){
        std::cout<<"SingletonEager Instance Created"<<std::endl;
    }

    static SingletonEager instance;

    std::queue<int> queue_;
    std::mutex mutex_;
    std::condition_variable cond_var_;

};

SingletonEager SingletonEager::instance;

void producer(){
    SingletonEager& queue = SingletonEager::getInstance();

    for(int i = 1; i <= 10; i++){
        std::cout<<"producer: "<<i<<std::endl;
        queue.push(i);
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }
}

void consumer(){
    SingletonEager& queue = SingletonEager::getInstance();

    for(int i = 1; i <= 10; i++){
        int value = queue.pop();
        std::cout<<"Consumer: "<<value<<std::endl;
    }
}
int main(){

    auto start = std::chrono::high_resolution_clock::now();

    std::thread producerThread(producer);
    std::thread consumerThread(consumer);

    producerThread.join();
    consumerThread.join();

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end- start;

    std::cout<<"the program run time: "<<duration.count()<<" seconds"<<std::endl;

    return 0;
}
