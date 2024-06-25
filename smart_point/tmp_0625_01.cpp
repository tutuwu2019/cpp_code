#include <iostream>
#include <memory>

/**
        g++ tmp_0625_01.cpp -o tmp_0625_01 -g 
        tmp_0625_01.cpp: In function ‘int main()’:
        tmp_0625_01.cpp:5:10: warning: ‘template<class> class std::auto_ptr’ is deprecated [-Wdeprecated-declarations]
            std::auto_ptr<int> sp1(new int(8));
                ^~~~~~~~
        In file included from /usr/include/c++/8/memory:80,
                        from tmp_0625_01.cpp:2:
        /usr/include/c++/8/bits/unique_ptr.h:53:28: note: declared here
        template<typename> class auto_ptr;
                                    ^~~~~~~~
        tmp_0625_01.cpp:8:10: warning: ‘template<class> class std::auto_ptr’ is deprecated [-Wdeprecated-declarations]
            std::auto_ptr<int> sp2(sp1);
                ^~~~~~~~
        In file included from /usr/include/c++/8/memory:80,
                        from tmp_0625_01.cpp:2:
        /usr/include/c++/8/bits/unique_ptr.h:53:28: note: declared here
        template<typename> class auto_ptr;

 * 
          main.cpp: In function ‘int main()’:
        main.cpp:5:10: warning: ‘template<class> class std::auto_ptr’ is deprecated: use 'std::unique_ptr' instead [-Wdeprecated-declarations]
            5 |     std::auto_ptr<int> sp1(new int(8));
            |          ^~~~~~~~
        In file included from /usr/include/c++/11/memory:76,
                        from main.cpp:2:
        /usr/include/c++/11/bits/unique_ptr.h:57:28: note: declared here
        57 |   template<typename> class auto_ptr;
            |                            ^~~~~~~~
        main.cpp:8:10: warning: ‘template<class> class std::auto_ptr’ is deprecated: use 'std::unique_ptr' instead [-Wdeprecated-declarations]
            8 |     std::auto_ptr<int> sp2(sp1);
            |          ^~~~~~~~
        In file included from /usr/include/c++/11/memory:76,
                        from main.cpp:2:
        /usr/include/c++/11/bits/unique_ptr.h:57:28: note: declared here
        57 |   template<typename> class auto_ptr;
            |                            ^~~~~~~~
        the sp1 sizeof is 8 the val is 8
        the sp1 is nullptr
        the sp2 is 8 or *(sp2.get()): 8

    注意c++1 已经抛弃了 auto_ptr，它的复制语义 有问题，很明显的在容器中对元素进行复制操作，会对原始元素置空，这是很严重的一个问题
 * 
 */

/**
 *  通过 get 获取本身地址
 *  
 */
int main(){
    std::auto_ptr<int> sp1(new int(8));
    std::cout<<"the sp1 sizeof is "<<sizeof(sp1)<<" the val is "<<*sp1<<std::endl;

    std::auto_ptr<int> sp2(sp1);

    if(sp1.get() == nullptr){
        std::cout<<"the sp1 is nullptr"<<std::endl;
    }else{
        std::cout<<"the sp1 is not nullptr"<<std::endl;
    }
    std::cout<<"the sp2 is "<<*sp2<<" or *(sp2.get()): "<<*(sp2.get())<<std::endl;

    return 0;
}
