//
// Created by Gxin on 2023/3/7.
//

#ifndef GX_GTIMER_H
#define GX_GTIMER_H

#include "gobject.h"

#include "gtime.h"
#include "gmutex.h"

#include <functional>
#include <queue>
#include <vector>


using GTimerEvent = std::function<void()>;
using GTimerCondition = std::function<bool()>;

/**
 * @class GTimerTask
 */
class GX_API GTimerTask
{
private:
    explicit GTimerTask(GTimerEvent event, GTimerCondition condition, int64_t delay, int64_t interval);

public:
    void cancel();

private:
    friend class GTimer;

    friend class GTimerScheduler;

    GTimerEvent mEvent;
    GTimerCondition mCondition;
    int64_t mInterval;
    GTime mTime;
    std::atomic_bool mValid;
    bool mOneShot{false};
};

class GX_API GTimerScheduler
{
public:
    using GTimerTaskPtr = std::shared_ptr<GTimerTask>;

private:
    explicit GTimerScheduler(std::string name);

public:
    static std::shared_ptr<GTimerScheduler> create(std::string name);

    static void makeGlobal(const std::shared_ptr<GTimerScheduler> &obj);

    static std::shared_ptr<GTimerScheduler> global();

    ~GTimerScheduler();

    GTimerScheduler(const GTimerScheduler &) = delete;

    GTimerScheduler(GTimerScheduler &&) noexcept = delete;

    GTimerScheduler &operator=(const GTimerScheduler &) = delete;

    GTimerScheduler &operator=(GTimerScheduler &&) noexcept = delete;

public:
    bool run();

    int64_t loop(int64_t maxTime = 10);

    /**
     * @brief Before using the loop() driver, you must first call start()
     */
    void start();

    void stop();

    bool isRunning() const;

    /**
     * @brief Push a one-time scheduled task
     * @param event
     * @param delay
     */
    GTimerTaskPtr post(GTimerEvent event, int64_t delay);

private:
    GTimerTaskPtr addTask(GTimerEvent event, GTimerCondition condition, int64_t delay, int64_t interval, bool oneShot);

private:
    friend class GTimer;

    std::string mName;

    mutable GMutex mLock;
    std::condition_variable mTaskCond;
    std::atomic<bool> mIsRunning{false};

    using CompFunc = std::function<bool(const GTimerTaskPtr &, const GTimerTaskPtr &)>;

    std::priority_queue<GTimerTaskPtr, std::vector<GTimerTaskPtr>, CompFunc> mTaskQueue{
        [](const GTimerTaskPtr &lhs, const GTimerTaskPtr &rhs) {
            return lhs->mTime > rhs->mTime;
        }
    };
};

using GTimerSchedulerPtr = std::shared_ptr<GTimerScheduler>;


class GX_API GTimer : public GObject
{
public:
    explicit GTimer(const GTimerSchedulerPtr &scheduler = nullptr, bool oneShot = false);

    explicit GTimer(bool oneShot);

    ~GTimer() override;

    GTimer(const GTimer &) = delete;

    GTimer(GTimer &&rh) noexcept;

    GTimer &operator=(const GTimer &) = delete;

    GTimer &operator=(GTimer &&rh) noexcept;

public:
    virtual void timeout();

    virtual bool condition();

    void timerEvent(GTimerEvent event);

    void setCondition(GTimerCondition cond);

    void setOneShot(bool oneShot = true);

    void start(int64_t interval);

    void start(int64_t delay, int64_t interval);

    void stop();

private:
    std::weak_ptr<GTimerScheduler> mScheduler;
    GTimerEvent mEvent;
    GTimerCondition mCondition;
    std::weak_ptr<GTimerTask> mTask;
    bool mOneShot{false};
};

#endif //GX_GTIMER_H
