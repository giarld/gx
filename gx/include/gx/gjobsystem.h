//
// Created by Gxin on 2021/2/28.
//

#ifndef GX_JOB_SYSTEM_H
#define GX_JOB_SYSTEM_H

#include "gwork_stealing_dequeue.h"

#include "allocator.h"
#include "memalign.h"
#include "gmutex.h"
#include "gthread.h"

#include <vector>
#include <functional>

#include <unordered_map>


class GX_API GJobSystem final : public GObject
{
private:
    constexpr static size_t MAX_JOB_COUNT = 4096;
    using WorkQueue = GWorkStealingDequeue<uint16_t, MAX_JOB_COUNT>;

public:
    class Job;

    using JobFunc = std::function<void(GJobSystem *, Job *)>;

    class alignas(GX_CACHE_LINE_SIZE) Job
    {
    public:
        Job() = default;

        Job(const Job &) = delete;

        Job(Job &&) = delete;

    private:
        friend class GJobSystem;

        JobFunc function;
        uint16_t parent = 0;
        std::atomic<uint16_t> runningJobCount = {1};
        std::atomic<uint16_t> refCount = {1};
    };

public:
    explicit GJobSystem(const std::string &name = "JobSystem", uint32_t threadCount = 0,
                       uint32_t adoptableThreadsCount = 1);

    ~GJobSystem() override;

    /**
     * @brief Make the current thread a part of the thread pool, that is, bind the current thread as a worker thread to the thread pool,
     * and call at startup
     */
    void adopt();

    /**
     * @brief Unbind User Thread
     */
    void emancipate();

    /**
     * @brief When a ParentJob is not specified when creating a Job, RootJob will be treated as a ParentJob,
     * After rootJob wait, it will be reset
     * @param job
     * @return
     */
    Job *setRootJob(Job *job);

    /**
     * @brief To create a job, the thread executing the current function must be either a thread from the JobSystem's thread pool or an adopt JobSystem thread
     * @param parent
     * @param func
     * @return
     */
    Job *createJob(Job *parent, JobFunc func);

    Job *createJob()
    {
        return createJob(nullptr, nullptr);
    }

    /**
     * @brief Jobs are generally completed automatically, and cancel() can be used to cancel a job before it is completed.
     * @note Jobs executed by run() cannot be cancelled
     * @param job
     */
    void cancel(Job *&job);

    /**
     * @brief Add references to the job
     *
     * Allow the caller to waitAndRelease() this task in multiple threads.
     * If you do not need to wait for multiple threads, use runAndWait().
     *
     * This job must wait with waitAndRelease() or release with release().
     *
     * @param job
     * @return
     */
    Job *retain(Job *job);

    /**
     * @brief Release a reference to a job that is returned through a run And Retain() or retain () call.
     *
     * The job after executing this function will no longer be used in other functions.
     *
     * @param job
     */
    void release(Job *&job);

    void release(Job *&&job)
    {
        Job *p = job;
        release(p);
    }

    /**
     * @brief Adding a job to the thread's execution queue will result in the job's reference being confiscated and automatically released upon completion of the job's execution,
     * The thread executing the current function must be a thread in the JobSystem's thread pool or an adopt JobSystem thread
     *
     * The job after executing this function will no longer be used in other functions
     *
     * @param job
     */
    void run(Job *&job);

    void run(Job *&&job)
    {
        Job *p = job;
        run(p);
    }

    /**
     * @brief Equivalent to wake(),
     * Attempt to wake up thread
     */
    void signal();

    /**
     * @brief Add the job to the thread's execution queue and retain the reference to the job.
     * The thread executing the current function must be a thread in the JobSystem's thread pool or an adopt JobSystem thread
     *
     * This job must wait with wait(), release with release(), or directly wait and release with waitAndRelease()
     *
     * @param job
     * @return
     */
    Job *runAndRetain(Job *job);

    /**
     * @brief Wait for a job and then destroy it.
     * The thread executing the current function must be a thread in the JobSystem's thread pool or an adopt JobSystem thread
     *
     * Job must first be obtained from run And Retain() or Retain().
     *
     * The job after executing this function will no longer be used in other functions
     *
     * @param job
     */
    void waitAndRelease(Job *&job);

    /**
     * @brief Equivalent to executing runAndRetain()+waitAndRelease();
     *
     * The job after executing this function will no longer be used in other functions
     * @param job
     */
    void runAndWait(Job *&job);

    void runAndWait(Job *&&job)
    {
        Job *p = job;
        runAndWait(p);
    }

private:
    class DefaultRandomEngine
    {
        constexpr static uint32_t m = 0x7fffffffu;
        uint32_t mState; // must be 0 < seed < 0x7fffffff
    public:
        constexpr explicit DefaultRandomEngine(uint32_t seed = 1u) noexcept
            : mState(((seed % m) == 0u) ? 1u : seed % m)
        {
        }

        uint32_t operator()() noexcept
        {
            return mState = static_cast<uint32_t>((static_cast<uint64_t>(mState) * 48271u) % m);
        }
    };

    struct alignas(GX_CACHE_LINE_SIZE) ThreadState
    {
        WorkQueue workQueue;

        alignas(GX_CACHE_LINE_SIZE)
        GJobSystem *js;
        GThread thread;
        DefaultRandomEngine rndGen;
        uint32_t id;
    };

    ThreadState &getState();

    void incRef(Job *job);

    void decRef(Job *job);

    Job *allocateJob();

    ThreadState *getStateToStealFrom(ThreadState &state);

    static bool hasJobCompleted(Job *job);

    void requestExit();

    bool exitRequested() const;

    bool hasActiveJobs() const;

    void loop(ThreadState *state);

    bool execute(ThreadState &state);

    Job *steal(ThreadState &state);

    void finish(Job *job);

    void put(WorkQueue &workQueue, Job *job);

    Job *pop(WorkQueue &workQueue);

    Job *steal(WorkQueue &workQueue);

    void wait(GLocker<GMutex> &lock);

    void wakeAll();

    void wakeOne();

    size_t getThreadCount() const;

    static void setThreadAffinityById(size_t id);

private:
    GMutex mWaitLock;
    std::condition_variable mWaitCondition;

    std::atomic<int32_t> mActiveJobs = {0};
    GArena<ObjectPoolAllocator<Job>, GLockingPolicy::Mutex> mJobPool;

    template<typename T>
    using AlignedVector = std::vector<T, GSTLAlignedAllocator<T> >;

    char padding[GX_CACHE_LINE_SIZE]{};

    alignas(16)
    AlignedVector<ThreadState> mThreadStates; // Offline storage of actual data
    std::atomic<bool> mExitRequested = {false};
    std::atomic<uint16_t> mAdoptedThreads = {0};
    Job *const mJobStorageBase;
    uint16_t mThreadCount = 0;
    Job *mRootJob = nullptr;

    GMutex mThreadMapLock; // this should have very little contention
    std::unordered_map<GThread::ThreadIdType, ThreadState *> mThreadMap;
};

#endif //GX_JOB_SYSTEM_H
