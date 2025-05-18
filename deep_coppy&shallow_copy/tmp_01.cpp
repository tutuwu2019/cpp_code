#include <iostream>
#include <cstring>
#include <stdlib.h>

class String{
public:
    String(const char* pStr = ""){
        std::cout<<"construct start "<<std::endl;
        if(const_cast<const char*>(pStr) == ""){
            _pStr = new char[1];
            *_pStr = '\0';
        }else{
            //注意 是在构造函数 new 开辟内存空间
            _pStr = new char[strlen(pStr) + 1];
            strcpy(_pStr, pStr);
        }
    }

    String(const String& s):_pStr(new char[strlen(s._pStr) + 1]){
        strcpy(_pStr, s._pStr);
        std::cout<<"String(const String& s) func"<<std::endl;
    }

    /**
     *  注意要清空旧内存，避免旧内存没有清空而导致内存泄漏
     */
    String& operator=(const String& s){
        std::cout<<"operator= func"<<std::endl;
        if(this != &s){
            char* tmp = new char[strlen(s._pStr) + 1];
            strcpy(_pStr, s._pStr);
            delete[] _pStr;
            _pStr = tmp;
        }
        return *this;
    }

    ~String(){
        //注意 打印 this->_pStr 就是完整的字符串，而 *(this->_pStr)就是字符串的第一个元素 h
        std::cout<<"destruct the str "<<(this->_pStr)<<std::endl;
        if(_pStr){
            //注意 是在这里进行 _pStr 内存释放
            delete[] _pStr;
            _pStr = nullptr;
        }
    }
private:
    char* _pStr;
};

int main(){
    //String s1;
    String s2("hello, world");
    String s3(s2);
    //s1[0] = '5';
    String s4;
    s4 = s2;

    return 0;
}
