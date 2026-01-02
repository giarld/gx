//
// Created by Gxin on 2022/6/4.
//

#include "gx/reg_gx.h"

#include "gx/gany.h"

#include "gx/debug.h"


extern void refGObject();

extern void refGString();

extern void refGFile();

extern void refGByteArray();

extern void refGTime();

extern void refGThread();

extern void refGUuid();

extern void refGVersion();

extern void refTaskSystem();

extern void refJobSystem();

extern void refGCrypto();

extern void refGHashSum();

extern void refGTimer();

extern void refOs();

extern void refZipFile();

static GAny sAnyDebugOutputWriter;

void anyDebugOutputWriter(int level, const char *buffer)
{
    if (sAnyDebugOutputWriter.isFunction()) {
        sAnyDebugOutputWriter(level, std::string(buffer));
    }
}

void logToAny()
{
    auto GLog = GAnyClass::Class("", "GLog", "gx log system");
    GAny::Export(GLog);
    GLog->staticFunc("Log", [](const std::string &msg) {
            gx::debugPrintf(0, msg.c_str());
        })
        .staticFunc("LogD", [](const std::string &msg) {
            gx::debugPrintf(1, msg.c_str());
        })
        .staticFunc("LogW", [](const std::string &msg) {
            gx::debugPrintf(2, msg.c_str());
        })
        .staticFunc("LogE", [](const std::string &msg) {
            gx::debugPrintf(3, msg.c_str());
        })
        .staticFunc(
            "setDebugOutputWriter",
            [](const GAny &writerFunc) {
                if (writerFunc.isFunction()) {
                    sAnyDebugOutputWriter = writerFunc;
                    gx::setDebugOutputWriter(anyDebugOutputWriter);
                } else {
                    sAnyDebugOutputWriter = GAny();
                    gx::setDebugOutputWriter(nullptr);
                }
            },
            {
                .doc = "writerFunc: function(int level, string msg)",
                .args = {"writerFunc"}
            });
}

REGISTER_GANY_MODULE(Gx)
{
    refGObject();
    refGString();
    refGFile();
    refGByteArray();
    refGTime();
    refGThread();
    refGUuid();
    refGVersion();
    refTaskSystem();
    refJobSystem();
    refGCrypto();
    refGHashSum();
    refGTimer();
    refOs();
    refZipFile();

    logToAny();
}
