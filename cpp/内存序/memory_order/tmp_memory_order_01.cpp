#include <iostream>
#include <chrono>
#include <ctime>
#include <atomic>
#include <thread>
#include <iomanip>
#include <condition_variable>
#include <mutex>

class DailyScheduler{
public:
    DailyScheduler(int target_hour, int target_min) : target_hour_(target_hour), target_min_(target_min){
        last_processed_index_.store(0);
        // 启动时将运行标志置为 true
        keep_running_.store(true, std::memory_order_release);
    }

    void run(){
        while(keep_running_.load(std::memory_order_acquire)){
            // 1. 获取当前时间
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);

            // localtime 线程不安全，改用 localtime_r
            std::tm now_tm;         //= std::localtime(&now_c);
            localtime_r(&now_c, &now_tm);
            // 2. 生成当前的 order_index (YYYYMMDD)
            long long current_order_index = (now_tm.tm_year + 1900) * 10000 + 
                                            (now_tm.tm_mon + 1) * 100 + 
                                            (now_tm.tm_mday);
            
            // 3. 核心时序判断：今天是否执行过
            if(current_order_index > last_processed_index_.load(std::memory_order_relaxed)){
                // 4. 是否到了检测时间
                if(now_tm.tm_hour == target_hour_ && now_tm.tm_min >= target_min_){
                    // 5. 执行业务逻辑
                    execute_bussiness_logic(current_order_index);

                    // 6. 更新 order_index 标记为今天已经处理
                    // 使用 memeory_order_release 确保业务逻辑执行的结果在更新索引前对其他线程可见
                    last_processed_index_.store(current_order_index, std::memory_order_release);

                }
            }

            // 使用条件变量等待代替长时间 sleep，这样 stop() 可以立即唤醒线程并退出
            std::unique_lock<std::mutex> lk(mutex_);
            cv_.wait_for(lk, std::chrono::seconds(60), [this]{
                return !keep_running_.load(std::memory_order_acquire);
            });
        }
    }

    void stop(){
        keep_running_.store(false, std::memory_order_release);
        // 唤醒可能在等待/睡眠的线程，让它尽快退出
        cv_.notify_one();
    }

private:
    void execute_bussiness_logic(long long index){
        std::cout << "\n==================================================" << std::endl;
        std::cout << "[触发业务逻辑] Order Index: " << index << std::endl;
        std::time_t now_c = std::time_t{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
        std::cout << "当前时间: " << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << std::endl;
        std::cout << "正在处理 15:00 的业务数据..." << std::endl;
        // 模拟业务处理
        std::cout << "处理完成。" << std::endl;
        std::cout << "==================================================\n" << std::endl;
    }

    int target_hour_;
    int target_min_;
    std::atomic<bool> keep_running_{false};
    std::atomic<long long> last_processed_index_;
    std::mutex mutex_;
    std::condition_variable cv_;

};

int main(){
    //DailyScheduler scheduler(15, 0);
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    DailyScheduler scheduler(now_tm->tm_hour, now_tm->tm_min);

    std::thread t(&DailyScheduler::run, &scheduler);

    std::this_thread::sleep_for(std::chrono::seconds(2));
   
    scheduler.stop();
    // if(t.joinable()){
    //     t.join();
    // }
    t.join();

    return 0;
}
