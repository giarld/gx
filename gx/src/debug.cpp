//
// Created by Gxin on 2020/8/13.
//

#include "gx/debug.h"

#include <gx/gtime.h>
#include <gx/gthread.h>

#include <cstdarg>
#include <cstdlib>
#include <cstdio>


#if GX_PLATFORM_ANDROID
#include <android/log.h>
#elif GX_PLATFORM_IOS
#include <os/log.h>
#endif // GX_PLATFORM_ANDROID

namespace gx
{

#define DEBUG_CHAR_BUFFER_SIZE 16384

void defaultOutputWriterFunc(int level, const char *buffer)
{
    const std::string ts = GTime::currentSystemTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
#if GX_PLATFORM_ANDROID
    switch (level) {
        case 0:
            __android_log_print(ANDROID_LOG_INFO, "Gx", "%s [INFO] %s", ts.c_str(), buffer);
            break;
        case 1:
            __android_log_print(ANDROID_LOG_DEBUG, "Gx", "%s [DEBUG] %s", ts.c_str(), buffer);
            break;
        case 2:
            __android_log_print(ANDROID_LOG_WARN, "Gx", "%s [WARN] %s", ts.c_str(), buffer);
            break;
        case 3:
            __android_log_print(ANDROID_LOG_ERROR, "Gx", "%s [ERROR] %s", ts.c_str(), buffer);
            break;
        default:
            break;
    }
#elif GX_PLATFORM_IOS
    switch (level) {
        case 0:
            os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_INFO, "%{public}s [INFO] %{public}s", ts.c_str(), buffer);
            break;
        case 1:
            os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_DEBUG, "%{public}s [DEBUG] %{public}s", ts.c_str(), buffer);
            break;
        case 2:
            os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_DEFAULT, "%{public}s [WARN] %{public}s", ts.c_str(), buffer);
            break;
        case 3:
            os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_ERROR, "%{public}s [ERROR] %{public}s", ts.c_str(), buffer);
            break;
        default:
            break;
    }
#else
    switch (level) {
        case 0:
            fprintf(stdout, "%s [INFO] %s\n", ts.c_str(), buffer);
            break;
        case 1:
            fprintf(stdout, "%s [DEBUG] %s\n", ts.c_str(), buffer);
            break;
        case 2:
            fprintf(stdout, "%s [WARN] %s\n", ts.c_str(), buffer);
            break;
        case 3:
            fprintf(stderr, "%s [ERROR] %s\n", ts.c_str(), buffer);
            break;
        default:
            break;
    }
#endif
}

static DebugOutputWriter sWriterFunc = defaultOutputWriterFunc;

void debugPrintf(int level, const GString &msg)
{
    if (sWriterFunc) {
        sWriterFunc(level, msg.c_str());
    }
}

void setDebugOutputWriter(DebugOutputWriter writer)
{
    if (writer) {
        sWriterFunc = writer;
    } else {
        sWriterFunc = defaultOutputWriterFunc;
    }
}

} // gx
