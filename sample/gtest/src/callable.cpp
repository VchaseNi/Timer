#include "callable.h"
// 普通函数
int print_hello()
{
    g_normalFuncCnt++;
    std::cout << "Normal Func Cnt: " << g_normalFuncCnt << std::endl;
    return g_normalFuncCnt;
}

// 带参数的函数
void print_hello(std::string str, int num)
{
    g_normalFuncCnt++;
    std::cout << "Normal Func Cnt: " << g_normalFuncCnt << " str: " << str << " num: " << num
              << std::endl;
}

// 普通函数
void print_message_param(std::string arg1, int arg2)
{
    g_normalFuncCnt++;
    std::cout << "Normal Func Cnt: " << g_normalFuncCnt << " arg1: " << arg1 << std::endl;
}


// 成员函数
void MyClass::member_func(const std::string &msg)
{
    memberFuncCnt++;
    std::cout << "Member Func " << msg << " Cnt: " << memberFuncCnt << std::endl;
}

uint32_t MyClass::staticFuncCnt = 0;
// 静态成员函数
void MyClass::static_func()
{
    staticFuncCnt++;
    std::cout << "Static function!" << std::endl;
}
