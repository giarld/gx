//
// Created by Gxin on 2022/6/5.
//

#include "gx/reg_gx_math.h"

#include "gx/math/math_la.h"
#include "gx/math/scalar.h"
#include "gx/math/fast.h"

#include "gx/gany.h"


using namespace gany;
using namespace math;

static void registerMathProperty(GAnyClass &clazz)
{
    clazz.constant("E", d::E)
         .constant("LOG2E", d::LOG2E)
         .constant("LOG10E", d::LOG10E)
         .constant("LN2", d::LN2)
         .constant("LN10", d::LN10)
         .constant("PI", d::PI)
         .constant("PI_2", d::PI_2)
         .constant("PI_4", d::PI_4)
         .constant("ONE_OVER_PI", d::ONE_OVER_PI)
         .constant("TWO_OVER_PI", d::TWO_OVER_PI)
         .constant("TWO_OVER_SQRTPI", d::TWO_OVER_SQRTPI)
         .constant("SQRT2", d::SQRT2)
         .constant("SQRT1_2", d::SQRT1_2)
         .constant("TAU", d::TAU)
         .constant("DEG_TO_RAD", d::DEG_TO_RAD)
         .constant("RAD_TO_DEG", d::RAD_TO_DEG)
         .constant("ESP", d::ESP);
}

template<typename T>
static void registerMathFunc(GAnyClass &clazz)
{
    clazz.staticFunc("min", [](T a, T b) {
             return min(a, b);
         })
         .staticFunc("max", [](T a, T b) {
             return max(a, b);
         })
         .staticFunc("clamp", [](T v, T min, T max) {
             return clamp(v, min, max);
         })
         .staticFunc("saturate", [](T v) {
             return saturate(v);
         })
         .staticFunc("mix", [](T x, T y, T a) {
             return mix(x, y, a);
         })
         .staticFunc("lerp", [](T x, T y, T a) {
             return lerp(x, y, a);
         })
         .staticFunc("sign", [](T x) {
             return sign(x);
         });
}

static void registerMathFast(GAnyClass &clazz)
{
    clazz.staticFunc("cos", [](float x) {
             return fast::cos(x);
         })
         .staticFunc("cos", [](double x) {
             return fast::cos(x);
         })
         .staticFunc("sin", [](float x) {
             return fast::sin(x);
         })
         .staticFunc("sin", [](double x) {
             return fast::sin(x);
         })
         .staticFunc("ilog2", [](float x) {
             return fast::ilog2(x);
         })
         .staticFunc("log2", [](float x) {
             return fast::log2(x);
         })
         .staticFunc("isqrt", [](float x) {
             return fast::isqrt(x);
         })
         .staticFunc("isqrt", [](double x) {
             return fast::isqrt(x);
         })
         .staticFunc("signbit", [](float x) {
             return fast::signbit(x);
         })
         .staticFunc("pow", [](double x, uint32_t y) {
             return fast::pow(x, y);
         })
         .staticFunc("factorial", [](uint32_t x) {
             return fast::factorial(x);
         })
         .staticFunc("exp", [](float x) {
             return fast::exp(x);
         })
         .staticFunc("exp", [](double x) {
             return fast::exp(x);
         });
}

extern void refMathVec();

extern void refMathMat();

extern void refMathQuat();

REGISTER_GANY_MODULE(Math)
{
    auto classMathD = GAnyClass::Class("Math", "d", "Math double precision floating point constant");
    auto classMathFunc = GAnyClass::Class("Math", "func", "Math common functions");
    auto classMathFast = GAnyClass::Class("Math", "fast", "Math fast mathematical function");

    GAny::Export(classMathD);
    GAny::Export(classMathFunc);
    GAny::Export(classMathFast);

    registerMathProperty(*classMathD);
    registerMathFast(*classMathFast);
    registerMathFunc<float>(*classMathFunc);
    registerMathFunc<double>(*classMathFunc);
    registerMathFunc<int32_t>(*classMathFunc);
    registerMathFunc<int64_t>(*classMathFunc);
    registerMathFunc<int16_t>(*classMathFunc);
    registerMathFunc<int8_t>(*classMathFunc);

    refMathVec();
    refMathMat();
    refMathQuat();
}
