#include <iostream>
#include <arpa/inet.h> // for htonl
#include <cstdint> // for uint32_t

using u32_t = uint32_t;

/*
SDBM 哈希函数特点
    速度快：SDBM 哈希函数计算速度很快，适合处理大量数据。
    冲突少：由于使用了移位和减法操作，SDBM 哈希函数生成的哈希值分布比较均匀，冲突较少。
    应用广泛：SDBM 哈希函数被广泛应用于文件系统和数据库管理系统中。
*/

u32_t sdbm(unsigned char *str, int len) {
    u32_t hash = 0; // 初始化哈希值为0
    int c; // 用于存储当前字符的ASCII值

    for (int i = 0; i < len; i++) { // 遍历字符串的每个字符
        c = *(str++); // 获取当前字符的ASCII值并移动指针到下一个字符
        // 计算新的哈希值
        // 左移6 相当于乘以2^6
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    // hash = htonl(hash); // 将哈希值转换为网络字节序，如果需要跨网络传输哈希值，可以启用这行代码

    return hash; // 返回计算得到的哈希值
}


int main() {
    unsigned char str[] = "hello";
    int len = sizeof(str) - 1; // 字符串长度
    u32_t hash_value = sdbm(str, len);
    std::cout << "Hash value: " << hash_value << std::endl;
    return 0;
}
