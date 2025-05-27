/**
 * @file task.h
 * @author vc (VchaseNi@gmail.com)
 * @brief 任务，用于定时器的任务对象，支持普通函数、成员函数、静态成员函数、lambda表达式等可调用对象
 * @version 0.1
 * @date 2025-05-24
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef __VC_TASK__
#define __VC_TASK__
#include <future>
#include <iostream>
#include <memory>
#include <type_traits>
namespace vcTimer {
class TaskBase {
public:
    virtual ~TaskBase() = default;
    virtual void execute() = 0;
};

template <typename Ret = void>
class Task : public TaskBase {
public:
    /**
     * @brief Construct a new Task object
     *
     * @tparam F: 可调用对象模板参数
     * @tparam Args: 可调用对象参数模板参数
     * @tparam typename: 检查函数返回值
     * @param isFut: 是否获取返回值
     * @param f: 可调用对象
     * @param args: 可调用对象参数
     */
    template <typename F, typename... Args,
              typename = std::enable_if_t<std::is_same_v<Ret, std::invoke_result_t<F, Args...>>, void>>
    Task(bool isFut, F &&f, Args &&...args)
        : m_cb([f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() {
              return std::apply(f, args);
          }),
          m_isFut(isFut)
    {
    }

    ~Task() override {}

    /**
     * @brief 获取异步任务的结果
     *
     * @return std::future<Ret>
     */
    std::future<Ret> getFuture()
    {
        return m_promise.get_future();
    }

    /**
     * @brief 执行定时任务
     *
     */
    void execute() override
    {
        if constexpr (std::is_same_v<Ret, void>) {
            m_cb();
            if (m_isFut) {
                m_promise.set_value(); // 如果是void类型，直接set_value
            }
        }
        else {
            Ret ret = m_cb();
            if (m_isFut) {
                m_promise.set_value(ret); // 如果有返回值，set_value
            }
        }
    }

private:
    std::function<Ret()> m_cb;   // 保存可调用对象
    bool m_isFut;                // 是否获取返回值
    std::promise<Ret> m_promise; // 用于异步任务的承诺
};

/**
 * @brief task的工厂函数
 *
 * @tparam F：可调用对象模板参数
 * @tparam Args：可调用对象参数模板参数
 * @tparam Ret：可调用对象返回值
 * @param isFut：是否获取返回值
 * @param f：可调用对象
 * @param args：可调用对象参数
 * @return std::unique_ptr<Task<Ret>>
 */
template <typename F, typename... Args, typename Ret = std::invoke_result_t<F, Args...>>
std::tuple<std::unique_ptr<Task<Ret>>, std::future<Ret>> makeTask(bool isFut, F &&f, Args &&...args)
{
    auto task = std::make_unique<Task<Ret>>(isFut, std::forward<F>(f), std::forward<Args>(args)...);
    return {std::move(task), isFut ? task->getFuture() : std::future<Ret>{}};
}
}; // namespace vcTimer
#endif
