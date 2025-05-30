// #include <memory>
#include "timer.h"
#include <iostream>
#include <numeric>
namespace vcTimer {
Timer::Timer() : m_active(true)
{
    m_thread = std::thread([this]() {
        while (m_active.load(std::memory_order_acquire)) {
            execute();
            std::this_thread::sleep_for(std::chrono::milliseconds(m_gcd));
        }
    });
}

Timer::~Timer()
{
    if (m_active.exchange(false, std::memory_order_acq_rel)) {
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
};

/**
 * @brief 执行所有定时任务
 *
 */
void Timer::execute()
{
    bool isFinished = false;
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it = m_taskMap.begin(); it != m_taskMap.end();) {
        auto &[id, info] = *it;
        if (info.status == TaskStatus::running) {
            auto [isEx, isFin] = isExecuteAndFinished(info, std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                std::chrono::system_clock::now().time_since_epoch())
                                                                .count());
            // std::cout << "isEx: " << isEx << " isFin: " << isFin << std::endl;
            if (isEx) {
                info.task->execute();
            }
            if (isFin) {
                it = m_taskMap.erase(it);
                isFinished = true;
            }
            else {
                it++;
            }
        }
        else {
            ++it;
        }
    }
    if (isFinished) {
        m_gcd = gcd(); // 因task列表变更，重新计算最大公约数
    }
}
/**
 * @brief 控制任务启动/停止/暂停
 *
 * @param id task id
 * @param control TaskControl
 */
void Timer::control(TaskId id, TaskControl control)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_taskMap.find(id);
    if (it != m_taskMap.end()) {
        switch (control) {
        case TaskControl::start: {
            it->second.status = TaskStatus::running;
            it->second.startTime =
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                    .count();
        } break;
        case TaskControl::stop:
            m_taskMap.erase(it);
            break;
        default:
            break;
        }
        m_gcd = gcd(); // 因task列表变更，重新计算最大公约数
    }
    else {
        std::cerr << "Task not found!" << std::endl;
    }
}
/**
 * @brief Get the Task Id object
 *
 * @return TaskId
 */
TaskId Timer::getTaskId() { return ++m_taskId; }

/**
 * @brief 计算最大公约数
 *
 * @return int64_t
 */
int64_t Timer::gcd()
{
    bool isFirst = false;
    int64_t value = 0;
    for (const auto &[id, info] : m_taskMap) {
        if (info.status == TaskStatus::running) {
            if (!isFirst) {
                isFirst = true;
                value = info.interval;
            }
            else {
                value = std::gcd(value, info.interval);
            }
        }
    }
    return (value == 0 || value > TimerGcd) ? TimerGcd : value;
}
/**
 * @brief 判断task是要执行以及是否任务完成
 *
 * @param info: task info
 * @param curStamp: 当前时间戳
 * @return std::tuple<bool, bool> first: 是否执行, second: 是否完成
 */
std::tuple<bool, bool> Timer::isExecuteAndFinished(TaskInfo &info, int64_t curStamp)
{
    bool isEx = false;
    bool isFin = false;
    /* std::cout << "info.startTime: " << info.startTime << " info.lastExecuteTime: " << info.lastExecuteTime
              << " info.interval: " << info.interval << " span: " << info.span << " curStamp: " << curStamp << std::endl;
     */
    // 第一次执行
    if (info.lastExecuteTime == 0 && curStamp - info.startTime >= info.interval) {
        info.lastExecuteTime = curStamp;
        info.startTime = curStamp - info.interval; // 矫正因其他Timer导致的误差
        isEx = true;
        if (TimerMode::single == info.mode || TimerMode::singleFuture == info.mode ||
            (TimerMode::span == info.mode && curStamp - info.startTime >= info.span)) {
            isFin = true;
        }
    }
    else if (info.lastExecuteTime != 0) {
        if (TimerMode::span == info.mode && curStamp - info.startTime >= info.span) {
            isFin = true;
        }

        if (curStamp - info.lastExecuteTime >= info.interval) {
            info.lastExecuteTime = curStamp;
            isEx = true;
        }
    }

    return {isEx, isFin};
}
} // namespace vcTimer