//
// Created by Gxin on 2021-02-24
//

#ifndef GX_MATH_VEC4_H
#define GX_MATH_VEC4_H

#include "math_compiler.h"

#include "vec3.h"
#include "half.h"

#include <gx/gbytearray.h>
#include "gx/common.h"


namespace math
{
// -------------------------------------------------------------------------------------

namespace details
{
template<typename T>
class MATH_EMPTY_BASES TVec4 :
        public TVecProductOperators<TVec4, T>,
        public TVecAddOperators<TVec4, T>,
        public TVecUnaryOperators<TVec4, T>,
        public TVecComparisonOperators<TVec4, T>,
        public TVecFunctions<TVec4, T>
{
public:
    typedef T value_type;
    typedef T &reference;
    typedef const T &const_reference;
    typedef size_t size_type;
    constexpr static size_t SIZE = 4;

    union
    {
        T v[SIZE] MATH_CONSTEXPR_INIT;
        TVec2<T> xy, st, rg;
        TVec3<T> xyz, stp, rgb;

        struct
        {
            union
            {
                T x, s, r;
            };

            union
            {
                TVec2<T> yz, tp, gb;
                TVec3<T> yzw, tpq, gba;

                struct
                {
                    union
                    {
                        T y, t, g;
                    };

                    union
                    {
                        TVec2<T> zw, pq, ba;

                        struct
                        {
                            T z, w;
                        };

                        struct
                        {
                            T p, q;
                        };

                        struct
                        {
                            T b, a;
                        };
                    };
                };
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
    MATH_DEFAULT_CTOR_CONSTEXPR TVec4() MATH_DEFAULT_CTOR

    // handles implicit conversion to a tvec4. must not be explicit.
    template<typename A, typename = enable_if_arithmetic_t<A> >
    constexpr TVec4(A v) noexcept
        : v{T(v), T(v), T(v), T(v)}
    {
    }

    template<typename A, typename B, typename C, typename D,
        typename = enable_if_arithmetic_t<A, B, C, D> >
    constexpr TVec4(A x, B y, C z, D w) noexcept
        : v{T(x), T(y), T(z), T(w)}
    {
    }

    template<typename A, typename B, typename C,
        typename = enable_if_arithmetic_t<A, B, C> >
    constexpr TVec4(const TVec2<A> &v, B z, C w) noexcept
        : v{T(v[0]), T(v[1]), T(z), T(w)}
    {
    }

    template<typename A, typename B, typename = enable_if_arithmetic_t<A, B> >
    constexpr TVec4(const TVec2<A> &v, const TVec2<B> &w) noexcept
        : v{
            T(v[0]), T(v[1]), T(w[0]), T(w[1])
        }
    {
    }

    template<typename A, typename B, typename = enable_if_arithmetic_t<A, B> >
    constexpr TVec4(const TVec3<A> &v, B w) noexcept
        : v{T(v[0]), T(v[1]), T(v[2]), T(w)}
    {
    }

    template<typename A, typename = enable_if_arithmetic_t<A> >
    constexpr TVec4(const TVec4<A> &v) noexcept
        : v{T(v[0]), T(v[1]), T(v[2]), T(v[3])}
    {
    }
};
} // namespace details

// ----------------------------------------------------------------------------------------

template<typename T, typename = details::enable_if_arithmetic_t<T> >
using vec4 = details::TVec4<T>;

using double4 = vec4<double>;
using float4 = vec4<float>;
using half4 = vec4<half>;
using int4 = vec4<int32_t>;
using uint4 = vec4<uint32_t>;
using short4 = vec4<int16_t>;
using ushort4 = vec4<uint16_t>;
using byte4 = vec4<int8_t>;
using ubyte4 = vec4<uint8_t>;
using bool4 = vec4<bool>;

// ----------------------------------------------------------------------------------------
} // namespace math

template<typename T>
GByteArray &operator<<(GByteArray &ba, const math::vec4<T> &out)
{
    ba.write(out);
    return ba;
}

template<typename T>
const GByteArray &operator>>(const GByteArray &ba, math::vec4<T> &out)
{
    ba.read(out);
    return ba;
}

namespace std
{
template<typename T>
struct hash<math::vec4<T> >
{
    size_t operator()(const math::vec4<T> &type) const
    {
        return gx::hashOf(type);
    }
};
}

#endif  // GX_MATH_VEC4_H
