//
// Created by Gxin on 2021-02-24
//

#ifndef GX_MATH_VEC3_H
#define GX_MATH_VEC3_H

#include "math_compiler.h"

#include "vec2.h"
#include "half.h"

#include <gx/gbytearray.h>
#include "gx/common.h"


namespace math
{
namespace details
{
template<typename T>
class MATH_EMPTY_BASES TVec3 :
        public TVecProductOperators<TVec3, T>,
        public TVecAddOperators<TVec3, T>,
        public TVecUnaryOperators<TVec3, T>,
        public TVecComparisonOperators<TVec3, T>,
        public TVecFunctions<TVec3, T>
{
public:
    typedef T value_type;
    typedef T &reference;
    typedef const T &const_reference;
    typedef size_t size_type;
    constexpr static size_t SIZE = 3;

    union
    {
        T v[SIZE] MATH_CONSTEXPR_INIT;
        TVec2<T> xy;
        TVec2<T> st;
        TVec2<T> rg;

        struct
        {
            union
            {
                T x;
                T s;
                T r;
            };

            union
            {
                struct
                {
                    T y, z;
                };

                struct
                {
                    T t, p;
                };

                struct
                {
                    T g, b;
                };

                TVec2<T> yz;
                TVec2<T> tp;
                TVec2<T> gb;
            };
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
    MATH_DEFAULT_CTOR_CONSTEXPR TVec3() MATH_DEFAULT_CTOR

    // handles implicit conversion to a tvec3. must not be explicit.
    template<typename A, typename = enable_if_arithmetic_t<A> >
    constexpr TVec3(A v) noexcept
        : v{T(v), T(v), T(v)}
    {
    }

    template<typename A, typename B, typename C,
        typename = enable_if_arithmetic_t<A, B, C> >
    constexpr TVec3(A x, B y, C z) noexcept
        : v{T(x), T(y), T(z)}
    {
    }

    template<typename A, typename B, typename = enable_if_arithmetic_t<A, B> >
    constexpr TVec3(const TVec2<A> &v, B z) noexcept
        : v{T(v[0]), T(v[1]), T(z)}
    {
    }

    template<typename A, typename = enable_if_arithmetic_t<A> >
    constexpr TVec3(const TVec3<A> &v) noexcept
        : v{T(v[0]), T(v[1]), T(v[2])}
    {
    }

    // cross product works only on vectors of size 3
    template<typename U>
    friend constexpr
    TVec3<arithmetic_result_t<T, U> > cross(const TVec3 &u, const TVec3<U> &v) noexcept
    {
        return {
            u[1] * v[2] - u[2] * v[1],
            u[2] * v[0] - u[0] * v[2],
            u[0] * v[1] - u[1] * v[0]
        };
    }
};
} // namespace details

// ----------------------------------------------------------------------------------------

template<typename T, typename = details::enable_if_arithmetic_t<T> >
using vec3 = details::TVec3<T>;

using double3 = vec3<double>;
using float3 = vec3<float>;
using half3 = vec3<half>;
using int3 = vec3<int32_t>;
using uint3 = vec3<uint32_t>;
using short3 = vec3<int16_t>;
using ushort3 = vec3<uint16_t>;
using byte3 = vec3<int8_t>;
using ubyte3 = vec3<uint8_t>;
using bool3 = vec3<bool>;

// ----------------------------------------------------------------------------------------
} // namespace math

template<typename T>
GByteArray &operator<<(GByteArray &ba, const math::vec3<T> &out)
{
    ba.write(out);
    return ba;
}

template<typename T>
const GByteArray &operator>>(const GByteArray &ba, math::vec3<T> &out)
{
    ba.read(out);
    return ba;
}

namespace std
{
template<typename T>
struct hash<math::vec3<T> >
{
    size_t operator()(const math::vec3<T> &type) const
    {
        return gx::hashOf(type);
    }
};
}

#endif  // GX_MATH_VEC3_H
