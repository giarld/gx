//
// Created by Gxin on 2023/3/8.
//

#include <gx/gtimer.h>
#include <gx/gthread.h>


class ATimer : public GTimer
{
public:
    ~ATimer() override = default;

    /**
     * @brief 继承并重写 timeout 作用与设置 timerEvent 一致, 但如果同时设置了 timerEvent, timeout 方法将无效
     */
    void timeout() override
    {
        Log("ATimer timeout.");
    }
};

int main(int argc, char *argv[])
{
    const GTimerSchedulerPtr scheduler = GTimerScheduler::create("TimerScheduler");
    GTimerScheduler::makeGlobal(scheduler);

    GTimer timer1(scheduler);  // 从指定调度器构造
    GTimer timer2(true);
    ATimer timer3;
    GTimer timer4(scheduler);    // 一次性计时器
    timer4.setOneShot();

    uint32_t counter = 0;

    timer1.timerEvent([&]() {
        Log("Timer1 timeout.");
        ++counter;
    });

    timer2.timerEvent([]() {
        Log("Timer2 timeout.");
        auto global = GTimerScheduler::global();
        if (global) {
            global->stop();
        }
    });
    timer2.setCondition([&]() {
        Log("Timer2 condition: counter = {}", counter);
        return counter == 10;
    });

    timer4.timerEvent([&]() {
        Log("Timer4 timeout.");
    });
    timer1.start(0, 1000);  // 立即执行一次, 之后每1000ms执行一次
    timer2.start(500);    // 每500ms执行一次检查，当条件达成时执行 timerEvent
    timer3.start(1000);     // 每1000ms执行一次
    timer4.start(500);
    timer4.start(1000);     // 重新开始, 会覆盖上一个计时任务, 并从头计时

    // 直接通过 GTimerScheduler 提交一次性任务
    GTimerScheduler::global()->post([]() {
        Log("Post timer timeout.");
    }, 3000);

    // 由其他循环驱动
//    scheduler->start();
//    while (scheduler->isRunning()) {
//        scheduler->loop();
//        GThread::mSleep(10);
//    }

    // 同步执行
    return scheduler->run() ? EXIT_SUCCESS : EXIT_FAILURE;
}