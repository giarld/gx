//
// Created by Gxin on 2021/2/28.
//

#include "gx/gjobsystem.h"

#include <random>
#include <utility>
#include <sstream>

#if GX_PLATFORM_LINUX
// There is no glibc wrapper for gettid on linux so we need to syscall it.
#    include <unistd.h>
#    include <sys/syscall.h>
#    define gettid() syscall(SYS_gettid)
#endif


GJobSystem::GJobSystem(const std::string &name, uint32_t threadCount, uint32_t adoptableThreadsCount)
    : mJobPool("JobSystem Job pool", MAX_JOB_COUNT * sizeof(Job)),
      mJobStorageBase(static_cast<Job *>(mJobPool.getAllocator().getCurrent()))
{
    uint32_t threadPoolCount = threadCount;

    if (threadPoolCount == 0) {
        uint32_t hwThreads = GThread::hardwareConcurrency();

        hwThreads = std::max(static_cast<uint32_t>(2), hwThreads);
        // 其中一个线程是用户线程
        threadPoolCount = hwThreads - 1;
    }
    threadPoolCount = std::max(1u, std::min(GThread::hardwareConcurrency() - 1, threadPoolCount));

    mThreadStates = AlignedVector<ThreadState>(threadPoolCount + adoptableThreadsCount);
    mThreadCount = static_cast<uint16_t>(threadPoolCount);

    static_assert(std::atomic<bool>::is_always_lock_free);
    static_assert(std::atomic<uint16_t>::is_always_lock_free);

    std::random_device rd;
    const size_t hardwareThreadCount = mThreadCount;
    auto &states = mThreadStates;

#if GX_COMPILER_CLANG
#pragma nounroll
#endif
    for (size_t i = 0, n = states.size(); i < n; i++) {
        auto &state = states[i];
        state.rndGen = DefaultRandomEngine(rd());
        state.id = static_cast<uint32_t>(i);
        state.js = this;
        if (i < hardwareThreadCount) {
            std::stringstream tNameS;
            tNameS << name << "_" << i;
            state.thread.setRunnable([this, pState = &state] {
                loop(pState);
            });
            state.thread.setName(tNameS.str());
            state.thread.start();
        }
    }
}

GJobSystem::~GJobSystem()
{
    requestExit();

#if GX_COMPILER_CLANG
#pragma nounroll
#endif
    for (auto &state: mThreadStates) {
        state.thread.join();
    }
}

void GJobSystem::adopt()
{
    const auto tid = GThread::currentThreadId();

    mThreadMapLock.lock();
    const auto iter = mThreadMap.find(tid);
    const ThreadState *const state = iter == mThreadMap.end() ? nullptr : iter->second;
    mThreadMapLock.unlock();

    if (state) {
        GX_ASSERT_S(this == state->js,
                    "Called adopt on a thread owned by another JobSystem (%llx), this=%llx!",
                    (uintptr_t)state->js, (uintptr_t)this);
        return;
    }

    const uint16_t adopted = mAdoptedThreads.fetch_add(1, std::memory_order_relaxed);
    const size_t index = mThreadCount + adopted;

    GX_ASSERT_S(index < mThreadStates.size(),
                "Too many calls to adopt(). No more adoptable threads!");

    mThreadMapLock.lock();
    mThreadMap[tid] = &mThreadStates[index];
    mThreadMapLock.unlock();
}

void GJobSystem::emancipate()
{
    const auto tid = GThread::currentThreadId();
    GLockerGuard lock(mThreadMapLock);
    const auto iter = mThreadMap.find(tid);
    const ThreadState *const state = iter == mThreadMap.end() ? nullptr : iter->second;
    GX_ASSERT_S(state, "This thread is not an adopted thread!");
    GX_ASSERT_S(state->js == this, "This thread is not adopted by us!");
    mThreadMap.erase(iter);
}

GJobSystem::Job *GJobSystem::setRootJob(Job *job)
{
    return mRootJob = job;
}

GJobSystem::Job *GJobSystem::createJob(Job *parent, JobFunc func)
{
    parent = (parent == nullptr) ? mRootJob : parent;
    Job *const job = allocateJob();
    if (job) {
        size_t index = 0x7FFF;
        if (parent) {
            const auto parentJobCount = parent->runningJobCount.fetch_add(1, std::memory_order_relaxed);

            // 无法为已终止的父 Job 创建子 Job
            GX_ASSERT(parentJobCount > 0);

            index = parent - mJobStorageBase;
            GX_ASSERT(index < MAX_JOB_COUNT);
        }
        job->function = std::move(func);
        job->parent = static_cast<uint16_t>(index);
    }
    return job;
}

void GJobSystem::cancel(Job *&job)
{
    finish(job);
    job = nullptr;
}

GJobSystem::Job *GJobSystem::retain(Job *job)
{
    Job *retained = job;
    incRef(retained);
    return retained;
}

void GJobSystem::release(Job *&job)
{
    decRef(job);
    job = nullptr;
}

void GJobSystem::run(Job *&job)
{
    ThreadState &state(getState());

    put(state.workQueue, job);

    // run 过的 job 不能再被 run，所以应该置空 (运行结束后会自然死亡)
    job = nullptr;
}

void GJobSystem::signal()
{
    wakeAll();
}

GJobSystem::Job *GJobSystem::runAndRetain(Job *job)
{
    Job *retained = retain(job);
    run(job);
    return retained;
}

void GJobSystem::waitAndRelease(Job *&job)
{
    GX_ASSERT(job);
    GX_ASSERT(job->refCount.load(std::memory_order_relaxed) >= 1);

    ThreadState &state = getState();
    do {
        if (!execute(state)) {
            // 测试 Job 是否先完成，以避免获取锁
            if (hasJobCompleted(job)) {
                break;
            }

            // 当前线程没有竞争到 Job 的执行权, 说明 Job 正在被别的工作线程执行, 我们只需等待 Job 执行完成即可.

            GLocker<GMutex> lock(mWaitLock);
            if (!hasJobCompleted(job) && !hasActiveJobs() && !exitRequested()) {
                wait(lock);
            }
        }
    } while (!hasJobCompleted(job) && !exitRequested());

    if (job == mRootJob) {
        mRootJob = nullptr;
    }

    release(job);
}

void GJobSystem::runAndWait(Job *&job)
{
    runAndRetain(job);
    waitAndRelease(job);
}

GJobSystem::ThreadState &GJobSystem::getState()
{
    GLockerGuard lock(mThreadMapLock);
    const auto iter = mThreadMap.find(GThread::currentThreadId());
    GX_ASSERT_S(iter != mThreadMap.end(), "This thread has not been adopted.");
    return *iter->second;
}

void GJobSystem::incRef(Job *job)
{
    job->refCount.fetch_add(1, std::memory_order_relaxed);
}

void GJobSystem::decRef(Job *job)
{
    const auto c = job->refCount.fetch_sub(1, std::memory_order_acq_rel);
    GX_ASSERT(c > 0);
    if (c == 1) {
        // 这是最后一次引用，可以安全地销毁 Job
        mJobPool.destroy(job);
    }
}

GJobSystem::Job *GJobSystem::allocateJob()
{
    return mJobPool.make<Job>();
}

GJobSystem::ThreadState *GJobSystem::getStateToStealFrom(ThreadState &state)
{
    auto &threadStates = mThreadStates;

    const uint16_t adopted = mAdoptedThreads.load(std::memory_order_relaxed);
    const uint16_t threadCount = mThreadCount + adopted;

    ThreadState *stateToStealFrom = nullptr;

    // 如果是唯一的线程就不需要执行以下操作
    if (threadCount >= 2) {
        do {
            const auto index = static_cast<uint16_t>(state.rndGen() % threadCount);
            GX_ASSERT(index < threadStates.size());
            stateToStealFrom = &threadStates[index];
        } while (stateToStealFrom == &state);
    }
    return stateToStealFrom;
}

bool GJobSystem::hasJobCompleted(Job *job)
{
    return job->runningJobCount.load(std::memory_order_acquire) <= 0;
}

void GJobSystem::requestExit()
{
    mExitRequested.store(true);
    GLockerGuard lock(mWaitLock);
    mWaitCondition.notify_all();
}

bool GJobSystem::exitRequested() const
{
    return mExitRequested.load(std::memory_order_relaxed);
}

bool GJobSystem::hasActiveJobs() const
{
    return mActiveJobs.load(std::memory_order_relaxed) > 0;
}

void GJobSystem::loop(ThreadState *state)
{
    setThreadAffinityById(state->id);

    mThreadMapLock.lock();
    const bool inserted = mThreadMap.emplace(GThread::currentThreadId(), state).second;
    mThreadMapLock.unlock();
    GX_ASSERT_S(inserted, "This thread is already in a loop.");

    do {
        if (!execute(*state)) {
            GLocker<GMutex> lock(mWaitLock);
            while (!exitRequested() && !hasActiveJobs()) {
                wait(lock);
                setThreadAffinityById(state->id);
            }
        }
    } while (!exitRequested());
}

bool GJobSystem::execute(ThreadState &state)
{
    Job *job = pop(state.workQueue);
    if (job == nullptr) {
        // our queue is empty, try to steal a job
        job = steal(state);
    }

    if (job) {
        GX_ASSERT(job->runningJobCount.load(std::memory_order_relaxed) >= 1);

        if (job->function) {
            job->function(this, job);
        }
        finish(job);
    }
    return job != nullptr;
}

GJobSystem::Job *GJobSystem::steal(ThreadState &state)
{
    Job *job = nullptr;
    do {
        ThreadState *const stateToStealFrom = getStateToStealFrom(state);
        if (stateToStealFrom) {
            job = steal(stateToStealFrom->workQueue);
        }
    } while (!job && hasActiveJobs());
    return job;
}

void GJobSystem::finish(Job *job)
{
    bool notify = false;

    // 终止这个 Job, 并通知他的父 Job
    Job *const storage = mJobStorageBase;
    do {
        const auto runningJobCount = job->runningJobCount.fetch_sub(1, std::memory_order_acq_rel);
        GX_ASSERT(runningJobCount > 0);
        if (runningJobCount == 1) {
            notify = true;
            Job *const parent = job->parent == 0x7FFF ? nullptr : &storage[job->parent];
            decRef(job);
            job = parent;
        } else {
            break;
        }
    } while (job);

    if (notify) {
        wakeAll();
    }
}

void GJobSystem::put(WorkQueue &workQueue, Job *job)
{
    GX_ASSERT(job);
    const size_t index = job - mJobStorageBase;
    GX_ASSERT(index < MAX_JOB_COUNT);

    workQueue.push(static_cast<uint16_t>(index + 1));
    const int32_t oldActiveJobs = mActiveJobs.fetch_add(1, std::memory_order_relaxed);

    // 有可能 Job 已经被选中, 所以 oldActiveJobs 可能是负数
    if (oldActiveJobs >= 0) {
        wakeOne();
    }
}

GJobSystem::Job *GJobSystem::pop(WorkQueue &workQueue)
{
    mActiveJobs.fetch_sub(1, std::memory_order_relaxed);

    const size_t index = workQueue.pop();
    GX_ASSERT(index <= MAX_JOB_COUNT);
    Job *job = !index ? nullptr : &mJobStorageBase[index - 1];

    if (!job) {
        if (mActiveJobs.fetch_add(1, std::memory_order_relaxed) >= 0) {
            wakeOne();
        }
    }
    return job;
}

GJobSystem::Job *GJobSystem::steal(WorkQueue &workQueue)
{
    mActiveJobs.fetch_sub(1, std::memory_order_relaxed);

    const size_t index = workQueue.steal();
    assert(index <= MAX_JOB_COUNT);
    Job *job = !index ? nullptr : &mJobStorageBase[index - 1];

    if (!job) {
        if (mActiveJobs.fetch_add(1, std::memory_order_relaxed) >= 0) {
            wakeOne();
        }
    }
    return job;
}

void GJobSystem::wait(GLocker<GMutex> &lock)
{
    mWaitCondition.wait(lock);
}

void GJobSystem::wakeAll()
{
    GLockerGuard lock(mWaitLock);
    mWaitCondition.notify_all();
}

void GJobSystem::wakeOne()
{
    GLockerGuard lock(mWaitLock);
    mWaitCondition.notify_one();
}

size_t GJobSystem::getThreadCount() const
{
    return mThreadCount;
}

void GJobSystem::setThreadAffinityById(size_t id)
{
#if GX_PLATFORM_LINUX
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(id, &set);
    sched_setaffinity(gettid(), sizeof(set), &set);
#else
    GX_UNUSED(id);
#endif
}
