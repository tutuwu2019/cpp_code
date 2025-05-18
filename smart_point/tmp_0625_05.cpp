#include <iostream>
#include <memory>
#include <unistd.h> // 包含sleep函数 

class Socket
{
public:
    Socket()
    {
        std::cout<<"construct class Socket"<<std::endl;
    }

    ~Socket()
    {
        std::cout<<"destruct class Socket"<<std::endl;
    }

    //关闭资源句柄
    void close()
    {
        std::cout<<"close the sock"<<std::endl;
    }
};

/**
 *  test start
    test three
    construct class Socket
    test end
    test one
    close the sock
    test two
    destruct class Socket
 */
int main()
{
    std::cout<<"test start"<<std::endl;
    auto deletor = [](Socket* pSocket) {
        //关闭句柄
        std::cout<<"test one"<<std::endl;
        pSocket->close();
        //TODO: 你甚至可以在这里打印一行日志...
        std::cout<<"test two"<<std::endl;
        delete pSocket;
    };
    std::cout<<"test three"<<std::endl;
    //其中 T 是你要释放的对象类型，DeletorPtr 是一个自定义函数指针。
    //std::unique_ptr<Socket, void(*)(Socket * pSocket)> spSocket(new Socket(), deletor);
    std::unique_ptr<Socket, decltype(deletor)> spSocket(new Socket(), deletor);
    sleep(1);
    std::cout<<"test end"<<std::endl;
    sleep(1);
    return 0;
}
