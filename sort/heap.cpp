#include <iostream>
#include <vector>

// 调整堆，使得以root为根节点的堆满足最大堆性质
template<typename T>
void maxHeapify(std::vector<T>& vec, int root, int heapSize) {
    int left = 2 * root + 1;
    int right = 2 * root + 2;
    int largest = root;

    /**
     *  调整 子树 root 先找到最大的 val  的 下标
     */
    if (left < heapSize && vec[left] > vec[largest]) {
        largest = left;
    }
    if (right < heapSize && vec[right] > vec[largest]) {
        largest = right;
    }

    /**
     *  如果 idx    != root 继续交换后的子树
     */
    if (largest != root) {
        std::swap(vec[root], vec[largest]);
        maxHeapify(vec, largest, heapSize);
    }
}

// 堆排序算法
template<typename T>
void heapSort(std::vector<T>& vec) {
    int n = vec.size();

    // 构建最大堆
    for (int i = n / 2 - 1; i >= 0; i--) {
        maxHeapify(vec, i, n);
    }

    // 排序  把 头 和 尾交换
    for (int i = n - 1; i > 0; i--) {
        std::swap(vec[0], vec[i]);  // 将堆顶元素（最大值）与当前堆的最后一个元素交换
        maxHeapify(vec, 0, i);      // 调整剩余堆为最大堆
    }
}

// 打印数组元素
template<typename T>
void printArray(const std::vector<T>& vec) {
    for (const auto& elem : vec) {
        std::cout <<"\t"<< elem ;
    }
    std::cout << std::endl;
}

int main() {
    std::vector<int> vec1 = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::vector<double> vec2 = {9.2, 8.0, 7.1, 6.2, 5.5, 4.2, 3.5, 2.8, 1.9};
    std::vector<char> vec3 = {'i', 'h', 'g', 'f', 'e', 'd', 'c', 'b', 'a'};
    
    {
        std::cout << "原始数组: ";
        printArray<int>(vec1);
        heapSort<int>(vec1);
        std::cout << "堆排序结果: ";
        printArray<int>(vec1);
    }
    
    {
        std::cout << "原始数组: ";
        printArray<double>(vec2);
        heapSort<double>(vec2);
        std::cout << "堆排序结果: ";
        printArray<double>(vec2);
    }

    {
        std::cout << "原始数组: ";
        printArray<char>(vec3);
        heapSort<char>(vec3);
        std::cout << "堆排序结果: ";
        printArray<char>(vec3);
    }

    return 0;
}
