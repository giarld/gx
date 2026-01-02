//
// Created by Gxin on 24-4-25.
//

#include "gx/math/math_la.h"

#include "gx/gany.h"


using namespace gany;
using namespace math;

template<typename V, typename T>
static void registerVecFunc(Class<V> &C)
{
    C.func("dot", [](const V &a, const V &b) {
         return dot(a, b);
     })
     .func("norm", [](const V &a) {
         return norm(a);
     })
     .func("length", [](const V &a) {
         return length(a);
     })
     .func("norm2", [](const V &a) {
         return norm2(a);
     })
     .func("length2", [](const V &a) {
         return length2(a);
     })
     .func("distance", [](const V &a, const V &b) {
         return distance(a, b);
     })
     .func("distance2", [](const V &a, const V &b) {
         return distance2(a, b);
     })
     .func("normalize", [](const V &a) {
         return normalize(a);
     })
     .func("rcp", [](const V &a) {
         return rcp(a);
     })
     .func("abs", [](const V &a) {
         return abs(a);
     })
     .func("floor", [](const V &a) {
         return floor(a);
     })
     .func("ceil", [](const V &a) {
         return ceil(a);
     })
     .func("round", [](const V &a) {
         return round(a);
     })
     .func("fmod", [](const V &a, const V &b) {
         return fmod(a, b);
     })
     .func("remainder", [](const V &a, const V &b) {
         return remainder(a, b);
     })
     .func("inverseSqrt", [](const V &a) {
         return inverseSqrt(a);
     })
     .func("sqrt", [](const V &a) {
         return sqrt(a);
     })
     .func("cbrt", [](const V &a) {
         return cbrt(a);
     })
     .func("exp", [](const V &a) {
         return exp(a);
     })
     .func("sign", [](const V &a) {
         return sign(a);
     })
     .func("pow", [](const V &v, T p) {
         return pow(v, p);
     })
     .func("pow", [](const V &a, const V &b) {
         return pow(a, b);
     })
     .func("log", [](const V &a) {
         return log(a);
     })
     .func("log10", [](const V &a) {
         return log10(a);
     })
     .func("log2", [](const V &a) {
         return log2(a);
     })
     .func("saturate", [](const V &a) {
         return saturate(a);
     })
     .func("clamp", [](const V &v, T min, T max) {
         return clamp(v, min, max);
     })
     .func("clamp", [](const V &v, const V &min, const V &max) {
         return clamp(v, min, max);
     })
     .func("fma", [](const V &lv, const V &rv, const V &a) {
         return fma(lv, rv, a);
     })
     .func("min", [](const V &a, const V &b) {
         return min(a, b);
     })
     .func("max", [](const V &a, const V &b) {
         return max(a, b);
     })
     .func("min", [](const V &a) {
         return min(a);
     })
     .func("max", [](const V &a) {
         return max(a);
     })
     .func("mix", [](const V &u, const V &v, T a) {
         return mix(u, v, a);
     })
     .staticFunc("smoothstep", [](T edge0, T edge1, const V &v) {
         return smoothstep(edge0, edge1, v);
     })
     .func("step", [](const V &v, T edge) {
         return step(edge, v);
     })
     .func("step", [](const V &edge, const V &v) {
         return step(edge, v);
     })
     .func("any", [](const V &a) {
         return any(a);
     })
     .func("all", [](const V &a) {
         return all(a);
     });
}

template<typename V, typename M, typename T>
static void registerVecOp(Class<V> &C)
{
    C.func(MetaFunction::Addition, [](V &self, const V &other) {
         return self + other;
     })
     .func(MetaFunction::Addition, [](V &self, T other) {
         return self + other;
     })
     .func(MetaFunction::Subtraction, [](V &self, const V &other) {
         return self - other;
     })
     .func(MetaFunction::Subtraction, [](V &self, T other) {
         return self - other;
     })
     .func(MetaFunction::Multiplication, [](V &self, const V &other) {
         return self * other;
     })
     .func(MetaFunction::Multiplication, [](V &self, const M &other) {
         return self * other;
     })
     .func(MetaFunction::Multiplication, [](V &self, T other) {
         return self * other;
     })
     .func(MetaFunction::Division, [](V &self, const V &other) {
         return self / other;
     })
     .func(MetaFunction::Division, [](V &self, T other) {
         return self / other;
     })
     .func(MetaFunction::Negate, [](V &self) {
         return -self;
     });
}

template<typename T>
static void registerVec2(const std::string &name, const std::string &doc)
{
    using Vec2 = vec2<T>;
    using Mat = details::TMat22<T>;
    auto &C = Class<Vec2>("Math", name, doc)
            .staticFunc(MetaFunction::Init, []() {
                return Vec2{};
            })
            .staticFunc(MetaFunction::Init, [](T x, T y) {
                return Vec2(x, y);
            })
            .staticFunc(MetaFunction::Init, [](GAny v) {
                if (v.isNumber()) {
                    return Vec2(v.castAs<T>());
                }
                if (v.is<Vec2>()) {
                    return Vec2(*v.as<Vec2>());
                }
                if (v.isUserObject()) {
                    v = v.toObject();
                }
                Vec2 vec;
                if (v.isArray()) {
                    if (v.length() >= vec.size()) {
                        for (int32_t i = 0; i < vec.size(); i++) {
                            vec[i] = v[i].castAs<T>();
                        }
                    }
                } else if (v.isObject()) {
                    vec = *caster<Vec2>::castTo(v).template as<Vec2>();
                }
                return vec;
            })
            .func("cross", [](const Vec2 &a, const Vec2 &b) {
                return cross(a, b);
            })
            .property("x",
                      [](Vec2 &self) { return self.x; },
                      [](Vec2 &self, T x) { self.x = x; })
            .property("y",
                      [](Vec2 &self) { return self.y; },
                      [](Vec2 &self, T y) { self.y = y; })
            .property("s",
                      [](Vec2 &self) { return self.s; },
                      [](Vec2 &self, T s) { self.s = s; })
            .property("t",
                      [](Vec2 &self) { return self.t; },
                      [](Vec2 &self, T t) { self.t = t; })
            .property("r",
                      [](Vec2 &self) { return self.r; },
                      [](Vec2 &self, T r) { self.r = r; })
            .property("g",
                      [](Vec2 &self) { return self.g; },
                      [](Vec2 &self, T g) { self.g = g; })
            .func(MetaFunction::Length, [](Vec2 &self) {
                return self.size();
            })
            .func(MetaFunction::ToString, [](Vec2 &self) {
                std::stringstream ss;
                ss << self.x << "," << self.y;
                return ss.str();
            })
            .func(MetaFunction::ToObject, [](Vec2 &self) {
                GAny obj = {
                        self.x, self.y
                };
                return obj;
            })
            .func(MetaFunction::GetItem, [](Vec2 &self, int32_t index) {
                return self[index];
            })
            .func(MetaFunction::SetItem, [](Vec2 &self, int32_t index, T v) {
                self[index] = v;
            })
            .func(MetaFunction::EqualTo, [](Vec2 &self, const Vec2 &other) {
                return self == other;
            })
            .func("remquo", [](const Vec2 &x, const Vec2 &y, vec2<int> *q) {
                return remquo(x, y, q);
            });
    if constexpr (std::is_same<T, float>::value
                  || std::is_same<T, double>::value
                  || std::is_same<T, int32_t>::value) {
        registerVecOp<Vec2, Mat, T>(C);
        registerVecFunc<Vec2, T>(C);
    }
}

template<typename T>
static void registerVec3(const std::string &name, const std::string &doc)
{
    using Vec3 = vec3<T>;
    using Mat = details::TMat33<T>;
    auto &C = Class<Vec3>("Math", name, doc)
            .staticFunc(MetaFunction::Init, []() {
                return Vec3{};
            })
            .staticFunc(MetaFunction::Init, [](T x, T y, T z) {
                return Vec3(x, y, z);
            })
            .staticFunc(MetaFunction::Init, [](GAny v) {
                if (v.isNumber()) {
                    return Vec3(v.castAs<T>());
                }
                if (v.is<Vec3>()) {
                    return Vec3(*v.as<Vec3>());
                }
                if (v.isUserObject()) {
                    v = v.toObject();
                }
                Vec3 vec;
                if (v.isArray()) {
                    if (v.length() >= vec.size()) {
                        for (int32_t i = 0; i < vec.size(); i++) {
                            vec[i] = v[i].castAs<T>();
                        }
                    }
                } else if (v.isObject()) {
                    vec = *caster<Vec3>::castTo(v).template as<Vec3>();
                }
                return vec;
            })
            .func("cross", [](const Vec3 &a, const Vec3 &b) {
                return cross(a, b);
            })
            .property("x",
                      [](Vec3 &self) { return self.x; },
                      [](Vec3 &self, T x) { self.x = x; })
            .property("y",
                      [](Vec3 &self) { return self.y; },
                      [](Vec3 &self, T y) { self.y = y; })
            .property("z",
                      [](Vec3 &self) { return self.z; },
                      [](Vec3 &self, T z) { self.z = z; })
            .property("s",
                      [](Vec3 &self) { return self.s; },
                      [](Vec3 &self, T s) { self.s = s; })
            .property("t",
                      [](Vec3 &self) { return self.t; },
                      [](Vec3 &self, T t) { self.t = t; })
            .property("p",
                      [](Vec3 &self) { return self.p; },
                      [](Vec3 &self, T p) { self.p = p; })
            .property("r",
                      [](Vec3 &self) { return self.r; },
                      [](Vec3 &self, T r) { self.r = r; })
            .property("g",
                      [](Vec3 &self) { return self.g; },
                      [](Vec3 &self, T g) { self.g = g; })
            .property("b",
                      [](Vec3 &self) { return self.b; },
                      [](Vec3 &self, T b) { self.b = b; })
            .func(MetaFunction::Length, [](Vec3 &self) {
                return self.size();
            })
            .func(MetaFunction::ToString, [](Vec3 &self) {
                std::stringstream ss;
                ss << self.x << "," << self.y << "," << self.z;
                return ss.str();
            })
            .func(MetaFunction::ToObject, [](Vec3 &self) {
                GAny obj = {
                        self.x, self.y, self.z
                };
                return obj;
            })
            .func(MetaFunction::GetItem, [](Vec3 &self, int32_t index) {
                return self[index];
            })
            .func(MetaFunction::SetItem, [](Vec3 &self, int32_t index, T v) {
                self[index] = v;
            })
            .func(MetaFunction::EqualTo, [](Vec3 &self, const Vec3 &other) {
                return self == other;
            })
            .func("remquo", [](const Vec3 &x, const Vec3 &y, vec3<int> *q) {
                return remquo(x, y, q);
            });

    if constexpr (std::is_same<T, float>::value
                  || std::is_same<T, double>::value
                  || std::is_same<T, int32_t>::value) {
        registerVecOp<Vec3, Mat, T>(C);
        registerVecFunc<Vec3, T>(C);
    }
}

template<typename T>
static void registerVec4(const std::string &name, const std::string &doc)
{
    using Vec4 = vec4<T>;
    using Mat = details::TMat44<T>;
    auto &C = Class<Vec4>("Math", name, doc)
            .staticFunc(MetaFunction::Init, []() {
                return Vec4{};
            })
            .staticFunc(MetaFunction::Init, [](T x, T y, T z, T w) {
                return Vec4(x, y, z, w);
            })
            .staticFunc(MetaFunction::Init, [](GAny v) {
                if (v.isNumber()) {
                    return Vec4(v.castAs<T>());
                }
                if (v.is<Vec4>()) {
                    return Vec4(*v.as<Vec4>());
                }
                if (v.isUserObject()) {
                    v = v.toObject();
                }

                Vec4 vec;
                if (v.isArray()) {
                    if (v.length() >= vec.size()) {
                        for (int32_t i = 0; i < vec.size(); i++) {
                            vec[i] = v[i].castAs<T>();
                        }
                    }
                } else if (v.isObject()) {
                    vec = *caster<Vec4>::castTo(v).template as<Vec4>();
                }
                return vec;
            })
            .property("x",
                      [](Vec4 &self) { return self.x; },
                      [](Vec4 &self, T x) { self.x = x; })
            .property("y",
                      [](Vec4 &self) { return self.y; },
                      [](Vec4 &self, T y) { self.y = y; })
            .property("z",
                      [](Vec4 &self) { return self.z; },
                      [](Vec4 &self, T z) { self.z = z; })
            .property("w",
                      [](Vec4 &self) { return self.w; },
                      [](Vec4 &self, T w) { self.w = w; })
            .property("s",
                      [](Vec4 &self) { return self.s; },
                      [](Vec4 &self, T s) { self.s = s; })
            .property("t",
                      [](Vec4 &self) { return self.t; },
                      [](Vec4 &self, T t) { self.t = t; })
            .property("p",
                      [](Vec4 &self) { return self.p; },
                      [](Vec4 &self, T p) { self.p = p; })
            .property("q",
                      [](Vec4 &self) { return self.q; },
                      [](Vec4 &self, T q) { self.q = q; })
            .property("r",
                      [](Vec4 &self) { return self.r; },
                      [](Vec4 &self, T r) { self.r = r; })
            .property("g",
                      [](Vec4 &self) { return self.g; },
                      [](Vec4 &self, T g) { self.g = g; })
            .property("b",
                      [](Vec4 &self) { return self.b; },
                      [](Vec4 &self, T b) { self.b = b; })
            .property("a",
                      [](Vec4 &self) { return self.a; },
                      [](Vec4 &self, T a) { self.a = a; })
            .func(MetaFunction::Length, [](Vec4 &self) {
                return self.size();
            })
            .func(MetaFunction::ToString, [](Vec4 &self) {
                std::stringstream ss;
                ss << self.x << "," << self.y << "," << self.z << "," << self.w;
                return ss.str();
            })
            .func(MetaFunction::ToObject, [](Vec4 &self) {
                GAny obj = {
                        self.x, self.y, self.z, self.w
                };
                return obj;
            })
            .func(MetaFunction::GetItem, [](Vec4 &self, int32_t index) {
                return self[index];
            })
            .func(MetaFunction::SetItem, [](Vec4 &self, int32_t index, T v) {
                self[index] = v;
            })
            .func(MetaFunction::EqualTo, [](Vec4 &self, const Vec4 &other) {
                return self == other;
            })
            .func("remquo", [](const Vec4 &x, const Vec4 &y, vec4<int> *q) {
                return remquo(x, y, q);
            });

    if constexpr (std::is_same<T, float>::value
                  || std::is_same<T, double>::value
                  || std::is_same<T, int32_t>::value) {
        registerVecOp<Vec4, Mat, T>(C);
        registerVecFunc<Vec4, T>(C);
    }
}

void refMathVec()
{
    registerVec2<float>("Float2", "Math Float2");
    registerVec2<double>("Double2", "Math Double2");
    registerVec2<int32_t>("Int2", "Math Int2");
    registerVec2<uint32_t>("UInt2", "Math UInt2");
    registerVec2<bool>("Bool2", "Math Bool2");

    registerVec3<float>("Float3", "Math Float3");
    registerVec3<double>("Double3", "Math Double3");
    registerVec3<int32_t>("Int3", "Math Int3");
    registerVec3<uint32_t>("UInt3", "Math UInt3");
    registerVec3<bool>("Bool3", "Math Bool3");

    registerVec4<float>("Float4", "Math Float4");
    registerVec4<double>("Double4", "Math Double4");
    registerVec4<int32_t>("Int4", "Math Int4");
    registerVec4<uint32_t>("UInt4", "Math UInt4");
    registerVec4<bool>("Bool4", "Math Bool4");
}