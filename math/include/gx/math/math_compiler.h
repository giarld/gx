//
// Created by Gxin on 2021/2/24.
//

#include <gx/gglobal.h>
#include <gx/platform.h>

#include <gx/debug.h>

#include <type_traits>

#if GX_PLATFORM_WINDOWS

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifdef far
#undef far
#endif

#ifdef near
#undef near
#endif

#endif

#ifndef GX_COMPILER_H
#define GX_COMPILER_H

#if GX_COMPILER_MSVC || (GX_COMPILER_CLANG && GX_PLATFORM_WINDOWS)

#define MATH_EMPTY_BASES __declspec(empty_bases)

#define MATH_CONSTEXPR_INIT {}
#define MATH_DEFAULT_CTOR {}
#define MATH_DEFAULT_CTOR_CONSTEXPR constexpr

#else

#define MATH_EMPTY_BASES

#define MATH_CONSTEXPR_INIT
#define MATH_DEFAULT_CTOR = default;
#define MATH_DEFAULT_CTOR_CONSTEXPR

#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

namespace math
{
template<typename T>
struct is_arithmetic : std::integral_constant<bool,
            std::is_integral<T>::value || std::is_floating_point<T>::value>
{
};
}

#endif //GX_COMPILER_H
