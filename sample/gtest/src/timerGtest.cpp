#include <iostream>
#include <gtest/gtest.h>
#include "callable.h"
#include "timer.h"

using namespace vcTimer;
TEST(timer, normalFunc)
{
    g_normalFuncCnt = 0;
    Timer tm;
    auto [id, fut] = tm.addTask<TaskMode::singleFuture>(1000, 1000, print_hello);
    tm.control(id, TaskControl::start);
    ASSERT_TRUE(fut.valid());
    ASSERT_EQ(1, fut.get());

    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    Timer tm1;
    auto [id1, fut1] = tm1.addTask<TaskMode::span>(500, 1000, print_hello);
    tm1.control(id1, TaskControl::start);
    ASSERT_FALSE(fut1.valid());
    while (!tm1.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    ASSERT_EQ(3, g_normalFuncCnt);
}

TEST(timer, normalParamFunc)
{
    Timer tm;
    g_normalParamFuncCnt = 0;
    auto [id, _] = tm.addTask<TaskMode::span>(500, 2 * TimerSecond, print_message_param, "hello", 1);
    tm.control(id, TaskControl::start);

    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    ASSERT_EQ(g_normalParamFuncCnt, 4);

    Timer tm1;
    auto [id1, fut] = tm1.addTask<TaskMode::single>(500, 500, print_message_param, "hello", 1);
    tm1.control(id1, TaskControl::start);

    ASSERT_FALSE(fut.valid());
    while (!tm1.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    ASSERT_EQ(g_normalParamFuncCnt, 5);
}

TEST(timer, memberFunc)
{
    Timer tm;
    MyClass obj;
    auto [id, fut] = tm.addTask<TaskMode::singleFuture>(500, 500, &MyClass::member_func, &obj, "Hello");
    tm.control(id, TaskControl::start);

    fut.get();  // 等待任务完成
    ASSERT_EQ(obj.memberFuncCnt, 1);

    auto t = tm.addTask<TaskMode::span>(1000, 5 * TimerSecond, &MyClass::member_func, &obj, "Hello");
    tm.control(std::get<0>(t), TaskControl::start);

    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    ASSERT_EQ(obj.memberFuncCnt, 6);
}

TEST(timer, staticFunc)
{
    Timer tm;
    MyClass::staticFuncCnt = 0;
    auto [id, _] = tm.addTask<TaskMode::span>(500, 2 * TimerSecond, &MyClass::static_func);
    tm.control(id, TaskControl::start);

    auto [id1, future] = tm.addTask<TaskMode::span>(800, 2 * TimerSecond, &MyClass::static_func);
    tm.control(id1, TaskControl::start);

    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    ASSERT_EQ(MyClass::staticFuncCnt, 6);
}

TEST(timer, lambda)
{
    uint32_t lambdaCnt = 0;
    Timer tm;
    auto [id, fut] = tm.addTask<TaskMode::span>(500, 2 * TimerSecond, [&]() {
        std::cout << "Lambda called!" << std::endl;
        lambdaCnt++;
    });
    tm.control(id, TaskControl::start);

    auto t = tm.addTask<TaskMode::span>(1500, 3 * TimerSecond, [&]() {
        std::cout << "Lambda called!" << std::endl;
        lambdaCnt++;
    });
    tm.control(std::get<0>(t), TaskControl::start);

    auto tp = tm.addTask<TaskMode::period>(500, 0, [&]() {
        std::cout << "Lambda period called!" << std::endl;
    });
    tm.control(std::get<0>(tp), TaskControl::start);

    uint32_t cnt = 0;
    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (++cnt == 6) {
            tm.control(std::get<0>(tp), TaskControl::stop);
        }
    }

    ASSERT_EQ(lambdaCnt, 6);
}

TEST(timer, functor)
{
    Functor functor;
    Timer tm;
    auto [id, _] = tm.addTask<TaskMode::span>(50, 200, std::ref(functor));
    tm.control(id, TaskControl::start);

    auto t = tm.addTask<TaskMode::singleFuture>(100, 100, std::ref(functor));
    tm.control(std::get<0>(t), TaskControl::start);
    std::get<1>(t).get();

    auto tp = tm.addTask<TaskMode::period>(200, 0, std::ref(functor));
    tm.control(std::get<0>(tp), TaskControl::start);

    uint32_t cnt = 0;
    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cnt++;
        if (cnt == 12) {
            break;
        }
    }
    ASSERT_EQ(functor.functorCnt, 10);
}

TEST(timer, mixed)
{
    Timer tm;
    MyClass obj;
    auto [id, fut] = tm.addTask<TaskMode::singleFuture>(500, 500, &MyClass::member_func, &obj, "Hello1");
    tm.control(id, TaskControl::start);

    fut.get();  // 等待任务完成
    ASSERT_EQ(obj.memberFuncCnt, 1);

    auto t1 = tm.addTask<TaskMode::span>(1000, 5 * TimerSecond, &MyClass::member_func, &obj, "Hello2");
    tm.control(std::get<0>(t1), TaskControl::start);

    auto t2 = tm.addTask<TaskMode::period>(100, 0, &MyClass::member_func, &obj, "Hello3");
    tm.control(std::get<0>(t2), TaskControl::start);

    Functor functor;
    auto t3 = tm.addTask<TaskMode::period>(200, 0, std::ref(functor));
    tm.control(std::get<0>(t3), TaskControl::start);

    uint32_t cnt = 0;
    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        cnt++;
        if (cnt == 10) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            break;
        }
    }
}
