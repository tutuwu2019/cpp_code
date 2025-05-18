#include <iostream>

// function-try-catch

struct Str{
    Str(){
        std::cout<<"create a struct str obj"<<std::endl;
        throw 100;
    }
};

/**
 *  注意：在初始化构造的时候，无论写与写  :m_mem 都会在 初始化构造抛出异常，无非一个是隐式成员变量初始化，一个显示成员初始化
 *  再注意一点：对于function-try-catch 捕获异常以后会在catch 再隐式的throw 扔出异常，这是为了在构造 A a; 这个对象的时候能够有机会 catch 异常
 * 
 *  注意：在构造函数使用 try 块，不能像普通函数 那样写成形如  A(){ try: m_mem(){xxx}catch(...){yyy}}
 */
class A{
public:
    A()
    try: m_mem(){
        std::cout<<"creat a class A obj"<<std::endl;
    }catch(int){
        std::cout<<"Exception at A:A()"<<std::endl;
        //throw;
    }
    
    int x;
private:
    Str m_mem;    
};
/**
 *  Exception at A:A()
    terminate called after throwing an instance of 'int'        
    Aborted

    注意：如果没有在main 函数中加上 try-catch 会抛出上面的 terminate 
 */
int main(){
    try{
        A obj;
    }catch(int& e){
        std::cout<<"Exception at main func, is "<<e<<std::endl;
    }
    return 0;
}
