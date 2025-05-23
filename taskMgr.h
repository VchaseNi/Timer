#ifndef __VC_TASK_MGR__
#define __VC_TASK_MGR__
#include "task.h"
#include <map>
#include <memory>
#include <mutex>
#include <tuple>
#include <vector>

namespace vcDynTimer {

enum class TaskStatus {
    notStarted = 0, // 无状态
    pausing = 1,    // 暂停
    runing = 2,     // 运行中
    finished = 3,   // 完成
};

enum class TaskControl {
    start = 0, // 启动
    stop = 1,  // 停止
    pause = 2, // 暂停
    reset = 3, // 重置
};

struct TaskInfo {
    TaskMode mode;                  // 任务模式
    int64_t interval;               // 间隔时间
    int64_t span;                   // 周期时间
    int64_t lastExecuteTime;        // 上次执行时间
    TaskStatus status;              // 任务状态
    std::unique_ptr<TaskBase> task; // 任务对象
};
using TaskId = uint32_t;

class TaskMgr {
public:
    TaskMgr();
    ~TaskMgr();

    void start();

    void stop();
    // 添加任务
    template <typename F, typename... Args, typename Ret = std::invoke_result_t<F, Args...>>
    std::pair<TaskId, std::future<Ret>> addTask(TaskMode mode, int64_t interval, int64_t span,
                                                F &&f, Args &&...args)
    {
        auto task = makeTask(std::forward<F>(f), std::forward<Args>(args)...);
        auto id = getTaskId();
        auto fut = task->getFuture();

        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskMap.emplace(id,
                          TaskInfo{mode, interval, span, TaskStatus::notStarted, std::move(task)});
        return {id, std::move(fut)};
    }

    /**
     * @brief 控制任务启动/停止/暂停
     *
     * @param id task id
     * @param control TaskControl
     */
    void control(TaskId id, TaskControl control);

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

private:
    std::atomic<bool> m_active{false}; // 任务管理器是否处于活动状态
    std::mutex m_mutex;                // 互斥锁
    std::map<TaskId, TaskInfo> m_taskMap;

    std::atomic<TaskId> m_taskId{0};
    std::thread m_thread;
};
}; // namespace vcDynTimer
#endif