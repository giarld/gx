//
// Created by Gxin on 2023/3/7.
//

#include "gx/gtimer.h"

#include "gx/debug.h"

#if GX_PLATFORM_WINDOWS

#include <windows.h>

#pragma comment(lib, "winmm.lib")

#endif

namespace gx
{
#if GX_PLATFORM_WINDOWS

void timeBeginPeriod(int32_t t)
{
    ::timeBeginPeriod(t);
}

void timeEndPeriod(int32_t t)
{
    ::timeEndPeriod(t);
}

#else

inline void timeBeginPeriod(int32_t t)
{
}

inline void timeEndPeriod(int32_t t)
{
}

#endif
}

static std::weak_ptr<GTimerScheduler> sGlobalScheduler;

GTimerTask::GTimerTask(GTimerEvent event, GTimerCondition condition, int64_t delay, int64_t interval)
    : mEvent(std::move(event)),
      mCondition(std::move(condition)),
      mInterval(interval),
      mTime(GTime::currentSteadyTime()),
      mValid(true)
{
    mTime.addMilliSecs(delay);
}

void GTimerTask::cancel()
{
    if (mValid.exchange(false)) {
        mEvent = nullptr;
        mCondition = nullptr;
    }
}

GTimerScheduler::GTimerScheduler(std::string name)
    : mName(std::move(name))
{
}

std::shared_ptr<GTimerScheduler> GTimerScheduler::create(std::string name)
{
    return std::shared_ptr<GTimerScheduler>(GX_NEW(GTimerScheduler, std::move(name)));
}

void GTimerScheduler::makeGlobal(const std::shared_ptr<GTimerScheduler> &obj)
{
    if (obj) {
        sGlobalScheduler = obj;
    }
}

std::shared_ptr<GTimerScheduler> GTimerScheduler::global()
{
    return sGlobalScheduler.lock();
}

GTimerScheduler::~GTimerScheduler() = default;

bool GTimerScheduler::run()
{
    mIsRunning.store(true);

    while (true) {
        GTimerTaskPtr task;
        //
        {
            GLocker<GMutex> locker(mLock);
            mTaskCond.wait(locker, [this] {
                return !mIsRunning.load() || !mTaskQueue.empty();
            });
            if (!mIsRunning.load()) {
                break;
            }
            task = mTaskQueue.top();
            GTime now = GTime::currentSteadyTime();
            if (task->mValid.load() && task->mTime > now) {
                GTime::TimeType timeDiff = task->mTime.milliSecsTo(now);
                if (timeDiff > 0) {
                    task.reset();
                    gx::timeBeginPeriod(1);
                    mTaskCond.wait_for(locker, std::chrono::milliseconds(timeDiff));
                    gx::timeEndPeriod(1);
                    continue;
                }
            }
            mTaskQueue.pop();
        }
        GTimerEvent event;
        if (task && task->mValid.load() && ((event = task->mEvent))) {
            auto pushQueue = [&] {
                task->mTime.update();
                task->mTime.addMilliSecs(task->mInterval); {
                    GLocker<GMutex> locker(mLock);
                    mTaskQueue.push(task);
                }
            };
            if (task->mCondition && !task->mCondition()) {
                pushQueue();
            } else {
                event();
                if (!task->mOneShot) {
                    pushQueue();
                }
            }
        }
    }

    return true;
}

int64_t GTimerScheduler::loop(int64_t maxTime)
{
    const GTime beginTime = GTime::currentSteadyTime();
    GTimerTaskPtr task;
    while (mIsRunning.load()) {
        {
            GLocker<GMutex> locker(mLock);
            if (mTaskQueue.empty()) {
                break;
            }
            GTime now = GTime::currentSteadyTime();
            task = mTaskQueue.top();
            if (task->mValid.load() && task->mTime > now) {
                break;
            }
            mTaskQueue.pop();
        }
        GTimerEvent event;
        if (task && task->mValid.load() && ((event = task->mEvent))) {
            auto pushQueue = [&] {
                task->mTime.update();
                task->mTime.addMilliSecs(task->mInterval); {
                    GLocker<GMutex> locker(mLock);
                    mTaskQueue.push(task);
                }
            };
            if (task->mCondition && !task->mCondition()) {
                pushQueue();
            } else {
                event();
                if (!task->mOneShot && task->mValid.load()) {
                    pushQueue();
                }
            }
        }
        if (GTime::currentSteadyTime().milliSecsTo(beginTime) > maxTime) {
            break;
        }
    }

    return GTime::currentSteadyTime().milliSecsTo(beginTime);
}

void GTimerScheduler::start()
{
    mIsRunning.store(true);
}

void GTimerScheduler::stop()
{
    if (mIsRunning.exchange(false)) {
        GLockerGuard locker(mLock);
        while (!mTaskQueue.empty()) {
            mTaskQueue.pop();
        }
        mTaskCond.notify_all();
    }
}

bool GTimerScheduler::isRunning() const
{
    return mIsRunning.load();
}

GTimerScheduler::GTimerTaskPtr GTimerScheduler::post(GTimerEvent event, int64_t delay)
{
    return addTask(std::move(event), nullptr, delay, 0, true);
}

GTimerScheduler::GTimerTaskPtr GTimerScheduler::addTask(GTimerEvent event,
                                                        GTimerCondition condition,
                                                        int64_t delay,
                                                        int64_t interval,
                                                        bool oneShot)
{
    auto task = std::shared_ptr<GTimerTask>(new GTimerTask(std::move(event), std::move(condition), delay, interval));
    task->mOneShot = oneShot;
    GLockerGuard locker(mLock);
    mTaskQueue.push(task);
    mTaskCond.notify_one();
    return task;
}


GTimer::GTimer(const std::shared_ptr<GTimerScheduler> &scheduler, bool oneShot)
    : mScheduler(scheduler),
      mOneShot(oneShot)
{
    if (mScheduler.expired()) {
        mScheduler = sGlobalScheduler;
    }
    mEvent = [this] { timeout(); };
    mCondition = [this] { return condition(); };
}

GTimer::GTimer(bool oneShot)
    : mOneShot(oneShot)
{
    mScheduler = sGlobalScheduler;
    mEvent = [this] { timeout(); };
    mCondition = [this] { return condition(); };
}

GTimer::~GTimer()
{
    stop();
}

GTimer::GTimer(GTimer &&rh) noexcept
    : mScheduler(std::move(rh.mScheduler)),
      mEvent(std::move(rh.mEvent)),
      mCondition(std::move(rh.mCondition)),
      mTask(std::move(rh.mTask))
{
}

GTimer &GTimer::operator=(GTimer &&rh) noexcept
{
    if (this != &rh) {
        mScheduler = std::move(rh.mScheduler);
        mEvent = std::move(rh.mEvent);
        mCondition = std::move(rh.mCondition);
        mTask = std::move(rh.mTask);
    }
    return *this;
}

void GTimer::timeout()
{
}

bool GTimer::condition()
{
    return true;
}

void GTimer::timerEvent(GTimerEvent event)
{
    if (!event) {
        mEvent = [this] { timeout(); };
    } else {
        mEvent = std::move(event);
    }
    if (mTask.expired()) {
        return;
    }
    const auto taskPtr = mTask.lock();
    if (taskPtr && taskPtr->mValid.load()) {
        taskPtr->mEvent = mEvent;
    }
}

void GTimer::setCondition(GTimerCondition cond)
{
    if (!cond) {
        mCondition = [this] { return condition(); };
    } else {
        mCondition = std::move(cond);
    }
    if (mTask.expired()) {
        return;
    }
    const auto task = mTask.lock();
    if (task && task->mValid.load()) {
        task->mCondition = mCondition;
    }
}

void GTimer::setOneShot(bool oneShot)
{
    mOneShot = oneShot;
    if (mTask.expired()) {
        return;
    }
    const auto taskPtr = mTask.lock();
    if (taskPtr && taskPtr->mValid.load()) {
        taskPtr->mOneShot = mOneShot;
    }
}

void GTimer::start(int64_t interval)
{
    start(interval, interval);
}

void GTimer::start(int64_t delay, int64_t interval)
{
    if (!mEvent) {
        return;
    }
    stop();

    GX_ASSERT_S(!mScheduler.expired(), "GTimer: Invalid scheduler");
    const auto scheduler = mScheduler.lock();
    if (scheduler) {
        mTask = scheduler->addTask(mEvent, mCondition, delay, interval, mOneShot);
    }
}

void GTimer::stop()
{
    if (mTask.expired()) {
        return;
    }
    const auto taskPtr = mTask.lock();
    if (taskPtr) {
        taskPtr->cancel();
        mTask.reset();
    }
}
