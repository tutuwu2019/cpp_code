/*
 * 自旋锁 vs 互斥锁 — 多线程日志文件写入 benchmark
 *
 * 原理：
 *   互斥锁有竞争时会调用 futex_wait 陷入内核，线程挂起 + 唤醒一轮约 1~10 µs
 *   自旋锁始终在用户态忙等，每次 CAS 约 1~5 ns
 *   日志临界区 = "把一行写入 ofstream 缓冲区"（~100 ns），远小于 futex 唤醒代价
 *   所以在线程数 ≤ 核数、临界区极短的条件下，自旋锁开销更低
 *
 * 当线程数 > 核数时自旋锁会浪费 CPU，mutex 反而可能更好，demo 中一并展示
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>

// ── SpinLock ─────────────────────────────────────────────────────────────────

class SpinLock {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
public:
    void lock() noexcept {
        while (flag_.test_and_set(std::memory_order_acquire))
#if defined(__x86_64__) || defined(__i386__)
            __builtin_ia32_pause();   // 减少流水线冲刷 + 降低功耗
#else
            ;
#endif
    }
    void unlock() noexcept { flag_.clear(std::memory_order_release); }
};

// ── 日志格式 ──────────────────────────────────────────────────────────────────

// 全局单调递增序号，保证每条记录的 base64 ID 跨线程唯一
static std::atomic<uint64_t> gSeq{0};

// 将 uint64_t 低 6 字节编码为 8 位 base64 字符串（无填充，6 字节 = 2 组×3）
static std::string base64Id(uint64_t v) {
    static constexpr char kTab[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint8_t b[6];
    for (int i = 5; i >= 0; --i) { b[i] = v & 0xFF; v >>= 8; }
    char out[9] = {};
    for (int i = 0, k = 0; i < 6; i += 3) {
        uint32_t g = (uint32_t(b[i]) << 16) | (uint32_t(b[i+1]) << 8) | b[i+2];
        out[k++] = kTab[(g >> 18) & 63];
        out[k++] = kTab[(g >> 12) & 63];
        out[k++] = kTab[(g >>  6) & 63];
        out[k++] = kTab[ g        & 63];
    }
    return {out};  // 8 chars
}

// 格式: [YYYY-MM-DD HH:MM:SS] [INFO] [req:XXXXXXXX] th=NN seq=NNNNN action=user-login result=ok latency=NNNms
// 锁外完成全部格式化，临界区只做流写入
static std::string makeLine(int threadId, int seq) {
    std::time_t now = std::time(nullptr);
    char ts[20];
    std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    uint64_t uid = gSeq.fetch_add(1, std::memory_order_relaxed);
    std::string id = base64Id(uid);

    char buf[128];
    int n = std::snprintf(buf, sizeof(buf),
        "[%s] [INFO] [req:%s] th=%02d seq=%05d action=user-login result=ok latency=%03dms",
        ts, id.c_str(), threadId, seq, seq % 999 + 1);

    // 补空格使正文恰好 100 字符（不含 \n）
    while (n < 100) buf[n++] = ' ';
    buf[n++] = '\n';
    buf[n]   = '\0';
    return {buf, static_cast<size_t>(n)};
}

// ── Logger 变体 ───────────────────────────────────────────────────────────────

struct MutexLogger {
    std::ofstream out_;
    std::mutex mtx_;
    explicit MutexLogger(const std::string& f) : out_(f, std::ios::trunc) {}

    void write(const std::string& line) {
        std::lock_guard<std::mutex> lk(mtx_);
        out_ << line;        // ofstream 内置用户态缓冲，临界区 ≈ 内存拷贝
    }
    void flush() { out_.flush(); }
};

struct SpinLogger {
    std::ofstream out_;
    SpinLock spin_;
    explicit SpinLogger(const std::string& f) : out_(f, std::ios::trunc) {}

    void write(const std::string& line) {
        spin_.lock();
        out_ << line;
        spin_.unlock();
    }
    void flush() { out_.flush(); }
};

// ── Benchmark ─────────────────────────────────────────────────────────────────

template<typename Logger>
long long bench(const std::string& file, int nThreads, int nMsgsPerThread) {
    Logger logger(file);
    std::vector<std::thread> threads;
    threads.reserve(nThreads);

    // 栅栏：所有线程就绪后统一起跑，最大化竞争
    std::atomic<int>  ready{0};
    std::atomic<bool> go{false};

    for (int i = 0; i < nThreads; ++i) {
        threads.emplace_back([&, i]() {
            ++ready;
            while (!go.load(std::memory_order_acquire))
#if defined(__x86_64__) || defined(__i386__)
                __builtin_ia32_pause();
#else
                ;
#endif
            for (int j = 0; j < nMsgsPerThread; ++j)
                logger.write(makeLine(i, j));
        });
    }

    while (ready.load() < nThreads) {}
    auto t0 = std::chrono::high_resolution_clock::now();
    go.store(true, std::memory_order_release);
    for (auto& t : threads) t.join();
    logger.flush();
    auto t1 = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
}

// ── main ──────────────────────────────────────────────────────────────────────

int main() {
    constexpr int kMsgs = 20'000;   // 每线程写入条数

    std::printf("%-8s %12s %12s %10s  %s\n",
                "threads", "mutex(µs)", "spin(µs)", "m/s ratio", "winner");
    std::printf("%-8s %12s %12s %10s\n",
                "-------", "---------", "--------", "---------");

    for (int t : {1, 2, 4, 8, 12, 16, 24, 32}) {
        long long m = bench<MutexLogger>("bench_mutex.log", t, kMsgs);
        long long s = bench<SpinLogger> ("bench_spin.log",  t, kMsgs);

        double ratio = (double)m / s;
        const char* winner = ratio >= 1.05 ? "spin  <--"
                           : ratio <= 0.95 ? "mutex <--"
                           : "tie";

        std::printf("%-8d %12lld %12lld %10.2fx  %s\n",
                    t, m, s, ratio, winner);
    }

    std::puts("\n说明:");
    std::puts("  m/s > 1 → 自旋锁更快: 临界区(内存写缓冲)远短于 futex 唤醒延迟");
    std::puts("  m/s < 1 → 互斥锁更快: 线程超过核数，自旋占满 CPU 反而阻碍持锁者");
    std::puts("  临界区 = ofstream << line (用户态内存拷贝，锁外已完成格式化)");
    return 0;
}
