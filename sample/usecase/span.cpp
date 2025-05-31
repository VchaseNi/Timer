/**
 * @file span.cpp
 * @author vc (VchaseNi@gmail.com)
 * @brief 适用于以一定周期频率定时触发，并持续span时间的场景；eg：因某事件发生，以100ms的频率采集信号，持续500ms；
 *        备注：addTask的参数(mode = vcTimer::TaskMode::span, span是interval的整数倍)
 * @version 0.1
 * @date 2025-05-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <cassert>
#include <iostream>
#include "timer.h"

void collect(std::string signal) {
    std::cout <<"Collecting: " << signal << std::endl;
}

int main()
{
    vcTimer::Timer tm;
    auto [id, _] = tm.addTask<vcTimer::TaskMode::span>(500, 5000, collect, "Signal xy");
    tm.control(id, vcTimer::TaskControl::start);

    // 在任务队列为空时退出，Timer析构函数会自动停止所有任务
    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}