#include <iostream>
#include <vector>

class MemoryPool{
public:
    MemoryPool(size_t blockSize, size_t blockCount)
    :_blockSize(blockSize), _blockCount(blockCount), _freeList(0){
        _pool = malloc(_blockSize * _blockCount);

        //初始化列表
        char* p  = static_cast<char*>(_pool);

        for(size_t i = 0; i < _blockCount; i++){
            _freeList.push_back(p + i * _blockSize);
        }
    }
    ~MemoryPool(){
        free(_pool);
    }
    void* allocate(){
        if(_freeList.empty()){
            throw std::bad_alloc();
        }
        void* block = _freeList.back();
        _freeList.pop_back();

        return block;
    }

    void deallocate(void* block){
        _freeList.push_back(static_cast<char*>(block));
    }


private:
    size_t _blockSize;
    size_t _blockCount;
    void* _pool;
    std::vector<void*> _freeList;
};
/**
     优点：

    高效的内存分配和释放，特别适用于频繁分配和释放小块内存的场景。
    避免内存碎片问题。
    局限：

    不适用于需要分配不同大小内存块的场景。
    需要预先分配内存，因此在内存池初始化时会占用较大的内存。
        Allocated block at 0x2242fd0
        Allocated block at 0x2242fb0
        Deallocated block at 0x2242fd0
        Allocated block at 0x2242fd0
 */
int main(){
    const size_t blockSize = 32;        //每个内存块的大小  32B
    const size_t blockCount = 10;       //内存块的数量

    MemoryPool pool(blockSize, blockCount);

    void* p1 = pool.allocate();
    std::cout<<"Allocated block at "<< p1<<std::endl;

    void* p2 = pool.allocate();
    std::cout<<"Allocated block at "<<p2<<std::endl;

    pool.deallocate(p1);
    std::cout<<"Deallocated block at "<<p1<<std::endl;

    void* p3 = pool.allocate();
    std::cout<<"Allocated block at "<<p3<<std::endl;

    return 0;
}
