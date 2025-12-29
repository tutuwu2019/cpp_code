/**
 * @file file_02.cpp
 * 格式化写入文件： 当前线程id、当前写入时间、当前写入内容，写入的内容采用 fmt 库进行格式化
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <ctime>
#include <list>
#include <csignal>
#include <execinfo.h>
#include <cxxabi.h>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>


using namespace std;

// ---------- Crash diagnostics (backtrace + demangle) ----------
static void print_backtrace(){
    void* callstack[128];
    int frames = ::backtrace(callstack, static_cast<int>(sizeof(callstack) / sizeof(callstack[0])));
    char** strs = ::backtrace_symbols(callstack, frames);
    if(!strs){
        std::cerr << "backtrace_symbols failed" << std::endl;
        return;
    }

    std::cerr << "\n===== Backtrace (" << frames << ") =====\n";
    for(int i = 0; i < frames; ++i){
        // Try to demangle function name in the backtrace string.
        // Typical format: binary(function+0x...) [addr]
        std::string s(strs[i]);
        std::string demangled = s;

        auto lparen = s.find('(');
        auto plus = s.find('+', lparen);
        if(lparen != std::string::npos && plus != std::string::npos && plus > lparen + 1){
            std::string mangled = s.substr(lparen + 1, plus - (lparen + 1));
            int status = 0;
            char* out = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
            if(status == 0 && out){
                demangled = s.substr(0, lparen + 1) + out + s.substr(plus);
            }
            std::free(out);
        }

        std::cerr << demangled << "\n";
    }
    std::cerr << "===== End Backtrace =====\n";
    std::free(strs);
}

static void on_terminate(){
    std::cerr << "\n*** terminate called ***\n";
    if(auto eptr = std::current_exception()){
        try{
            std::rethrow_exception(eptr);
        }catch(const std::exception& e){
            std::cerr << "exception: " << e.what() << "\n";
        }catch(...){
            std::cerr << "exception: <non-std exception>\n";
        }
    }
    print_backtrace();
    std::_Exit(1);
}

static void on_signal(int sig){
    std::cerr << "\n*** signal " << sig << " received ***\n";
    print_backtrace();
    std::_Exit(128 + sig);
}

#define RWLOCK_WRITE_FLAG 0x80000000
// 设置文件大小
const size_t MAX_FILE_SIZE = 256 * 1024; // 256 KB (smoke-test rotation; change back to 10MB for production)
// 最多保留多少份轮转文件：.1/.2/.../.N
const int MAX_ROTATE_FILES = 10;

struct LogEntry {
public:
    std::string fileName;
    mutex mtx;

    // 获取文件当前大小（失败返回 0）
    static size_t getFileSize(const std::string& path){
        struct stat st;
        if(::stat(path.c_str(), &st) != 0){
            return 0;
        }
        return static_cast<size_t>(st.st_size);
    }

public:
    // 日志轮转（在持有 mtx 的前提下调用）
    // 规则：file -> file.1, file.1 -> file.2, ... 直到 file.(MAX_ROTATE_FILES-1) -> file.MAX_ROTATE_FILES
    void rotateLogFileLocked() {
        if(MAX_ROTATE_FILES <= 0){
            return;
        }

        // 先删除最老的 file.N
        {
            const std::string oldest = fileName + "." + std::to_string(MAX_ROTATE_FILES);
            ::remove(oldest.c_str());
        }

        // 再从后往前挪动：.2 <- .1, .3 <- .2 ...
        for(int i = MAX_ROTATE_FILES - 1; i >= 1; --i){
            const std::string src = fileName + "." + std::to_string(i);
            const std::string dst = fileName + "." + std::to_string(i + 1);
            // 若 src 不存在，rename 会失败，忽略即可
            (void)std::rename(src.c_str(), dst.c_str());
        }

        // 最后把当前文件挪到 .1
        if(std::rename(fileName.c_str(), (fileName + ".1").c_str()) != 0){
            // 文件可能不存在（首次写入）或被其他逻辑移走；这里打印但不中断
            cerr << "Failed to rotate log file: " << fileName << endl;
        }
    }
};

 /**
  *  读数据 vs 写数据 本质上是共享访问 vs 独占修改
  *  当共享访问时，避免乱序交错，在共享访问的时候把读锁变成互斥锁
  */

static void writeLogEntry(LogEntry& entry,
                          const std::string& timestamp,
                          std::thread::id tid,
                          const std::string& message) {
    std::lock_guard<std::mutex> lock(entry.mtx);

    // 轮转判断放在锁内，保证 rename + 写入不会并发交错
    if(LogEntry::getFileSize(entry.fileName) >= MAX_FILE_SIZE){
        entry.rotateLogFileLocked();
    }

    FILE* fp = std::fopen(entry.fileName.c_str(), "a");
    if(fp == nullptr){
        cerr << "Failed to open log file: " << entry.fileName << endl;
        return;
    }
    std::fprintf(fp, "[%s] [Thread %lu]: %s\n",
                 timestamp.c_str(),
                 static_cast<unsigned long>(std::hash<std::thread::id>{}(tid)),
                 message.c_str());
    std::fclose(fp);
}

// 线程池，多线程调用
//线程数量，以及消息队列
std::list<std::thread> threads;
const int NUM_THREADS = 5;
const int NUM_LOGS_PER_THREAD = 10;

// 写入总时长（“写10分钟”）
constexpr auto RUN_DURATION = std::chrono::minutes(5);
// 每条日志之间的间隔，避免纯忙等占满 CPU
constexpr auto LOG_INTERVAL = std::chrono::milliseconds(10);


// Base64 编码表
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
// 编码任意二进制数据为 base64
static std::string base64_encode_bytes(const unsigned char* data, size_t len){
    std::string out;
    out.reserve(((len + 2) / 3) * 4);

    size_t i = 0;
    while(i + 3 <= len){
        uint32_t n = (static_cast<uint32_t>(data[i]) << 16) |
                     (static_cast<uint32_t>(data[i + 1]) << 8) |
                     (static_cast<uint32_t>(data[i + 2]));
        out.push_back(base64_chars[(n >> 18) & 0x3F]);
        out.push_back(base64_chars[(n >> 12) & 0x3F]);
        out.push_back(base64_chars[(n >> 6) & 0x3F]);
        out.push_back(base64_chars[n & 0x3F]);
        i += 3;
    }

    // Handle 1 or 2 leftover bytes explicitly (avoid reading past end: i+2 may not exist)
    if(i < len){
        const size_t rem = len - i; // 1 or 2
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        if(rem == 2){
            n |= static_cast<uint32_t>(data[i + 1]) << 8;
        }

        out.push_back(base64_chars[(n >> 18) & 0x3F]);
        out.push_back(base64_chars[(n >> 12) & 0x3F]);
        if(rem == 2){
            out.push_back(base64_chars[(n >> 6) & 0x3F]);
            out.push_back('=');
        }else{ // rem == 1
            out.push_back('=');
            out.push_back('=');
        }
    }
    return out;
}

// 生成一个短的 base64 token（不再用 string 拼接，避免任何异常的超大分配）
// 输出长度固定：24 bytes -> base64 32 chars
static void timeStampBase64(std::thread::id threadId, std::string& timeStamp){
    uint64_t salt = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    uint64_t tid = static_cast<uint64_t>(std::hash<std::thread::id>{}(threadId));
    uint64_t t = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count());

    unsigned char buf[24];
    std::memcpy(buf, &t, sizeof(t));
    std::memcpy(buf + 8, &tid, sizeof(tid));
    std::memcpy(buf + 16, &salt, sizeof(salt));

    timeStamp = base64_encode_bytes(buf, sizeof(buf));
}
// 每个线程写入日志
void threadFunction(int threadIndex, LogEntry& entry) {
    const auto endTime = std::chrono::steady_clock::now() + RUN_DURATION;
    int i = 0;
    const std::string prefix = "Log message ";
    const std::string fromThread = " from thread " + std::to_string(threadIndex) + " at ";
    while(std::chrono::steady_clock::now() < endTime){
        const auto tid = std::this_thread::get_id();
        // message 采用 时间+时间戳base64+threadId
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::string ts = std::ctime(&now_c);
        ts.pop_back(); // 去掉换行符
    // base64 token（固定长度 32 chars）
    std::string token;
    timeStampBase64(tid, token);

    // 预分配，减少频繁内存申请
    std::string msg;
    msg.reserve(prefix.size() + 20 + fromThread.size() + token.size());
    msg += prefix;
    msg += std::to_string(i++);
    msg += fromThread;
    msg += token;

        writeLogEntry(entry, ts, tid, msg);
        std::this_thread::sleep_for(LOG_INTERVAL);
    }
}

//传入参数，跑的时间以及文件名
int main(int argc, char** argv) {
    // Enable crash stack traces.
    std::set_terminate(on_terminate);
    std::signal(SIGABRT, on_signal);
    std::signal(SIGSEGV, on_signal);
    std::signal(SIGILL,  on_signal);
    std::signal(SIGFPE,  on_signal);

    LogEntry logEntry;
    if(argc < 2){
        std::cout<<"default file name: threaded_log.txt"<<std::endl;
        logEntry.fileName = "threaded_log.txt";
    }else{
        logEntry.fileName = argv[1];
    }
    // 一直写10分钟
    for(int i = 0; i < NUM_THREADS; ++i){
        threads.emplace_back(threadFunction, i, std::ref(logEntry));
    }
    
    for(auto& t : threads){
        if(t.joinable()){
            t.join();
        }
    }

    return 0;
}

