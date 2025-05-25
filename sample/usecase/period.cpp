/**
 * @file period.cpp
 * @author vc (VchaseNi@gmail.com)
 * @brief 适用于周期性定时器，周期性执行任务；eg：每隔1s采集一次信号；
 *        备注：addTask的参数(mode = vcTimer::TimerMode::period, span = 0)
 * @version 0.1
 * @date 2025-05-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <cassert>
#include <iostream>
#include <vector>
#include "timer.h"

void collect(std::vector<std::string> signals) {
    for (const auto &signal : signals) {
        std::cout << "Collecting: " << signal << std::endl;
    }
}

int main()
{
    vcTimer::Timer tm;
    std::vector<std::string> signals = {"x", "y"};
    auto [id, _] = tm.addTask(vcTimer::TimerMode::period, 1000, 0, collect, signals);
    tm.control(id, vcTimer::TaskControl::start);

    // 在任务队列为空时退出，Timer析构函数会自动停止所有任务
    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}