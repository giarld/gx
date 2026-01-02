//
// Created by Gxin on 2022/5/12.
//

#include <utility>

#include "gx/gtasksystem.h"

#include "gx/gthread.h"
#include "gx/debug.h"

#include <sstream>


GTaskSystem::GTaskSystem(std::string name, uint32_t threadCount)
    : mName(std::move(name)),
      mThreadCount(threadCount)
{
    if (mThreadCount == 0 || mThreadCount > GThread::hardwareConcurrency()) {
        mThreadCount = GThread::hardwareConcurrency();
    }
}

GTaskSystem::~GTaskSystem()
{
    stop();
}

uint32_t GTaskSystem::threadCount() const
{
    return mThreadCount;
}

void GTaskSystem::start()
{
    if (mIsRunning.load()) {
        return;
    }

    mIsRunning.store(true);

    mThreads.resize(mThreadCount);

    for (uint32_t i = 0; i < mThreadCount; i++) {
        std::stringstream tNameS;
        tNameS << mName << "_" << i;

        mThreads[i] = std::make_unique<GThread>([this] {
            TaskFuncRef taskFuncRef;
            while (true) {
                {
                    GLocker<GMutex> locker(mLock);
                    mTaskCond.wait(locker, [this] {
                        return !mIsRunning.load() || !mTaskQueue.empty();
                    });
                    if (!mIsRunning.load() && mTaskQueue.empty()) {
                        break;
                    }
                    taskFuncRef = mTaskQueue.front();
                    mTaskQueue.pop_front();
                }
                if (taskFuncRef.active->load(std::memory_order_relaxed)) {
                    GX_ASSERT(taskFuncRef.func);
                    taskFuncRef.func();
                }
            }
        }, tNameS.str());
        mThreads[i]->setPriority(mPriority);
    }
}

void GTaskSystem::stopAndWait()
{
    if (!mIsRunning.load()) {
        return;
    } {
        GLockerGuard locker(mLock);
        mIsRunning.store(false);
        mTaskCond.notify_all();
    }
    for (const auto &thread: mThreads) {
        thread->join();
    }
}

void GTaskSystem::stop()
{
    if (!mIsRunning.load()) {
        return;
    }
    clearTask();
    stopAndWait();
}

bool GTaskSystem::isRunning() const
{
    return mIsRunning.load();
}

void GTaskSystem::setThreadPriority(ThreadPriority priority)
{
    mPriority = priority;
    if (!mThreads.empty()) {
        for (const auto &thread: mThreads) {
            thread->setPriority(priority);
        }
    }
}

ThreadPriority GTaskSystem::getThreadPriority() const
{
    return mPriority;
}

uint64_t GTaskSystem::waitingTaskCount() const
{
    GLockerGuard locker(mLock);
    return mTaskQueue.size();
}

GTaskSystem::TaskFuncRef GTaskSystem::pushTask(const TaskFunc &task)
{
    GLockerGuard locker(mLock);
    TaskFuncRef taskRef{task, std::make_shared<std::atomic<bool> >(true)};
    mTaskQueue.push_back(taskRef);
    mTaskCond.notify_one();
    return taskRef;
}

GTaskSystem::TaskFuncRef GTaskSystem::pushTaskFront(const TaskFunc &task)
{
    GLockerGuard locker(mLock);
    TaskFuncRef taskRef{task, std::make_shared<std::atomic<bool> >(true)};
    mTaskQueue.push_front(taskRef);
    mTaskCond.notify_one();
    return taskRef;
}

void GTaskSystem::clearTask()
{
    GLockerGuard locker(mLock);
    mTaskQueue.clear();
}
