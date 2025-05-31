/**
 * @file once.cpp
 * @author vc (VchaseNi@gmail.com)
 * @brief 适用于请求响应的定时器，在发送请求后，等待响应超时的场景, 仅执行一次，并且支持等待可调用对象返回值；
 *        备注：addTask的参数(mode = vcTimer::TaskMode::span, interval == span)
 * @version 0.1
 * @date 2025-05-24
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "timer.h"
#include <cassert>

bool recieveResp(int requestId)
{
    if (requestId > 0) {
        return true;
    }
    else {
        return false;
    }
}

int main()
{
    vcTimer::Timer tm;
    auto [id, fut] = tm.addTask<vcTimer::TaskMode::singleFuture>(50, 50, recieveResp, 1);
    tm.control(id, vcTimer::TaskControl::start);

    assert(fut.valid());
    assert(fut.get());

    auto t = tm.addTask<vcTimer::TaskMode::singleFuture>(100, 100, recieveResp, 0);
    tm.control(std::get<0>(t), vcTimer::TaskControl::start);
    assert(!std::get<1>(t).get());

    // 在任务队列为空时退出，Timer析构函数会自动停止所有任务
    while (!tm.isTaskEmpty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}