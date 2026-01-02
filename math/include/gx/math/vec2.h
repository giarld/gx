//
// Created by Gxin on 2021-02-24
//

#ifndef GX_MATH_VEC2_H
#define GX_MATH_VEC2_H

#include "math_compiler.h"
#include "t_vec_helpers.h"
#include "half.h"

#include <gx/gbytearray.h>
#include "gx/common.h"

#include <type_traits>


namespace math
{
namespace details
{
template<typename T>
class MATH_EMPTY_BASES TVec2 :
        public TVecProductOperators<TVec2, T>,
        public TVecAddOperators<TVec2, T>,
        public TVecUnaryOperators<TVec2, T>,
        public TVecComparisonOperators<TVec2, T>,
        public TVecFunctions<TVec2, T>
{
public:
    typedef T value_type;
    typedef T &reference;
    typedef const T &const_reference;
    typedef size_t size_type;
    constexpr static size_t SIZE = 2;

    union
    {
        T v[SIZE] MATH_CONSTEXPR_INIT;

        struct
        {
            T x, y;
        };

        struct
        {
            T s, t;
        };

        struct
        {
            T r, g;
        };
    };

    constexpr size_type size() const
    {
        return SIZE;
    }

    // array access
    constexpr const T &operator[](size_t i) const noexcept
    {
        GX_ASSERT(i < SIZE);
        return v[i];
    }

    constexpr T &operator[](size_t i) noexcept
    {
        GX_ASSERT(i < SIZE);
        return v[i];
    }

    // constructors

    // default constructor
    MATH_DEFAULT_CTOR_CONSTEXPR TVec2() MATH_DEFAULT_CTOR

    // handles implicit conversion to a tvec4. must not be explicit.
    template<typename A, typename = enable_if_arithmetic_t<A> >
    constexpr TVec2(A v) noexcept
        : v{T(v), T(v)}
    {
    }

    template<typename A, typename B, typename = enable_if_arithmetic_t<A, B> >
    constexpr TVec2(A x, B y) noexcept
        : v{T(x), T(y)}
    {
    }

    template<typename A, typename = enable_if_arithmetic_t<A> >
    constexpr TVec2(const TVec2<A> &v) noexcept
        : v{T(v[0]), T(v[1])}
    {
    }

    // cross product works only on vectors of size 2 or 3
    template<typename U>
    friend constexpr
    arithmetic_result_t<T, U> cross(const TVec2 &u, const TVec2<U> &v) noexcept
    {
        return u[0] * v[1] - u[1] * v[0];
    }
};
} // namespace details

// ----------------------------------------------------------------------------------------

template<typename T, typename = details::enable_if_arithmetic_t<T> >
using vec2 = details::TVec2<T>;

using double2 = vec2<double>;
using float2 = vec2<float>;
using half2 = vec2<half>;
using int2 = vec2<int32_t>;
using uint2 = vec2<uint32_t>;
using short2 = vec2<int16_t>;
using ushort2 = vec2<uint16_t>;
using byte2 = vec2<int8_t>;
using ubyte2 = vec2<uint8_t>;
using bool2 = vec2<bool>;

// ----------------------------------------------------------------------------------------
} // namespace math

template<typename T>
GByteArray &operator<<(GByteArray &ba, const math::vec2<T> &out)
{
    ba.write(out);
    return ba;
}

template<typename T>
const GByteArray &operator>>(const GByteArray &ba, math::vec2<T> &out)
{
    ba.read(out);
    return ba;
}

namespace std
{
template<typename T>
struct hash<math::vec2<T> >
{
    size_t operator()(const math::vec2<T> &type) const
    {
        return gx::hashOf(type);
    }
};
}

#endif  // GX_MATH_VEC2_H
