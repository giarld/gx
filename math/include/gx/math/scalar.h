//
// Created by Gxin on 2021-02-24
//

#ifndef GX_MATH_SCALAR_H
#define GX_MATH_SCALAR_H

#include "math_compiler.h"


namespace math
{
constexpr double F_E = 2.71828182845904523536028747135266250;
constexpr double F_LOG2E = 1.44269504088896340735992468100189214;
constexpr double F_LOG10E = 0.434294481903251827651128918916605082;
constexpr double F_LN2 = 0.693147180559945309417232121458176568;
constexpr double F_LN10 = 2.30258509299404568401799145468436421;
constexpr double F_PI = 3.14159265358979323846264338327950288;
constexpr double F_PI_2 = 1.57079632679489661923132169163975144;
constexpr double F_PI_4 = 0.785398163397448309615660845819875721;
constexpr double F_1_PI = 0.318309886183790671537767526745028724;
constexpr double F_2_PI = 0.636619772367581343075535053490057448;
constexpr double F_2_SQRTPI = 1.12837916709551257389615890312154517;
constexpr double F_SQRT2 = 1.41421356237309504880168872420969808;
constexpr double F_SQRT1_2 = 0.707106781186547524400844362104849039;
constexpr double F_TAU = 2.0 * F_PI;

namespace d
{
constexpr double E = F_E;
constexpr double LOG2E = F_LOG2E;
constexpr double LOG10E = F_LOG10E;
constexpr double LN2 = F_LN2;
constexpr double LN10 = F_LN10;
constexpr double PI = F_PI;
constexpr double PI_2 = F_PI_2;
constexpr double PI_4 = F_PI_4;
constexpr double ONE_OVER_PI = F_1_PI;
constexpr double TWO_OVER_PI = F_2_PI;
constexpr double TWO_OVER_SQRTPI = F_2_SQRTPI;
constexpr double SQRT2 = F_SQRT2;
constexpr double SQRT1_2 = F_SQRT1_2;
constexpr double TAU = F_TAU;
constexpr double DEG_TO_RAD = F_PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / F_PI;
constexpr double ESP = 1e-6;
} // namespace d

namespace f
{
constexpr float E = d::E;
constexpr float LOG2E = d::LOG2E;
constexpr float LOG10E = d::LOG10E;
constexpr float LN2 = d::LN2;
constexpr float LN10 = d::LN10;
constexpr float PI = d::PI;
constexpr float PI_2 = d::PI_2;
constexpr float PI_4 = d::PI_4;
constexpr float ONE_OVER_PI = d::ONE_OVER_PI;
constexpr float TWO_OVER_PI = d::TWO_OVER_PI;
constexpr float TWO_OVER_SQRTPI = d::TWO_OVER_SQRTPI;
constexpr float SQRT2 = d::SQRT2;
constexpr float SQRT1_2 = d::SQRT1_2;
constexpr float TAU = d::TAU;
constexpr float DEG_TO_RAD = d::DEG_TO_RAD;
constexpr float RAD_TO_DEG = d::RAD_TO_DEG;
constexpr float ESP = 1e-6f;
} // namespace f

template<typename T>
constexpr T min(T a, T b) noexcept
{
    return a < b ? a : b;
}

template<typename T>
constexpr T max(T a, T b) noexcept
{
    return a > b ? a : b;
}

template<typename T>
constexpr T clamp(T v, T min, T max) noexcept
{
    GX_ASSERT(min <= max);
    return T(math::min(max, math::max(min, v)));
}

template<typename T>
constexpr T saturate(T v) noexcept
{
    return clamp(v, T(0), T(1));
}

template<typename T>
constexpr T mix(T x, T y, T a) noexcept
{
    return x * (T(1) - a) + y * a;
}

template<typename T>
constexpr T lerp(T x, T y, T a) noexcept
{
    return mix(x, y, a);
}

template<typename T>
constexpr T smoothstep(T e0, T e1, T x) noexcept
{
    T t = clamp((x - e0) / (e1 - e0), T(0), T(1));
    return t * t * (T(3) - T(2) * t);
}

template<typename T>
constexpr T sign(T x) noexcept
{
    return x < T(0) ? T(-1) : x > T(0) ? T(1) : T(0);
}

inline bool isZero(int32_t v)
{
    return v == 0;
}

inline bool isZero(float v)
{
    return std::abs(v) < f::ESP;
}

inline bool isZero(double v)
{
    return std::abs(v) < d::ESP;
}

inline bool isOne(int32_t v)
{
    return v == 1;
}

inline bool isOne(float v)
{
    return isZero(v - 1.0f);
}

inline bool isOne(double v)
{
    return isZero(v - 1.0);
}
}

#endif // GX_MATH_SCALAR_H
