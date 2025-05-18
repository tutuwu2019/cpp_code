#include <iostream>
#include <vector>

/**
 *  基数排序的方式可以采用 LSD（Least significant digital） 或 MSD（Most significant digital） 。

    LSD 的排序方式由键值的 最右边（最小位） 开始，而 MSD 则相反，由键值的 最左边（最大位） 开始。

    MSD 方式适用于位数多的序列，LSD 方式适用于位数少的序列。
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>

// 基数排序适用于整数类型
void countingSortForRadix(std::vector<int>& vec, int exp) {
    int n = vec.size();
    std::vector<int> output(n);
    int count[10] = {0};

    for (int i = 0; i < n; i++) {
        count[(vec[i] / exp) % 10]++;
    }

    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }

    for (int i = n - 1; i >= 0; i--) {
        output[count[(vec[i] / exp) % 10] - 1] = vec[i];
        count[(vec[i] / exp) % 10]--;
    }

    for (int i = 0; i < n; i++) {
        vec[i] = output[i];
    }
}

void radixSortInt(std::vector<int>& vec) {
    int maxElement = *std::max_element(vec.begin(), vec.end());

    for (int exp = 1; maxElement / exp > 0; exp *= 10) {
        // 1 -> 10 -> 100
        countingSortForRadix(vec, exp);
    }
}

// 基数排序适用于字符串类型
void countingSortForRadix(std::vector<std::string>& vec, int pos) {
    int n = vec.size();
    std::vector<std::string> output(n);
    int count[256] = {0};

    for (int i = 0; i < n; i++) {
        count[(pos < vec[i].length()) ? (vec[i][pos] + 1) : 0]++;
    }

    for (int i = 1; i < 256; i++) {
        count[i] += count[i - 1];
    }

    for (int i = n - 1; i >= 0; i--) {
        output[count[(pos < vec[i].length()) ? (vec[i][pos] + 1) : 0] - 1] = vec[i];
        count[(pos < vec[i].length()) ? (vec[i][pos] + 1) : 0]--;
    }

    for (int i = 0; i < n; i++) {
        vec[i] = output[i];
    }
}

void radixSortString(std::vector<std::string>& vec) {
    int maxLen = 0;
    for (const auto& str : vec) {
        if (str.length() > maxLen) {
            maxLen = str.length();
        }
    }

    for (int pos = maxLen - 1; pos >= 0; pos--) {
        countingSortForRadix(vec, pos);
    }
}

// 打印函数
template<typename T>
void myPrint(const std::vector<T>& vec) {
    std::cout << "The sorted vec: ";
    for (const auto& it : vec) {
        std::cout << "\t" << it;
    }
    std::cout << std::endl;
}

int main() {
    std::vector<int> vec1 = {170, 45, 75, 90, 802, 24, 2, 66};
    std::vector<std::string> vec2 = {"apple", "banana", "grape", "cherry", "mango"};

    std::cout << "Before sorting vec1: ";
    myPrint(vec1);
    radixSortInt(vec1);
    std::cout << "After sorting vec1: ";
    myPrint(vec1);

    std::cout << "Before sorting vec2: ";
    myPrint(vec2);
    radixSortString(vec2);
    std::cout << "After sorting vec2: ";
    myPrint(vec2);

    return 0;
}
