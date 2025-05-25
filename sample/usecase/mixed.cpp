/**
 * @file mixed.cpp
 * @author vc (VchaseNi@gmail.com)
 * @brief 定时器混合测试
 * @version 0.1
 * @date 2025-05-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "timer.h"
#include <cassert>
#include <iostream>
#include <vector>

bool onceFunc(int requestId)
{
    if (requestId > 0) {
        return true;
    }
    else {
        return false;
    }
}

void spanFunc(std::string signal) {
    std::cout <<"Span Collecting: " << signal << std::endl;
}

void periodFunc(std::vector<std::string> signals) {
    for (const auto &signal : signals) {
        std::cout << "Collecting: " << signal << std::endl;
    }
}

int main()
{
    vcTimer::Timer tm;
    auto sp = tm.addTask(vcTimer::TimerMode::span, 100, 50 * vcTimer::TimerSecond, spanFunc, "xy");
    tm.control(std::get<0>(sp), vcTimer::TaskControl::start);



    auto pe = tm.addTask(vcTimer::TimerMode::period, 200, 0, periodFunc, std::vector<std::string>{"x", "y"});
    tm.control(std::get<0>(pe), vcTimer::TaskControl::start);

    auto [id, fut] = tm.addTask(vcTimer::TimerMode::span, 50, 50, onceFunc, 1);
    tm.control(id, vcTimer::TaskControl::start);
    assert(fut.has_value());
    assert(fut.value().get());
    // 在任务队列为空时退出，Timer析构函数会自动停止所有任务
    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}