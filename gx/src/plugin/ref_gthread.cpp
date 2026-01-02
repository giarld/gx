//
// Created by Gxin on 2022/6/5.
//

#include <gx/gany.h>

#include "gx/gthread.h"
#include "gx/gmutex.h"
#include "gx/debug.h"


using namespace gany;

void refGThread()
{
    REF_ENUM(ThreadPriority, "Gx", "ThreadPriority");

    Class<GThread>("Gx", "GThread", "Gx thread")
            .inherit<GObject>()
            .construct<>()
            .construct<std::string>()
            .staticFunc(MetaFunction::Init, [](GThread::Runnable runnable) {
                return GAny::New<GThread>(runnable);
            }, {"Constructor with runnable.", {"runnable:function"}})
            .staticFunc(MetaFunction::Init, [](GThread::Runnable runnable, const std::string &name) {
                return GAny::New<GThread>(runnable, name);
            }, {"Constructor with runnable and name.", {"runnable:function", "name"}})
            .func("setRunnable", [](GThread &self, GThread::Runnable runnable) {
                self.setRunnable(runnable);
            }, {"Set runnable, Must be set before thread startup, invalid after startup.", {"runnable:function"}})
            .func("start", &GThread::start, {"Start thread."})
            .func("joinable", &GThread::joinable, {"check thread joinable."})
            .func("join", &GThread::join, {"Join thread."})
            .func("detach", &GThread::detach, {"Detach thread."})
            .func("getId", [](const GThread &self) {
                std::stringstream ss;
                ss << self.getId();
                return ss.str();
            }, {"Get thread id."})
            .func("setName", &GThread::setName, {"Set thread name.", {"name"}})
            .func("getName", &GThread::getName, {"Get thread name."})
            .func("setPriority", &GThread::setPriority, {"Set thread priority.", {"priority"}})
            .func("getPriority", &GThread::getPriority, {"Get thread priority."})
            .staticFunc("sleep", &GThread::sleep, {"Sleep with second.", {"seconds"}})
            .staticFunc("mSleep", &GThread::mSleep, {"Sleep with millisecond.", {"ms"}})
            .staticFunc("nSleep", &GThread::nSleep, {"Sleep with nanosecond.", {"ns"}})
            .staticFunc("currentThreadId", []() {
                std::stringstream ss;
                ss << GThread::currentThreadId();
                return ss.str();
            }, {"Get current thread id."})
            .staticFunc("hardwareConcurrency", []() {
                return static_cast<int32_t>(GThread::hardwareConcurrency());
            }, {"Get hardware concurrency thread count."});

    Class<GMutex>("Gx", "GMutex", "Gx thread mutex.")
            .construct<>()
            .func("lock", [](GMutex &self) {
                self.lock();
            })
            .func("tryLock", [](GMutex &self) {
                return self.try_lock();
            })
            .func("unlock", [](GMutex &self) {
                self.unlock();
            })
            .func("lock", [](GMutex &self, std::function<void()> func) {
                self.lock();
                func();
                self.unlock();
            }, {"Using locked code blocks.", {"func:function"}});

    Class<GRecursiveMutex>("Gx", "GRecursiveMutex", "Gx thread recursive mutex.")
            .construct<>()
            .func("lock", [](GRecursiveMutex &self) {
                self.lock();
            })
            .func("tryLock", [](GRecursiveMutex &self) {
                return self.try_lock();
            })
            .func("unlock", [](GRecursiveMutex &self) {
                self.unlock();
            })
            .func("lock", [](GRecursiveMutex &self, std::function<void()> func) {
                self.lock();
                func();
                self.unlock();
            }, {"Using locked code blocks.", {"func:function"}});

    Class<GSpinLock>("Gx", "GSpinLock", "Gx SpinLock.")
            .construct<>()
            .func("lock", [](GSpinLock &self) {
                self.lock();
            })
            .func("unlock", [](GSpinLock &self) {
                self.unlock();
            })
            .func("lock", [](GSpinLock &self, std::function<void()> func) {
                self.lock();
                func();
                self.unlock();
            }, {"Using locked code blocks.", {"func:function"}});

    Class<GNoLock>("Gx", "GNoLock", "Gx NoLock.")
            .construct<>()
            .func("lock", [](GNoLock &self) {
                self.lock();
            })
            .func("tryLock", [](GNoLock &self) {
                return self.try_lock();
            })
            .func("unlock", [](GNoLock &self) {
                self.unlock();
            })
            .func("lock", [](GNoLock &self, std::function<void()> func) {
                self.lock();
                func();
                self.unlock();
            }, {"Using locked code blocks.", {"func:function"}});

    Class<GRWLock>("Gx", "GRWLock", "Gx read/write lock.")
            .construct<>()
            .func("readLock", [](GRWLock &self) {
                self.readLock();
            })
            .func("readUnlock", [](GRWLock &self) {
                self.readUnlock();
            })
            .func("writeLock", [](GRWLock &self) {
                self.writeLock();
            })
            .func("writeUnlock", [](GRWLock &self) {
                self.writeUnlock();
            })
            .func("readLock", [](GRWLock &self, std::function<void()> func) {
                self.readLock();
                func();
                self.readUnlock();
            }, {"Using read locked code blocks.", {"func:function"}})
            .func("writeLock", [](GRWLock &self, std::function<void()> func) {
                self.writeLock();
                func();
                self.writeUnlock();
            }, {"Using write locked code blocks.", {"func:function"}});

    Class<GSpinRWLock>("Gx", "GSpinRWLock", "Gx spin read/write lock.")
            .construct<>()
            .func("readLock", [](GSpinRWLock &self) {
                self.readLock();
            })
            .func("readUnlock", [](GSpinRWLock &self) {
                self.readUnlock();
            })
            .func("writeLock", [](GSpinRWLock &self) {
                self.writeLock();
            })
            .func("writeUnlock", [](GSpinRWLock &self) {
                self.writeUnlock();
            })
            .func("readLock", [](GSpinRWLock &self, std::function<void()> func) {
                self.readLock();
                func();
                self.readUnlock();
            }, {"Using read locked code blocks.", {"func:function"}})
            .func("writeLock", [](GSpinRWLock &self, std::function<void()> func) {
                self.writeLock();
                func();
                self.writeUnlock();
            }, {"Using write locked code blocks.", {"func:function"}});
}
