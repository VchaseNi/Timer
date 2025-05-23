#include "task.h"
#include "callable.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace vcDynTimer;

uint32_t g_normalFuncCnt = 0;
// 等待1000ms后的回调函数结果，使用场景：等待应答结果，因为使用future实现所以仅能使用一次，不能重复get()
TEST(task, normalFunc)
{
    g_normalFuncCnt = 0;
    auto task = makeTask(static_cast<int (*)()>(print_hello));
    std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        task->execute();
    });
    auto fut = task->getFuture();
    fut.wait();
    auto val = fut.get();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(g_normalFuncCnt, val);
}

TEST(task, normalFuncParam)
{
    g_normalFuncCnt = 0;
    auto task = makeTask(print_message_param, std::string("hello"), 1);
    task->execute();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(g_normalFuncCnt, 1);
}

TEST(task, memberFunc)
{
    MyClass obj;
    auto task = makeTask(&MyClass::member_func, &obj, "Hello");
    task->execute();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(obj.memberFuncCnt, 1);
}

TEST(task, staticFunc)
{
    auto task = makeTask(&MyClass::static_func);
    task->execute(); // 每秒调用一次静态成员函数
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(MyClass::staticFuncCnt, 1);
}

TEST(task, lambda)
{
    uint32_t lambdaCnt = 0;
    auto task = makeTask([&]() {
        std::cout << "Lambda called!" << std::endl;
        lambdaCnt++;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    task->execute();
    ASSERT_EQ(lambdaCnt, 1);
}

TEST(task, functor)
{
    Functor functor;
    auto task = makeTask(std::ref(functor));
    task->execute(); // 每秒调用一次仿函数
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(functor.functorCnt, 1);
}