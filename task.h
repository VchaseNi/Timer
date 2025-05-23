#ifndef __VC_TASK__
#define __VC_TASK__
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <thread>
#include <type_traits>

namespace vcDynTimer {
enum class TaskMode {
    once = 0,             // 仅一次
    periodForever = 0x10, // 周期永久
    periodLimit = 0x11,   // 周期限制
};

class TaskBase {
public:
    virtual ~TaskBase() = default;
    virtual void execute() = 0;
};

template <typename Ret = void>
class Task : public TaskBase {
public:
    template <
        typename F, typename... Args,
        typename = std::enable_if_t<std::is_same_v<Ret, std::invoke_result_t<F, Args...>>, void>>
    Task(F &&f, Args &&...args)
        : m_cb([f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() {
              return std::apply(f, args);
          })
    {
    }

    ~Task() override { std::cout << "Task destructor!" << std::endl; }

    std::future<Ret> getFuture() { return m_promise.get_future(); }

    // 执行任务
    void execute() override
    {
        if constexpr (std::is_same_v<Ret, void>) {
            m_cb();
        }
        else {
            m_promise.set_value(m_cb());
        }
    }

private:
    std::function<Ret()> m_cb;
    std::promise<Ret> m_promise; // 用于异步任务的承诺
};

template <typename F, typename... Args, typename Ret = std::invoke_result_t<F, Args...>>
std::unique_ptr<Task<Ret>> makeTask(F &&f, Args &&...args)
{
    return std::make_unique<Task<Ret>>(std::forward<F>(f), std::forward<Args>(args)...);
}
}; // namespace vcDynTimer
#endif
