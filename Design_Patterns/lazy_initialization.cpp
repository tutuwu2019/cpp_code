#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

//单例类  懒汉模式
// 懒汉模式，在第一次使用时才初始化实例，适用于延迟加载实例的场景
class SingleTonQueue{
public:
    static SingleTonQueue& getInstance(){
        static SingleTonQueue instance;     // c++11保障线程安全
        return instance;
    }

    //禁用拷贝构造和赋值操作
    SingleTonQueue(const SingleTonQueue&) = delete;
    SingleTonQueue& operator = (const SingleTonQueue&) = delete;

    void push(int value){
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        cond_var_.notify_one();     //通知等待的线程
    }

    int pop(){
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]{return !queue_.empty();});

        int value = queue_.front();
        queue_.pop();

        return value;
    }
private:
    SingleTonQueue() = default;

    std::queue<int> queue_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
};


//生产者
void producer(){
    SingleTonQueue& queue = SingleTonQueue::getInstance();

    for(int i = 1; i <= 10; i++){
        std::cout<<"Producing: "<< i <<std::endl;
        queue.push(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));        //模拟生产延迟  1s
    }
}

void consumer(){
    SingleTonQueue& queue = SingleTonQueue::getInstance();

    for(int i = 1; i <= 10; i ++){
        int value = queue.pop();
        std::cout<<"Consuming: "<<value<<std::endl;
    }
}

int main(){
    auto start = std::chrono::high_resolution_clock::now();
    std::thread producerThread(producer);

    std::thread consumerThread(consumer);

    producerThread.join();
    consumerThread.join();

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    std::cout << "Program run time: " << duration.count() << " seconds" << std::endl;

    return 0;
}
