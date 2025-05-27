#include <iostream>
#include <gtest/gtest.h>
#include "callable.h"
#include "task.h"

using namespace vcTimer;
// 等待1000ms后的回调函数结果，使用场景：等待应答结果，因为使用future实现所以仅能使用一次，不能重复get()
TEST(task, normalFunc)
{
    g_normalFuncCnt = 0;
    auto [task, fut] = makeTask(true, static_cast<int (*)()>(print_hello));
    auto _ = std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        task->execute();
    });
    fut.wait();
    auto val = fut.get();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(g_normalFuncCnt, val);
}

TEST(task, normalFuncParam)
{
    g_normalParamFuncCnt = 0;
    auto [task, _] = makeTask(false, print_message_param, std::string("hello"), 1);
    task->execute();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(g_normalParamFuncCnt, 1);
}

TEST(task, memberFunc)
{
    MyClass obj;
    auto [task, _] = makeTask(true, &MyClass::member_func, &obj, "Hello");
    task->execute();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(obj.memberFuncCnt, 1);
}

TEST(task, staticFunc)
{
    MyClass::staticFuncCnt = 0;
    auto [task, _] = makeTask(false, &MyClass::static_func);
    task->execute();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(MyClass::staticFuncCnt, 1);
}

TEST(task, lambda)
{
    uint32_t lambdaCnt = 0;
    auto [task, _] = makeTask(true, [&]() {
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
    auto [task, _] = makeTask(false, std::ref(functor));
    task->execute();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(functor.functorCnt, 1);
}
