#include <iostream>
#include <sstream>
#include <fstream>
#include <shared_mutex>
#include <ctime>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

class RWLogger {
    const std::string filename_;
    std::ofstream out_;           // 只追加写，常驻打开
    mutable std::shared_mutex rw_;

    static std::string timestamp() {
        std::time_t now = std::time(nullptr);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return buf;
    }

public:
    explicit RWLogger(const std::string& filename)
        : filename_(filename), out_(filename, std::ios::app) {}

    RWLogger(const RWLogger&) = delete;
    RWLogger& operator=(const RWLogger&) = delete;

    void write(const std::string& msg) {
        std::unique_lock lock(rw_);
        if (out_.good()) {
            out_ << '[' << timestamp() << "] " << msg << '\n';
            out_.flush();
        }
    }

    // 每次读取开一个本地 ifstream，多个读线程各自独立，不共享流状态
    std::string readAll() const {
        std::shared_lock lk(rw_);
        std::ifstream in(filename_);
        if (!in) return {};
        std::ostringstream oss;
        oss << in.rdbuf();
        return oss.str();
    }
};

// ── 并发测试 ──────────────────────────────────────────────

int main() {
    constexpr int kWriters    = 3;
    constexpr int kReaders    = 9;   // 读:写 = 3:1，读多写少
    constexpr int kWriteEach  = 10;
    constexpr int kReadEach   = 20;

    RWLogger logger("rw_log.txt");
    std::atomic<int> readOk{0}, writeOk{0};

    auto writer = [&](int id) {
        for (int i = 0; i < kWriteEach; ++i) {
            logger.write("writer-" + std::to_string(id) + " msg-" + std::to_string(i));
            ++writeOk;
        }
    };

    auto reader = [&](int /*id*/) {
        for (int i = 0; i < kReadEach; ++i) {
            std::string s = logger.readAll();
            (void)s;        // 仅验证不崩溃、不死锁
            ++readOk;
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(kWriters + kReaders);

    for (int i = 0; i < kWriters; ++i) threads.emplace_back(writer, i);
    for (int i = 0; i < kReaders; ++i) threads.emplace_back(reader, i);

    for (auto& t : threads) t.join();

    std::cout << "writes=" << writeOk
              << "  reads=" << readOk << '\n';
    std::cout << "── final log ──\n" << logger.readAll();
    return 0;
}
