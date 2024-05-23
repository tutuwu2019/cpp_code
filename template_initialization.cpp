#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <thread>



// ANSI color codes
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"


template<typename T>
class SingletonEager{
public:
    static SingletonEager& getInstance(){
        return instance;
    }

    //删除拷贝构造含糊和赋值运算函数
    SingletonEager(const SingletonEager&) = delete;
    SingletonEager& operator=(const SingletonEager&) = delete;

    void push(const T& value){
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        cond_var_.notify_one();
    }

    T pop(){
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]{return !queue_.empty();});

        T value = queue_.front();
        queue_.pop();

        return value;
    }
private:
    SingletonEager(){
        std::cout<<"SingletonEager Instance Created"<<std::endl;
    }

    static SingletonEager instance;

    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
};
template<typename T>
SingletonEager<T> SingletonEager<T>::instance;


template<typename T>
void producer(){
    SingletonEager<T>& queue = SingletonEager<T>::getInstance();

    for(int i = 1; i <= 10; i++){
        T value = static_cast<T>(i);
        std::cout<<GREEN<<"Producer: "<<value<<RESET<<std::endl;
        queue.push(value);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

template<typename T>
void consumer(){
    SingletonEager<T>& queue = SingletonEager<T>::getInstance();
    for(int i = 1; i <= 10; i++){
        T value = queue.pop();
        std::cout<<RED<<"Consumer: "<<value<<RESET<<std::endl;
    }
}

int main(){

    auto start = std::chrono::high_resolution_clock::now();
    std::thread producerThread(producer<int>);
    std::thread consumerThread(consumer<int>);
    
    producerThread.join();
    consumerThread.join();

    std::thread producerThreadString([](){
        SingletonEager<std::string>& queue = SingletonEager<std::string>::getInstance();

        for(int i = 1; i <= 10; i++){
            std::string value = "String " + std::to_string(i);
            std::cout<<GREEN<<"Producer: "<<value<<RESET<<std::endl;

            queue.push(value);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });

    std::thread consumerThreadString([](){
        SingletonEager<std::string>& queue = SingletonEager<std::string>::getInstance();

        for(int i = 1; i <= 10; i++){
             std::string value = queue.pop();
             std::cout<<RED<<"Consumer: "<<value <<RESET <<std::endl;
        }
    });

    producerThreadString.join();
    consumerThreadString.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout<<"the program duration "<<duration.count()<<" seconds."<<std::endl;

    return 0;

}