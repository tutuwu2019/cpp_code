/*
 * 悲观锁 (Pessimistic Lock) Demo — 日志限流场景
 *
 * 悲观锁核心思想：假设冲突必然发生，访问共享数据前无条件加锁。
 *
 * 本 demo 场景：多服务日志按条数限流
 *   每个服务有固定条数配额 (kQuota)，多线程并发写日志时需要原子完成：
 *     1. 检查剩余配额是否 > 0
 *     2. 扣减配额
 *     3. 写入日志文件
 *
 *   "检查-操作" 之间存在竞态窗口 (TOCTOU)，这正是悲观锁要解决的问题。
 *
 * 对比两种实现：
 *   ① 无锁 (racy)  — 检查与扣减分离，sleep 放大窗口，稳定复现超配额
 *   ② 悲观锁       — mutex 锁内完成三步，consumed 永远 ≤ quota
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
#include <cstdint>
#include <string>
#include <unordered_map>
#include <cassert>

// ── base64 唯一 ID ────────────────────────────────────────────────────────────

static std::atomic<uint64_t> gSeq{0};

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
    return {out};
}

static std::string makeLine(const std::string& svc, int tid, int seq) {
    std::time_t now = std::time(nullptr);
    char ts[20];
    std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    std::string id = base64Id(gSeq.fetch_add(1, std::memory_order_relaxed));

    char buf[128];
    int n = std::snprintf(buf, sizeof(buf),
        "[%s] [INFO] [req:%s] svc=%-8s th=%02d seq=%05d action=request result=ok",
        ts, id.c_str(), svc.c_str(), tid, seq);
    while (n < 100) buf[n++] = ' ';
    buf[n++] = '\n';
    buf[n]   = '\0';
    return {buf, static_cast<size_t>(n)};
}

// ═══════════════════════════════════════════════════════════════════════════════
// ① 无锁版本（TOCTOU 竞态，用于对比）
// ═══════════════════════════════════════════════════════════════════════════════

struct RacyQuota {
    std::atomic<int64_t> remaining;
    std::atomic<int64_t> written;
    std::atomic<int64_t> dropped;
    explicit RacyQuota(int64_t q) : remaining(q), written(0), dropped(0) {}
};

struct RacyLogger {
    std::ofstream out_;
    std::mutex    io_mtx_;   // 仅保护文件写入本身，不保护配额逻辑
    std::unordered_map<std::string, RacyQuota> quotas_;

    RacyLogger(const std::string& f,
               const std::vector<std::string>& svcs, int64_t quota)
        : out_(f, std::ios::trunc) {
        for (auto& s : svcs) quotas_.try_emplace(s, quota);
    }

    bool write(const std::string& svc, const std::string& line) {
        auto& q = quotas_.at(svc);

        // ── 步骤 1：检查 ──────────────────────────────────────────────────────
        if (q.remaining.load(std::memory_order_acquire) <= 0) {
            q.dropped.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        // ★ 竞态窗口：sleep 模拟业务耗时，保证另一线程在此间隙也通过步骤 1
        std::this_thread::sleep_for(std::chrono::microseconds(200));

        // ── 步骤 2：扣减（两个线程都到这里 → 双重扣减 → 超配额）──────────────
        q.remaining.fetch_sub(1, std::memory_order_acq_rel);
        q.written.fetch_add(1,   std::memory_order_relaxed);

        // ── 步骤 3：写文件（单独加锁，保证文件本身不乱）─────────────────────
        std::lock_guard<std::mutex> lk(io_mtx_);
        out_ << line;
        return true;
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
// ② 悲观锁版本（mutex 保护整个 检查-扣减-写入）
// ═══════════════════════════════════════════════════════════════════════════════

struct SafeQuota {
    int64_t    remaining;
    int64_t    written;
    int64_t    dropped;
    std::mutex mtx;
    SafeQuota() = default;
    SafeQuota(SafeQuota&&) = delete;
};

struct PessimisticLogger {
    std::ofstream out_;
    std::unordered_map<std::string, SafeQuota> quotas_;

    PessimisticLogger(const std::string& f,
                      const std::vector<std::string>& svcs, int64_t quota)
        : out_(f, std::ios::trunc) {
        for (auto& s : svcs) {
            auto& q = quotas_[s];
            q.remaining = quota;
            q.written   = 0;
            q.dropped   = 0;
        }
    }

    bool write(const std::string& svc, const std::string& line) {
        auto& q = quotas_.at(svc);

        std::lock_guard<std::mutex> lk(q.mtx);  // 悲观：进门先拿锁

        // 锁内：检查、扣减、写入原子完成，不存在竞态窗口
        if (q.remaining <= 0) {
            ++q.dropped;
            return false;
        }
        --q.remaining;
        ++q.written;
        out_ << line;
        return true;
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
// 并发驱动
// ═══════════════════════════════════════════════════════════════════════════════

template<typename Logger>
void runConcurrent(Logger& logger,
                   const std::vector<std::string>& svcs,
                   int nThreads, int nMsgsPerThread) {
    std::vector<std::thread> threads;
    threads.reserve(nThreads);
    std::atomic<int>  ready{0};
    std::atomic<bool> go{false};

    for (int i = 0; i < nThreads; ++i) {
        threads.emplace_back([&, i]() {
            std::string svc = svcs[i % svcs.size()];
            ++ready;
            while (!go.load(std::memory_order_acquire))
#if defined(__x86_64__) || defined(__i386__)
                __builtin_ia32_pause();
#else
                ;
#endif
            for (int j = 0; j < nMsgsPerThread; ++j)
                logger.write(svc, makeLine(svc, i, j));
        });
    }

    while (ready.load() < nThreads) {}
    go.store(true, std::memory_order_release);
    for (auto& t : threads) t.join();
}

// ═══════════════════════════════════════════════════════════════════════════════
// main
// ═══════════════════════════════════════════════════════════════════════════════

int main() {
    const std::vector<std::string> svcs = {"auth", "order", "payment"};
    constexpr int     kThreads = 9;    // 每服务 3 个并发写线程
    constexpr int     kMsgs    = 10;   // 每线程写 10 条（配额内外各有一些）
    constexpr int64_t kQuota   = 5;    // 每服务仅允许 5 条，配额边界频繁被踩

    std::puts("=== ① 无锁版本（TOCTOU 竞态）===");
    {
        RacyLogger rl("rw_racy.log", svcs, kQuota);
        runConcurrent(rl, svcs, kThreads, kMsgs);
        for (auto& s : svcs) {
            const auto& q = rl.quotas_.at(s);
            bool over = q.written.load() > kQuota;
            std::printf("  svc=%-8s  written=%3lld  dropped=%3lld  quota=%3lld  %s\n",
                        s.c_str(),
                        (long long)q.written.load(),
                        (long long)q.dropped.load(),
                        (long long)kQuota,
                        over ? "!!! OVER QUOTA !!!" : "ok");
        }
    }

    std::puts("\n=== ② 悲观锁版本 ===");
    {
        PessimisticLogger pl("rw_pess.log", svcs, kQuota);
        runConcurrent(pl, svcs, kThreads, kMsgs);
        for (auto& s : svcs) {
            const auto& q = pl.quotas_.at(s);
            bool over = q.written > kQuota;
            std::printf("  svc=%-8s  written=%3lld  dropped=%3lld  quota=%3lld  %s\n",
                        s.c_str(),
                        (long long)q.written,
                        (long long)q.dropped,
                        (long long)kQuota,
                        over ? "!!! OVER QUOTA !!!" : "ok");
            assert(!over);  // 悲观锁下绝对不超配额
        }
    }

    std::puts("\n结论:");
    std::puts("  悲观锁: 进门先拿锁，检查-扣减-写入原子完成，written ≤ quota 严格保证");
    std::puts("  无锁版: 检查与扣减之间存在窗口，多线程同时通过检查 → written > quota");
    return 0;
}
