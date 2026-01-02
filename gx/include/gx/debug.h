//
// Created by Gxin on 2019/8/4.
//

#ifndef GX_DEBUG_H
#define GX_DEBUG_H

#include "gx/gglobal.h"

#include <gx/gstring.h>


#if GX_DEBUG
#   define _Log(level, fmt, ...)    \
    do {                    \
        gx::debugPrintf(level, GString::format(__FILE__ "(" GX_STRINGIZE(__LINE__) ") : " fmt, ##__VA_ARGS__)); \
    } while (false)
#else
#   define _Log(level, fmt, ...)   \
    do {                \
        gx::debugPrintf(level, GString::format(" " fmt, ##__VA_ARGS__)); \
    } while (false)
#endif

#define Log(fmt, ...)   _Log(0, fmt, ##__VA_ARGS__)
#define LogW(fmt, ...)  _Log(2, fmt, ##__VA_ARGS__)
#define LogE(fmt, ...)  _Log(3, fmt, ##__VA_ARGS__)

#if GX_DEBUG
#define LogD(fmt, ...)  _Log(1, fmt, ##__VA_ARGS__)
#else
#define LogD(fmt, ...)  ((void)0)
#endif

#if GX_PLATFORM_WINDOWS
#   define DEBUG_BREAK __debugbreak()
#elif GX_PLATFORM_POSIX
#include <signal.h>
#   define DEBUG_BREAK raise(SIGTRAP)
#else
#   define DEBUG_BREAK abort()
#endif

#if GX_DEBUG
#   define GX_ASSERT_S(Exp, fmt, ...) (void)(!!(Exp) || (gx::debugPrintf(3, GString::format("Assertion failed: " __FILE__ "(" GX_STRINGIZE(__LINE__) "), " #Exp ", " fmt, ##__VA_ARGS__)), DEBUG_BREAK, 0))
#   define GX_ASSERT(Exp) (void)(!!(Exp) || (gx::debugPrintf(3, "Assertion failed: " __FILE__ "(" GX_STRINGIZE(__LINE__) "), " #Exp), DEBUG_BREAK, 0))

#   define CHECK_CONDITION_V(Exp) do { if (!(Exp)) { gx::debugPrintf(3, "Assertion failed: " __FILE__ "(" GX_STRINGIZE(__LINE__) "), " #Exp); DEBUG_BREAK; return;} } while(false)
#   define CHECK_CONDITION_R(Exp, RET) do { if (!(Exp)) { gx::debugPrintf(3, "Assertion failed: " __FILE__ "(" GX_STRINGIZE(__LINE__) "), " #Exp); DEBUG_BREAK; return RET;} } while(false)
#   define CHECK_CONDITION_S_V(Exp, fmt, ...) do { if (!(Exp)) { gx::debugPrintf(3, GString::format("Assertion failed: " __FILE__ "(" GX_STRINGIZE(__LINE__) "), " #Exp ", " fmt, ##__VA_ARGS__)); DEBUG_BREAK; return; } } while(false)
#   define CHECK_CONDITION_S_R(Exp, RET, fmt, ...) do { if (!(Exp)) { gx::debugPrintf(3, GString::format("Assertion failed: " __FILE__ "(" GX_STRINGIZE(__LINE__) "), " #Exp ", " fmt, ##__VA_ARGS__)); DEBUG_BREAK; return RET; } } while(false)

#else
#   define GX_ASSERT_S(Exp, fmt, ...) ((void)0)
#   define GX_ASSERT(Exp) ((void)0)

#   define CHECK_CONDITION_V(Exp) do { if (!(Exp)) {return;} } while(false)
#   define CHECK_CONDITION_R(Exp, RET) do { if (!(Exp)) {return RET;} } while(false)
#   define CHECK_CONDITION_S_V(Exp, fmt, ...) do { if (!(Exp)) { LogE(fmt, ##__VA_ARGS__); return; } } while(false)
#   define CHECK_CONDITION_S_R(Exp, RET, fmt, ...) do { if (!(Exp)) { LogE(fmt, ##__VA_ARGS__); return RET; } } while(false)

#endif //_DEBUG

namespace gx
{
typedef void (*DebugOutputWriter)(int level, const char *buffer);

/**
 * @brief print debug message
 * @param level     0: info, 1: debug, 2: warning, 3: error
 * @param msg
 */
GX_API void debugPrintf(int level, const GString &msg);

GX_API void setDebugOutputWriter(DebugOutputWriter writer);
} // gx

#endif //GX_DEBUG_H
