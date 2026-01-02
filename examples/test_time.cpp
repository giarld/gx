//
// Created by Gxin on 2020/6/23.
//

#include <gx/debug.h>
#include <gx/gtime.h>
#include <gx/gthread.h>


int main(int argc, char *argv[])
{
#if GX_PLATFORM_WINDOWS
    system("chcp 65001");
#endif
    GTime currentTime = GTime::currentSteadyTime();

    GThread::sleep(1);

    uint64_t diffU = GTime::currentSteadyTime().microSecsTo(currentTime);
    double diffD = GTime::currentSteadyTime().secsDTo(currentTime);
    Log("Time diff = {} or {}", diffU, diffD);

    currentTime = GTime::currentSystemTime();
    Log("{}", currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz"));
    Log("{}", currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz ap"));
    Log("{}", currentTime.toString("yyyy-MM-dd hh:mm:ss.zzz AP"));
    Log("{}", currentTime.toString("yyyy-MM-dd HH:mm:ss.z"));
    Log("{}", currentTime.toString("yyyy-MM-dd HH:mm:s.z"));
    Log("{}", currentTime.toString("yyyy-MM-dd HH:m:s.z"));
    Log("{}", currentTime.toString("yyyy-MM-dd H:m:s.z"));
    Log("{}", currentTime.toString("yyyy-MM-dd hh:m:s.z"));
    Log("{}", currentTime.toString("yyyy-MM-dd h:m:s.z"));
    Log("{}", currentTime.toString("yyyy-MM-d H:m:s.z"));
    Log("{}", currentTime.toString("yyyy-M-d H:m:s.z"));
    Log("{}", currentTime.toString("yy-M-d H:m:s.z"));
    Log("{}", currentTime.toString("yyyyMMddHHmmsszzz"));

    currentTime.addNanoSecs(10000000);

    Log("{}", currentTime.toString("yyyy-MM-dd HH:mm:ss.zzz"));

    return EXIT_SUCCESS;
}