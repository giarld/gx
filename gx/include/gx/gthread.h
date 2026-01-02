//
// Created by gxin on 18-7-21.
//

#ifndef GX_GTHREAD_H
#define GX_GTHREAD_H

#include "gobject.h"

#include "gx/enum.h"

#include <thread>
#include <functional>
#include <string>


DEF_ENUM_5(ThreadPriority, uint8_t, 0,
           Normal,
           AboveNormal,
           BelowNormal,
           Highest,
           Lowest
)

/**
 * @class GThread
 * @brief Thread class provides convenient methods for creating and using threads,
 * as well as methods for setting thread names and thread priorities.
 */
class GX_API GThread : public GObject
{
public:
    using ThreadIdType = std::thread::id;

    using Runnable = std::function<void()>;

public:
    explicit GThread(std::string name = "GThread");

    explicit GThread(const Runnable &runnable, std::string name = "GThread");

    GThread(const GThread &b) = delete;

    GThread(GThread &&b) = delete;

    ~GThread() override;

    GThread &operator=(const GThread &b) = delete;

    GThread &operator=(GThread &&b) noexcept = delete;

    /**
     * @brief Set Runnable, which will override the implementation of the run() method
     * @param runnable
     * @return *this
     */
    GThread &setRunnable(const Runnable &runnable);

    bool joinable() const;

    void start();

    void join();

    void detach();

    ThreadIdType getId() const;

    std::string getIdString() const;

    void setName(const std::string &name);

    std::string getName() const;

    void setPriority(ThreadPriority priority);

    ThreadPriority getPriority() const;

public:
    std::string toString() const override;

protected:
    /**
     * @brief Thread executes this method when Runnable is not set
     */
    virtual void run();

public: //static
    /**
     * @brief Sleep Current Thread
     * @param seconds Unit seconds
     */
    static void sleep(int64_t seconds);

    /**
     * @brief Sleep Current Thread
     * @param ms Unit millisecond
     */
    static void mSleep(int64_t ms);

    static void nSleep(int64_t nanos);

    static ThreadIdType currentThreadId();

    static std::string currentThreadIdString();

    /**
     * @brief Get the number of physical threads
     * @return
     */
    static uint32_t hardwareConcurrency();

private:
    std::thread mThread;
    Runnable mRunnable;

    std::string mName;
    ThreadPriority mPriority = ThreadPriority::Normal;
};

#endif //GX_GTHREAD_H
