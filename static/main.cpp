#include <iostream>
#include "globale.h"

int main(){
  globaleFunction();
  // 访问 globale.cpp 定义的 globalvariable 变量  是通过 extern  int globalVariable; 声明的
  std::cout<<"Global varibale: "<<globalVariable<<std::endl;

  return 0;

  /*
   编译
   g++ -c mian.cpp -o main.o
   g++ -c global.cpp -o global.o
   g++ main.o global.o -o program

   //运行
   ./program

  */

  /*
    注意如果把 global.cpp 中的 int globalVariable = 44; 修改成 static int globalVariable = 44; 再运行会报错。static 关键字该变量/函数在本文件使用，对内部可见，对外部不可见。
  
  */
}
