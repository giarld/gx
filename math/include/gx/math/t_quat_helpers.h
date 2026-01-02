//
// Created by Gxin on 2021-02-24
//


#ifndef GX_MATH_TQUATHELPERS_H
#define GX_MATH_TQUATHELPERS_H

#include "math_compiler.h"

#include "scalar.h"
#include "vec3.h"

#include <cmath>


namespace math::details
{
/*
 * No user serviceable parts here.
 *
 * Don't use this file directly, instead include math/quat.h
 */


/*
 * TQuatProductOperators implements basic arithmetic and basic compound assignment
 * operators on a quaternion of type BASE<T>.
 *
 * BASE only needs to implement operator[] and size().
 * By simply inheriting from TQuatProductOperators<BASE, T> BASE will automatically
 * get all the functionality here.
 */

template<template<typename T> class QUATERNION, typename T>
class TQuatProductOperators
{
public:
    /* compound assignment from a another quaternion of the same size but different
     * element type.
     */
    template<typename OTHER>
    constexpr QUATERNION<T> &operator*=(const QUATERNION<OTHER> &r)
    {
        QUATERNION<T> &q = static_cast<QUATERNION<T> &>(*this);
        q = q * r;
        return q;
    }

    /* compound assignment products by a scalar
     */
    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> > >
    constexpr QUATERNION<T> &operator*=(U v)
    {
        QUATERNION<T> &lhs = static_cast<QUATERNION<T> &>(*this);
        for (size_t i = 0; i < QUATERNION<T>::size(); i++) {
            lhs[i] *= v;
        }
        return lhs;
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> > >
    constexpr QUATERNION<T> &operator/=(U v)
    {
        QUATERNION<T> &lhs = static_cast<QUATERNION<T> &>(*this);
        for (size_t i = 0; i < QUATERNION<T>::size(); i++) {
            lhs[i] /= v;
        }
        return lhs;
    }


    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */

    /* The operators below handle operation between quaternions of the same size
     * but of a different element type.
     */
    template<typename U>
    friend constexpr
    QUATERNION<arithmetic_result_t<T, U> >

    operator*(
        const QUATERNION<T> &q, const QUATERNION<U> &r)
    {
        // could be written as:
        //  return QUATERNION<T>(
        //            q.w*r.w - dot(q.xyz, r.xyz),
        //            q.w*r.xyz + r.w*q.xyz + cross(q.xyz, r.xyz));
        return {
            q.w * r.w - q.x * r.x - q.y * r.y - q.z * r.z,
            q.w * r.x + q.x * r.w + q.y * r.z - q.z * r.y,
            q.w * r.y - q.x * r.z + q.y * r.w + q.z * r.x,
            q.w * r.z + q.x * r.y - q.y * r.x + q.z * r.w
        };
    }

    template<typename U>
    friend constexpr
    TVec3<arithmetic_result_t<T, U> >
    operator*(const QUATERNION<T> &q, const TVec3<U> &v)
    {
        // note: if q is known to be a unit quaternion, then this simplifies to:
        //  TVec3<T> t = 2 * cross(q.xyz, v)
        //  return v + (q.w * t) + cross(q.xyz, t)
        return imaginary(q * QUATERNION<U>(v, 0) * inverse(q));
    }


    /* For quaternions, we use explicit "by a scalar" products because it's much faster
     * than going (implicitly) through the quaternion multiplication.
     * For reference: we could use the code below instead, but it would be a lot slower.
     *  friend
     *  constexpr BASE<T> operator *(const BASE<T>& q, const BASE<T>& r) {
     *      return BASE<T>(
     *              q.w*r.w - q.x*r.x - q.y*r.y - q.z*r.z,
     *              q.w*r.x + q.x*r.w + q.y*r.z - q.z*r.y,
     *              q.w*r.y - q.x*r.z + q.y*r.w + q.z*r.x,
     *              q.w*r.z + q.x*r.y - q.y*r.x + q.z*r.w);
     *
     */
    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> > >
    friend constexpr
    QUATERNION<arithmetic_result_t<T, U> > operator*(QUATERNION<T> q, U scalar)
    {
        // don't pass q by reference because we need a copy anyway
        q *= scalar;
        return QUATERNION<arithmetic_result_t<T, U> >(q);
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> > >
    friend constexpr
    QUATERNION<arithmetic_result_t<T, U> > operator*(U scalar, QUATERNION<T> q)
    {
        // don't pass q by reference because we need a copy anyway
        q *= scalar;
        return QUATERNION<arithmetic_result_t<T, U> >(q);
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> > >
    friend constexpr
    QUATERNION<arithmetic_result_t<T, U> > operator/(QUATERNION<T> q, U scalar)
    {
        // don't pass q by reference because we need a copy anyway
        q /= scalar;
        return QUATERNION<arithmetic_result_t<T, U> >(q);
    }
};


/*
 * TQuatFunctions implements functions on a quaternion of type BASE<T>.
 *
 * BASE only needs to implement operator[] and size().
 * By simply inheriting from TQuatFunctions<BASE, T> BASE will automatically
 * get all the functionality here.
 */
template<template<typename T> class QUATERNION, typename T>
class TQuatFunctions
{
public:
    /*
     * NOTE: the functions below ARE NOT member methods. They are friend functions
     * with they definition inlined with their declaration. This makes these
     * template functions available to the compiler when (and only when) this class
     * is instantiated, at which point they're only templated on the 2nd parameter
     * (the first one, BASE<T> being known).
     */

    template<typename U>
    friend constexpr
    arithmetic_result_t<T, U> dot(
        const QUATERNION<T> &p, const QUATERNION<U> &q)
    {
        return p.x * q.x +
               p.y * q.y +
               p.z * q.z +
               p.w * q.w;
    }

    friend T norm(const QUATERNION<T> &q)
    {
        return std::sqrt(dot(q, q));
    }

    friend T length(const QUATERNION<T> &q)
    {
        return norm(q);
    }

    friend constexpr T length2(const QUATERNION<T> &q)
    {
        return dot(q, q);
    }

    friend QUATERNION<T> normalize(const QUATERNION<T> &q)
    {
        return length(q) ? q / length(q) : QUATERNION<T>(static_cast<T>(1));
    }

    friend constexpr QUATERNION<T> conj(const QUATERNION<T> &q)
    {
        return QUATERNION<T>(q.w, -q.x, -q.y, -q.z);
    }

    friend constexpr QUATERNION<T> inverse(const QUATERNION<T> &q)
    {
        return conj(q) * (T(1) / dot(q, q));
    }

    friend constexpr T real(const QUATERNION<T> &q)
    {
        return q.w;
    }

    friend constexpr TVec3<T> imaginary(const QUATERNION<T> &q)
    {
        return q.xyz;
    }

    friend constexpr QUATERNION<T> unreal(const QUATERNION<T> &q)
    {
        return QUATERNION<T>(q.xyz, 0);
    }

    template<typename U>
    friend constexpr
    QUATERNION<arithmetic_result_t<T, U> > cross(const QUATERNION<T> &p, const QUATERNION<U> &q)
    {
        return unreal(p * q);
    }

    friend QUATERNION<T> exp(const QUATERNION<T> &q)
    {
        const T nq(norm(q.xyz));
        return std::exp(q.w) * QUATERNION<T>((sin(nq) / nq) * q.xyz, cos(nq));
    }

    friend QUATERNION<T> log(const QUATERNION<T> &q)
    {
        const T nq(norm(q));
        return QUATERNION<T>((std::acos(q.w / nq) / norm(q.xyz)) * q.xyz, std::log(nq));
    }

    friend QUATERNION<T> pow(const QUATERNION<T> &q, T a)
    {
        // could also be computed as: exp(a*log(q));
        const T nq(norm(q));
        const T theta(a * std::acos(q.w / nq));
        return std::pow(nq, a) * QUATERNION<T>(normalize(q.xyz) * std::sin(theta), std::cos(theta));
    }

    friend QUATERNION<T> slerp(const QUATERNION<T> &p, const QUATERNION<T> &q, T t)
    {
        // could also be computed as: pow(q * inverse(p), t) * p;
        const T d = dot(p, q);
        const T absd = std::abs(d);
        constexpr static T value_eps = T(10) * std::numeric_limits<T>::epsilon();
        // Prevent blowing up when slerping between two quaternions that are very near each other.
        if ((T(1) - absd) < value_eps) {
            return normalize(lerp(d < 0 ? -p : p, q, t));
        }
        const T npq = std::sqrt(dot(p, p) * dot(q, q)); // ||p|| * ||q||
        const T a = std::acos(math::clamp(absd / npq, T(-1), T(1)));
        const T a0 = a * (1 - t);
        const T a1 = a * t;
        const T sina = sin(a);
        if (sina < value_eps) {
            return normalize(lerp(p, q, t));
        }
        const T isina = 1 / sina;
        const T s0 = std::sin(a0) * isina;
        const T s1 = std::sin(a1) * isina;
        // ensure we're taking the "short" side
        return normalize(s0 * p + ((d < 0) ? (-s1) : (s1)) * q);
    }

    friend constexpr QUATERNION<T> lerp(const QUATERNION<T> &p, const QUATERNION<T> &q, T t)
    {
        return ((1 - t) * p) + (t * q);
    }

    friend constexpr QUATERNION<T> nlerp(const QUATERNION<T> &p, const QUATERNION<T> &q, T t)
    {
        return normalize(lerp(p, q, t));
    }

    friend constexpr QUATERNION<T> positive(const QUATERNION<T> &q)
    {
        return q.w < 0 ? -q : q;
    }
};

// -------------------------------------------------------------------------------------
}

#endif  // GX_MATH_TQUATHELPERS_H
