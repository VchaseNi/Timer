/**
 * @file timer.h
 * @author vc (VchaseNi@gmail.com)
 * @brief ms定时器，用于管理定时器任务，支持多任务；支持周期任务和单次任务；支持任务的启动、停止操作
 *        建议：
 *          1. 一个timer可以管理多个任务，但不要过多，避免影响性能和精度；
 *          2. 可调用对象中禁止长期占用定时器，只能实现简单逻辑，如业务复杂建议使用队列或信号来唤醒另一个线程处理；
 *          3. 在一个定时器中，所有任务的频率间隔时间应该是比较接近的，过大会导致无效判断任务是否可以执行，也会使
 *             频率低的任务第一次加入到定时器中不会很快的响应；
 *             (eg: 上个任务10s，当前加入100ms任务，那么当前可能会在10s后才执行)；
 *          4. 如对时间精度敏感的任务建议使用单独的定时器来管理；
 * @version 0.1
 * @date 2025-05-24
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef __VC_TIMER__
#define __VC_TIMER__
#include "task.h"
#include <map>
#include <mutex>
#include <thread>
#include <tuple>
#include <numeric>

namespace vcTimer {

// 任务状态
enum class TaskStatus {
    notStarted = 0, // 无状态
    pausing = 1,    // 暂停
    running = 2,    // 运行中
    finished = 3,   // 完成
};

// 任务控制
enum class TaskControl {
    start = 0, // 启动
    stop = 1,  // 停止
};

struct TaskInfo {
    TaskMode mode;                  // 任务模式
    int64_t interval;               // 间隔时间
    int64_t span;                   // 周期时间
    int64_t lastExecuteTime;        // 上次执行时间
    int64_t firstExecuteTime;       // 第一次执行时间
    int64_t startTime;              // 启动时间
    TaskStatus status;              // 任务状态
    std::unique_ptr<TaskBase> task; // 任务对象
};
using TaskId = uint32_t;
using TimerUnit = std::chrono::milliseconds;

const int64_t TimerSecond = 1000;
const int64_t TimerGcd = 1000;

class Timer {
public:
    Timer() : m_active(true)
    {
        m_thread = std::thread([this]() {
            while (m_active.load(std::memory_order_acquire)) {
                execute();
                std::this_thread::sleep_for(std::chrono::milliseconds(m_gcd));
            }
        });
    }

    ~Timer()
    {
        if (m_active.exchange(false, std::memory_order_acq_rel)) {
            if (m_thread.joinable()) {
                m_thread.join();
            }
        }
    };
    /**
     * @brief 添加定时任务
     *
     * @tparam F: 可调用对象模板参数
     * @tparam Args: 可调用对象参数模板参数
     * @tparam Ret: 可调用对象返回值
     * @param mode: 定时器模式
     * @param isFut: 是否获取返回值
     * @param interval: 间隔
     * @param span: 有效时间
     * @param f: 可调用对象
     * @param args: 可调用对象参数
     * @return std::tuple<TaskId, std::optional<std::future<Ret>>>
     */
    template <TaskMode mode, typename F, typename... Args, typename Ret = std::invoke_result_t<F, Args...>>
    std::tuple<TaskId, std::future<Ret>> addTask(int64_t interval, int64_t span, F &&f, Args &&...args)
    {
        auto [task, fut] = makeTask<mode>(std::forward<F>(f), std::forward<Args>(args)...);
        auto id = getTaskId();

        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskMap.emplace(id, TaskInfo{mode, interval, span, 0, 0, 0, TaskStatus::notStarted, std::move(task)});
        return {id, std::move(fut)};
    }

    /**
     * @brief 控制任务启动/停止/暂停
     *
     * @param id task id
     * @param control TaskControl
     */
    void control(TaskId id, TaskControl control)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_taskMap.find(id);
        if (it != m_taskMap.end()) {
            switch (control) {
            case TaskControl::start: {
                it->second.status = TaskStatus::running;
                it->second.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                                           std::chrono::system_clock::now().time_since_epoch())
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
     * @brief 任务是否为空
     *
     * @return true
     * @return false
     */
    bool isTaskEmpty()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_taskMap.empty();
    }

private:
    /**
     * @brief 执行所有定时任务
     *
     */
    void execute()
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
     * @brief Get the Task Id object
     *
     * @return TaskId
     */
    TaskId getTaskId() { return ++m_taskId; };

    /**
     * @brief 计算最小公倍数
     *
     * @return int64_t
     */
    int64_t gcd()
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
    };

    /**
     * @brief 判断task是要执行以及是否任务完成
     *
     * @param info: task info
     * @param curStamp: 当前时间戳
     * @return std::tuple<bool, bool> first: 是否执行, second: 是否完成
     */
    std::tuple<bool, bool> isExecuteAndFinished(TaskInfo &info, int64_t curStamp)
    {
        bool isEx = false;
        bool isFin = false;
        /* std::cout << "info.startTime: " << info.startTime << " info.lastExecuteTime: " << info.lastExecuteTime
                  << " info.interval: " << info.interval << " span: " << info.span << " curStamp: " << curStamp <<
           std::endl;
         */
        // 第一次执行
        if (info.lastExecuteTime == 0 && curStamp - info.startTime >= info.interval) {
            info.lastExecuteTime = curStamp;
            info.startTime = curStamp - info.interval; // 矫正因其他Timer导致的误差
            isEx = true;
            if (TaskMode::single == info.mode || TaskMode::singleFuture == info.mode ||
                (TaskMode::span == info.mode && curStamp - info.startTime >= info.span)) {
                isFin = true;
            }
        }
        else if (info.lastExecuteTime != 0) {
            if (TaskMode::span == info.mode && curStamp - info.startTime >= info.span) {
                isFin = true;
            }

            if (curStamp - info.lastExecuteTime >= info.interval) {
                info.lastExecuteTime = curStamp;
                isEx = true;
            }
        }

        return {isEx, isFin};
    };

private:
    std::atomic<bool> m_active{false};    // 任务管理器是否处于活动状态
    int64_t m_gcd{TimerGcd};              // 最小公倍数
    std::mutex m_mutex;                   // 互斥锁
    std::map<TaskId, TaskInfo> m_taskMap; // 任务列表
    std::atomic<TaskId> m_taskId{0};      // 递增的任务ID
    std::thread m_thread;
};
}; // namespace vcTimer
#endif