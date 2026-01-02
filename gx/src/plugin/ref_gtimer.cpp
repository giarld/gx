//
// Created by Gxin on 2023/3/8.
//

#include <gx/gany.h>

#include <gx/gtimer.h>


using namespace gany;

void refGTimer()
{
    Class<GTimerTask>("Gx", "GTimerTask", "Gx timer task.")
            .func("cancel", &GTimerTask::cancel);

    Class<GTimerScheduler>("Gx", "GTimerScheduler", "Gx timer scheduler.")
            .staticFunc("create", &GTimerScheduler::create)
            .staticFunc("makeGlobal", &GTimerScheduler::makeGlobal)
            .staticFunc("global", &GTimerScheduler::global)
            .func("run", &GTimerScheduler::run)
            .func("loop", &GTimerScheduler::loop)
            .func("start", &GTimerScheduler::start)
            .func("stop", &GTimerScheduler::stop)
            .func("isRunning", &GTimerScheduler::isRunning)
            .func("post", &GTimerScheduler::post);

    Class<GTimer>("Gx", "GTimer", "Gx timer.")
            .construct<>()
            .construct<GTimerSchedulerPtr>()
            .construct<GTimerSchedulerPtr, bool>()
            .construct<bool>()
            .inherit<GObject>()
            .func("timerEvent", &GTimer::timerEvent)
            .func("setOneShot", [](GTimer &self, bool oneShot) {
                self.setOneShot(oneShot);
            })
            .func("setOneShot", [](GTimer &self) {
                self.setOneShot();
            })
            .func("start", [](GTimer &self, int64_t interval) {
                self.start(interval);
            }, {"", {"interval"}})
            .func("start", [](GTimer &self, int64_t delay, int64_t interval) {
                self.start(delay, interval);
            }, {"", {"delay", "interval"}})
            .func("stop", &GTimer::stop);
}
