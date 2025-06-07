#include <iostream>
#include <gtest/gtest.h>
#include <thread>
#include "callable.h"
#include "task.h"

using namespace vcTimer;
// 等待1000ms后的回调函数结果，使用场景：等待应答结果，因为使用future实现所以仅能使用一次，不能重复get()
TEST(task, normalFunc)
{
    g_normalFuncCnt = 0;
    auto [task, fut] = makeTask<TaskMode::singleFuture>(static_cast<int (*)()>(print_hello));
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
    auto [task, _] = makeTask<TaskMode::single>(print_message_param, "hello", 1);
    task->execute();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(g_normalParamFuncCnt, 1);
}

TEST(task, memberFunc)
{
    MyClass obj;
    auto [task, fut] = makeTask<TaskMode::singleFuture>(&MyClass::member_func, &obj, "Hello");
    task->execute();

    auto [task1, _] = makeTask<TaskMode::single>(&MyClass::member_func, &obj, "World");
    task1->execute();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(obj.memberFuncCnt, 2);
}

TEST(task, staticFunc)
{
    MyClass::staticFuncCnt = 0;
    auto [task, _] = makeTask<TaskMode::single>(&MyClass::static_func);
    task->execute();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(MyClass::staticFuncCnt, 1);
}

TEST(task, lambda)
{
    uint32_t lambdaCnt = 0;
    auto [task, _] = makeTask<TaskMode::singleFuture>([&]() {
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
    auto [task, _] = makeTask<TaskMode::single>(std::ref(functor));
    task->execute();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(functor.functorCnt, 1);
}