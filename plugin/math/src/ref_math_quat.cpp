//
// Created by Gxin on 24-4-28.
//

#include "gx/math/math_la.h"

#include "gx/gany.h"


using namespace gany;
using namespace math;

template<typename T>
static void registerQuat(const std::string &name, const std::string &doc)
{
    using Q = details::TQuaternion<T>;
    using Vec2 = vec2<T>;
    using Vec3 = vec3<T>;
    using Vec4 = vec4<T>;
    Class<Q>("Math", name, doc)
            .staticFunc(MetaFunction::Init, []() {
                return Q();
            })
            .staticFunc(MetaFunction::Init, [](T w, T x, T y, T z) {
                return Q(w, x, y, z);
            })
            .staticFunc(MetaFunction::Init, [](const Vec3 &v, T w) {
                return Q(v, w);
            })
            .staticFunc(MetaFunction::Init, [](GAny v) {
                if (v.isNumber()) {
                    return Q(v.castAs<T>());
                }
                if (v.is<Vec4>()) {
                    return Q(*v.as<Vec4>());
                }
                if (v.is<Q>()) {
                    return Q(*v.as<Q>());
                }
                if (v.isUserObject()) {
                    v = v.toObject();
                }
                Q q{};
                if (v.isArray()) {
                    if (v.length() == 4) {
                        q = {v[0].castAs<T>(), v[1].castAs<T>(), v[2].castAs<T>(), v[3].castAs<T>()};
                    }
                } else if (v.isObject()) {
                    q = *caster<Q>::castTo(v).template as<Q>();
                }
                return q;
            })
            .property("x",
                      [](Q &self) { return self.x; },
                      [](Q &self, T x) { self.x = x; })
            .property("y",
                      [](Q &self) { return self.y; },
                      [](Q &self, T y) { self.y = y; })
            .property("z",
                      [](Q &self) { return self.z; },
                      [](Q &self, T z) { self.z = z; })
            .property("w",
                      [](Q &self) { return self.w; },
                      [](Q &self, T w) { self.w = w; })
            .property("xyzw",
                      [](Q &self) { return self.xyzw; },
                      [](Q &self, const Vec4 &v) { self.xyzw = v; })
            .property("xyz",
                      [](Q &self) { return self.xyz; },
                      [](Q &self, const Vec3 &v) { self.xyz = v; })
            .property("xy",
                      [](Q &self) { return self.xy; },
                      [](Q &self, const Vec2 &v) { self.xy = v; })
            .func(MetaFunction::Length, [](Q &self) {
                return self.size();
            })
            .func(MetaFunction::ToString, [](Q &self) {
                std::stringstream ss;
                ss << self.x << "," << self.y << "," << self.z << "," << self.w;
                return ss.str();
            })
            .func(MetaFunction::ToObject, [](Q &self) {
                GAny obj = {
                        self.x, self.y, self.z, self.w
                };
                return obj;
            })
            .func(MetaFunction::GetItem, [](Q &self, int32_t index) {
                return self[index];
            })
            .func(MetaFunction::SetItem, [](Q &self, int32_t index, T v) {
                self[index] = v;
            })
            .func(MetaFunction::EqualTo, [](Q &self, const Q &other) {
                return self.xyzw == other.xyzw;
            })
            .staticFunc("fromAxisAngle", [](const Vec3 &axis, T angle) {
                return Q::fromAxisAngle(axis, angle);
            })
            .staticFunc("fromDirectedRotation", [](const Vec3 &from, const Vec3 &to) {
                return Q::fromDirectedRotation(from, to);
            })
            .func(MetaFunction::Addition, [](Q &self, const Q &other) {
                return self + other;
            })
            .func(MetaFunction::Addition, [](Q &self, T other) {
                return self + other;
            })
            .func(MetaFunction::Subtraction, [](Q &self, const Q &other) {
                return self - other;
            })
            .func(MetaFunction::Subtraction, [](Q &self, T other) {
                return self - other;
            })
            .func(MetaFunction::Multiplication, [](Q &self, const Q &other) {
                return self * other;
            })
            .func(MetaFunction::Multiplication, [](Q &self, T scalar) {
                return self * scalar;
            })
            .func(MetaFunction::Multiplication, [](Q &self, const Vec3 &v) {
                return self * v;
            })
            .func(MetaFunction::Division, [](Q &self, T scalar) {
                return self / scalar;
            })
            .func(MetaFunction::Negate, [](Q &self) {
                return -self;
            })
            .func("dot", [](const Q &p, const Q &q) {
                return dot(p, q);
            })
            .func("norm", [](const Q &q) {
                return norm(q);
            })
            .func("length", [](const Q &q) {
                return length(q);
            })
            .func("length2", [](const Q &q) {
                return length2(q);
            })
            .func("normalize", [](const Q &q) {
                return normalize(q);
            })
            .func("conj", [](const Q &q) {
                return conj(q);
            })
            .func("inverse", [](const Q &q) {
                return inverse(q);
            })
            .func("real", [](const Q &q) {
                return real(q);
            })
            .func("imaginary", [](const Q &q) {
                return imaginary(q);
            })
            .func("unreal", [](const Q &q) {
                return unreal(q);
            })
            .func("cross", [](const Q &p, const Q &q) {
                return cross(p, q);
            })
            .func("exp", [](const Q &q) {
                return exp(q);
            })
            .func("log", [](const Q &q) {
                return log(q);
            })
            .func("pow", [](const Q &q, T a) {
                return pow(q, a);
            })
            .func("slerp", [](const Q &p, const Q &q, T t) {
                return slerp(p, q, t);
            })
            .func("lerp", [](const Q &p, const Q &q, T t) {
                return lerp(p, q, t);
            })
            .func("nlerp", [](const Q &p, const Q &q, T t) {
                return nlerp(p, q, t);
            })
            .func("positive", [](const Q &q) {
                return positive(q);
            });
}

void refMathQuat()
{
    registerQuat<float>("Quatf", "Math Quatf");
    registerQuat<double>("Quat", "Math Quat");
}