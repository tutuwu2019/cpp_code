# note

## 流

> 文件操作

```text
template<typename _CharT, typename _Traits = char_traits<_CharT> >

    class basic_ios;


```

C++ I / O 体系结构

std::basic_ios、std::ios、std::wios

|:--:| :--:| :--:| :--: | 
类 / 别名 | 模板参数 _CharT | 用途 | 常见派生类
basic_ios | 模板参数 | 流状态、格式化、缓冲区连接 | "basic_istream, basic_ostream"
ios | char | 窄字符流 (字节流) 基础 | "istream, ostream, fstream, stringstream"
wios | wchar_t | 宽字符流 (Unicode) 基础 | "wistream, wostream, wfstream, wstringstream"
