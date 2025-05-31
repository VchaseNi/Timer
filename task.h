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
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <type_traits>

namespace vcTimer {
// 任务模式
enum class TaskMode {
    period = 0x1,       // 周期
    span = 0x2,         // 有效时段
    single = 0x3,       // 单次
    singleFuture = 0x4, // 单次Future,可获取返回值
};

class TaskBase {
public:
    virtual ~TaskBase() = default;
    virtual void execute() = 0;
};

template <typename Ret, TaskMode mode>
class Task : public TaskBase {
public:
/**
 * @brief Construct a new Task object
 *
 * @tparam F: 可调用对象模板参数
 * @tparam Args: 可调用对象参数模板参数
 * @tparam typename: 检查函数返回值
 * @param f: 可调用对象
 * @param args: 可调用对象参数
 */
#if __cplusplus >= 202002L
    template <typename F, typename... Args,
              typename = std::enable_if_t<std::is_same_v<Ret, std::invoke_result_t<F, Args...>>, void>>
    Task(F &&f, Args &&...args)
        : m_cb([f = std::forward<F>(f), ... args = std::forward<Args>(args)]() { return std::invoke(f, args...); })
    {
    }
#elif __cplusplus >= 201703L
    template <typename F, typename... Args,
              typename = std::enable_if_t<std::is_same_v<Ret, std::invoke_result_t<F, Args...>>, void>>
    Task(F &&f, Args &&...args)
        : m_cb([f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
              return std::apply(f, args); // 需要周期执行，所以不能move(args)
          })
    {
    }
#endif

    /**
     * @brief 获取异步任务的结果
     *
     * @return std::future<Ret>
     */
    std::future<Ret> getFuture() { return std::future<Ret>{}; }

    /**
     * @brief 执行定时任务
     *
     */
    void execute() override
    {
        m_cb();
    }

private:
    std::function<Ret()> m_cb;
};

template <typename Ret>
class Task<Ret, TaskMode::singleFuture> : public TaskBase {
public:
/**
 * @brief Construct a new Task object
 *
 * @tparam F: 可调用对象模板参数
 * @tparam Args: 可调用对象参数模板参数
 * @tparam typename: 检查函数返回值
 * @param f: 可调用对象
 * @param args: 可调用对象参数
 */
#if __cplusplus >= 202002L
    template <typename F, typename... Args,
              typename = std::enable_if_t<std::is_same_v<Ret, std::invoke_result_t<F, Args...>>, void>>
    Task(F &&f, Args &&...args)
        : m_cb([f = std::forward<F>(f), ... args = std::forward<Args>(args)]() { return std::invoke(f, args...); })
    {
    }
#elif __cplusplus >= 201703L
    template <typename F, typename... Args,
              typename = std::enable_if_t<std::is_same_v<Ret, std::invoke_result_t<F, Args...>>, void>>
    Task(F &&f, Args &&...args)
        : m_cb([f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
              return std::apply(f, std::move(args));
          })
    {
    }
#endif
    ~Task() override {}

    /**
     * @brief 获取异步任务的结果
     *
     * @return std::future<Ret>
     */
    std::future<Ret> getFuture() { return m_cb.get_future(); } // namespace vcTimer

    /**
     * @brief 执行定时任务
     *
     */
    void execute() override { m_cb(); }

private:
    std::packaged_task<Ret()> m_cb; // 保存可调用对象
};

/**
 * @brief task的工厂函数
 *
 * @tparam mode：定时器模式
 * @tparam F：可调用对象模板参数
 * @tparam Args：可调用对象参数模板参数
 * @tparam Ret：可调用对象返回值
 * @param f：可调用对象
 * @param args：可调用对象参数
 * @return std::unique_ptr<Task<Ret>>
 */
template <TaskMode mode, typename F, typename... Args, typename Ret = std::invoke_result_t<F, Args...>>
std::tuple<std::unique_ptr<Task<Ret, mode>>, std::future<Ret>> makeTask(F &&f, Args &&...args)
{
    auto task = std::make_unique<Task<Ret, mode>>(std::forward<F>(f), std::forward<Args>(args)...);
    return {std::move(task), task->getFuture()};
}
}; // namespace vcTimer
#endif
