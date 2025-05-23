#include "taskMgr.h"
namespace vcDynTimer {
TaskMgr::TaskMgr() {}

TaskMgr::~TaskMgr() { stop(); };

void TaskMgr::start()
{
    m_active.store(true, std::memory_order_release);
    m_thread = std::thread([this]() {
        while (m_active.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            execute();
        }
    });
}

void TaskMgr::stop()
{
    if (m_active.exchange(false, std::memory_order_acq_rel)) {
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
}
/**
 * @brief 执行所有定时任务
 *
 */
void TaskMgr::execute()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it = m_taskMap.begin(); it != m_taskMap.end();) {
        auto &[id, info] = *it;
        // std::cout << "erase TaskId:" << it->first << " executed!" << std::endl;
        if (info.status == TaskStatus::runing) {
            info.task->execute();
            std::cout << "execute TaskId:" << id << " executed!" << std::endl;
            if (info.mode == TaskMode::once) {
                it = m_taskMap.erase(it);
            }
            else if (info.mode == TaskMode::periodLimit) {
                
            }
            else {
                ++it;
            }
        }
        else {
            ++it;
        }
    }
}
/**
 * @brief 控制任务启动/停止/暂停
 *
 * @param id task id
 * @param control TaskControl
 */
void TaskMgr::control(TaskId id, TaskControl control)
{
    auto it = m_taskMap.find(id);
    if (it != m_taskMap.end()) {
        switch (control) {
        case TaskControl::start:
            it->second.status = TaskStatus::runing;
            break;
        case TaskControl::stop:
            m_taskMap.erase(it);
            break;
        case TaskControl::pause:
            it->second.status = TaskStatus::pausing;
            break;
        default:
            break;
        }
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
TaskId TaskMgr::getTaskId() { return ++m_taskId; }

} // namespace vcDynTimer