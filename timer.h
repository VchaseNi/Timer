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
#ifndef __VC_TASK_MGR__
#define __VC_TASK_MGR__
#include "task.h"
#include <map>
#include <mutex>
#include <thread>
#include <tuple>

namespace vcTimer {
// 定时器模式
enum class TimerMode {
    period = 0x1, // 周期永久
    span = 0x2,   // 周期限制
};

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
    TimerMode mode;                 // 任务模式
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
    Timer();
    ~Timer();
    /**
     * @brief 添加定时任务
     *
     * @tparam F: 可调用对象模板参数
     * @tparam Args: 可调用对象参数模板参数
     * @tparam Ret: 可调用对象返回值
     * @param mode: 定时器模式
     * @param interval: 间隔
     * @param span: 有效时间
     * @param f: 可调用对象
     * @param args: 可调用对象参数
     * @return std::tuple<TaskId, std::optional<std::future<Ret>>>
     */
    template <typename F, typename... Args, typename Ret = std::invoke_result_t<F, Args...>>
    std::tuple<TaskId, std::optional<std::future<Ret>>> addTask(TimerMode mode, int64_t interval, int64_t span, F &&f,
                                                                Args &&...args)
    {
        std::optional<std::future<Ret>> fut;
        auto task = makeTask(std::forward<F>(f), std::forward<Args>(args)...);
        auto id = getTaskId();
        if (interval == span) {
            fut = task->getFuture();
        }

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
    void control(TaskId id, TaskControl control);

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
    void execute();

    /**
     * @brief Get the Task Id object
     *
     * @return TaskId
     */
    TaskId getTaskId();

    /**
     * @brief 计算最小公倍数
     *
     * @return int64_t
     */
    int64_t gcd();

    /**
     * @brief 判断task是要执行以及是否任务完成
     *
     * @param info: task info
     * @param curStamp: 当前时间戳
     * @return std::tuple<bool, bool> first: 是否执行, second: 是否完成
     */
    std::tuple<bool, bool> isExecuteAndFinished(TaskInfo &info, int64_t curStamp);

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