//
// Created by Gxin on 2022/5/12.
//

#ifndef GX_TASK_SYSTEM_H
#define GX_TASK_SYSTEM_H

#include "gobject.h"

#include "gthread.h"
#include "gmutex.h"
#include "gtimer.h"

#include <atomic>
#include <future>
#include <functional>
#include <list>
#include <vector>


/**
 * @class GTaskSystem
 * @brief Multithreaded task system (thread pool)
 */
class GX_API GTaskSystem final : public GObject
{
public:
    template<class T>
    class Task
    {
    public:
        Task(std::future<T> &&future, const std::shared_ptr<std::atomic<bool> > &active)
            : mFuture(std::move(future)), mActive(active)
        {
        }

    public:
        explicit Task() = default;

        Task(const Task &other) = delete;

        Task(Task &&other) noexcept
            : mFuture(std::move(other.mFuture)),
              mActive(std::move(other.mActive))
        {
        }

        ~Task() = default;

        Task &operator=(const Task &other) = delete;

        Task &operator=(Task &&other) noexcept
        {
            if (this != &other) {
                std::swap(mFuture, other.mFuture);
                std::swap(mActive, other.mActive);
            }
            return *this;
        }

    public:
        T get()
        {
            return mFuture.get();
        }

        void wait()
        {
            if (!isValid()) {
                return;
            }
            mFuture.wait();
        }

        bool waitFor(int64_t ms)
        {
            if (!isValid()) {
                return false;
            }
            auto status = mFuture.wait_for(std::chrono::milliseconds(ms));
            return status != std::future_status::timeout;
        }

        bool isCompleted()
        {
            return waitFor(0);
        }

        void cancel()
        {
            if (mActive) {
                mActive->store(false, std::memory_order_release);
            }
        }

        bool isValid() const
        {
            if (!mFuture.valid()) {
                return false;
            }
            if (mActive) {
                return mActive->load(std::memory_order_relaxed);
            }
            return false;
        }

    private:
        std::future<T> mFuture;
        std::shared_ptr<std::atomic<bool> > mActive;
    };

public:
    /**
     * Construct Task System
     *
     * @param name
     * @param threadCount    Number of threads, when using the default value of 0, the number of allocated threads is the number of CPU cores
     */
    explicit GTaskSystem(std::string name = "TaskSystem", uint32_t threadCount = 0);

    ~GTaskSystem() override;

    GTaskSystem(const GTaskSystem &) = delete;

    GTaskSystem(GTaskSystem &&) noexcept = delete;

    GTaskSystem &operator=(const GTaskSystem &) = delete;

    GTaskSystem &operator=(GTaskSystem &&) noexcept = delete;

public:
    uint32_t threadCount() const;

    /**
     * @brief Start Task System
     */
    void start();

    /**
     * @brief Stop and wait for all tasks to end
     */
    void stopAndWait();

    /**
     * @brief Stop, do not wait for the task to end, all unfinished tasks will be cancelled
     */
    void stop();

    bool isRunning() const;

    void setThreadPriority(ThreadPriority priority);

    ThreadPriority getThreadPriority() const;

    template<typename F, typename... A,
             typename = std::enable_if_t<std::is_void_v<std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...> > > >
    Task<bool> submit(const F &taskFunc, const A &&... args)
    {
        auto promise = std::make_shared<std::promise<bool> >();
        auto future = promise->get_future();
        auto taskRef = pushTask(
            [taskFunc, args..., promise] {
                taskFunc(args...);
                promise->set_value(true);
            });
        return {std::move(future), taskRef.active};
    }

    template<typename F, typename... A,
             typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>,
             typename = std::enable_if_t<!std::is_void_v<R>> >
    Task<R> submit(const F &taskFunc, const A &&... args)
    {
        auto promise = std::make_shared<std::promise<R> >();
        auto future = promise->get_future();
        auto taskRef = pushTask([taskFunc, args..., promise] { promise->set_value(taskFunc(args...)); });
        return {std::move(future), taskRef.active};
    }

    template<typename F, typename... A,
             typename = std::enable_if_t<std::is_void_v<std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...> > > >
    Task<bool> submitFront(const F &taskFunc, const A &&... args)
    {
        auto promise = std::make_shared<std::promise<bool> >();
        auto future = promise->get_future();
        auto taskRef = pushTaskFront(
            [taskFunc, args..., promise] {
                taskFunc(args...);
                promise->set_value(true);
            });
        return {std::move(future), taskRef.active};
    }

    template<typename F, typename... A,
             typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>,
             typename = std::enable_if_t<!std::is_void_v<R>> >
    Task<R> submitFront(const F &taskFunc, const A &&... args)
    {
        auto promise = std::make_shared<std::promise<R> >();
        auto future = promise->get_future();
        auto taskRef = pushTaskFront([taskFunc, args..., promise] { promise->set_value(taskFunc(args...)); });
        return {std::move(future), taskRef.active};
    }

    uint64_t waitingTaskCount() const;

private:
    using TaskFunc = std::function<void()>;

    struct TaskFuncRef
    {
        TaskFunc func;
        std::shared_ptr<std::atomic<bool> > active;
    };

    TaskFuncRef pushTask(const TaskFunc &task);

    TaskFuncRef pushTaskFront(const TaskFunc &task);

    void clearTask();

private:
    std::string mName;

    uint32_t mThreadCount;
    ThreadPriority mPriority = ThreadPriority::Normal;

    std::vector<std::unique_ptr<GThread> > mThreads;
    std::list<TaskFuncRef> mTaskQueue;

    mutable GMutex mLock;
    std::condition_variable mTaskCond;
    std::atomic<bool> mIsRunning{false};
};

#endif //GX_TASK_SYSTEM_H
