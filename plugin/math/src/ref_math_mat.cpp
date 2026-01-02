//
// Created by Gxin on 24-4-25.
//

#include "gx/math/math_la.h"

#include "gx/gany.h"

#include <sstream>


using namespace gany;
using namespace math;

template<typename M, typename T>
static void registerMatFunc(Class<M> &C)
{
    C.func("inverse", [](M &m) {
         return inverse(m);
     })
     .func("cof", [](M &m) {
         return cof(m);
     })
     .func("transpose", [](M &m) {
         return transpose(m);
     })
     .func("trace", [](M &m) {
         return trace(m);
     })
     .func("det", [](M &m) {
         return det(m);
     })
     .func("diag", [](M &m) {
         return diag(m);
     })
     .func("getColumnSize", [](M &m) {
         return m.getColumnSize();
     })
     .func("getRowSize", [](M &m) {
         return m.getRowSize();
     })
     .func("getColumnCount", [](M &m) {
         return m.getColumnCount();
     })
     .func("getRowCount", [](M &m) {
         return m.getRowCount();
     })
     .func("size", [](M &m) {
         return m.size();
     });
}

template<typename T>
static void registerMat2(const std::string &name, const std::string &doc)
{
    using Mat = details::TMat22<T>;
    using Vec2 = vec2<T>;
    auto &C = Class<Mat>("Math", name, doc)
            .staticFunc(MetaFunction::Init, []() {
                return Mat{};
            })
            .staticFunc(MetaFunction::Init, [](Mat m) {
                return m;
            })
            .staticFunc(MetaFunction::Init, [](Vec2 v) {
                return Mat(v);
            })
            .staticFunc(MetaFunction::Init, [](Vec2 c0, Vec2 c1) {
                return Mat(c0, c1);
            })
            .staticFunc(MetaFunction::Init, [](T m00, T m01, T m10, T m11) {
                return Mat(m00, m01, m10, m11);
            })
            .staticFunc(MetaFunction::Init, [](GAny v) {
                if (v.isNumber()) {
                    return Mat(v.castAs<T>());
                }
                if (v.is<Mat>()) {
                    return Mat(*v.as<Mat>());
                }
                if (v.isUserObject()) {
                    v = v.toObject();
                }
                if (v.isArray()) {
                    if (v.size() == 4) {
                        return Mat(v[0].castAs<T>(), v[1].castAs<T>(), v[2].castAs<T>(), v[3].castAs<T>());
                    }
                    if (v.size() == 2 && v[0].isArray() && v[1].isArray() && v[0].size() == 2 && v[1].size() == 2) {
                        return Mat(v[0][0].castAs<T>(), v[0][1].castAs<T>(), v[1][0].castAs<T>(), v[1][1].castAs<T>());
                    }
                }
                if (v.isObject()) {
                    return *caster<Mat>::castTo(v).template as<Mat>();
                }
                return Mat{};
            })
            .func(MetaFunction::ToString, [](Mat &self) {
                std::stringstream ss;
                ss << "{"
                   << "{" << self[0][0] << "," << self[0][1] << "},"
                   << "{" << self[1][0] << "," << self[1][1] << "},"
                   << "}";
                return ss.str();
            })
            .func(MetaFunction::ToObject, [](Mat &self) {
                GAny obj = {
                        {self[0][0], self[0][1]},
                        {self[1][0], self[1][1]}
                };
                return obj;
            })
            .func(MetaFunction::GetItem, [](Mat &self, int32_t column) {
                return self[column];
            })
            .func(MetaFunction::SetItem, [](Mat &self, int32_t column, Vec2 v) {
                self[column] = v;
            })
            .func(MetaFunction::Negate, [](Mat &self) {
                return -self;
            })
            .func(MetaFunction::EqualTo, [](Mat &self, const Mat &b) {
                return self == b;
            })
            .func(MetaFunction::Addition, [](Mat &self, const Mat &b) {
                return self + b;
            })
            .func(MetaFunction::Subtraction, [](Mat &self, const Mat &b) {
                return self - b;
            })
            .func(MetaFunction::Multiplication, [](Mat &self, const Vec2 &b) {
                return self * b;
            })
            .func(MetaFunction::Multiplication, [](Mat &self, const Mat &b) {
                return self * b;
            })
            .func(MetaFunction::Multiplication, [](Mat &self, T b) {
                return self * b;
            })
            .func(MetaFunction::Division, [](Mat &self, T b) {
                return self / b;
            })
            .staticFunc("rotate", [](T radian) {
                return Mat::rotate(radian);
            })
            .staticFunc("translation", [](Vec2 t) {
                return Mat::translation(t);
            })
            .staticFunc("scaling", [](Vec2 s) {
                return Mat::scaling(s);
            })
            .staticFunc("scaling", [](T s) {
                return Mat::scaling(s);
            })
            .property("m00",
                      [](Mat &self) {
                          return self[0][0];
                      },
                      [](Mat &self, T v) {
                          self[0][0] = v;
                      })
            .property("m01",
                      [](Mat &self) {
                          return self[0][1];
                      },
                      [](Mat &self, T v) {
                          self[0][1] = v;
                      })
            .property("m10",
                      [](Mat &self) {
                          return self[1][0];
                      },
                      [](Mat &self, T v) {
                          self[1][0] = v;
                      })
            .property("m11",
                      [](Mat &self) {
                          return self[1][1];
                      },
                      [](Mat &self, T v) {
                          self[1][1] = v;
                      });
    registerMatFunc<Mat, T>(C);
}

template<typename T>
static void registerMat3(const std::string &name, const std::string &doc)
{
    using Mat = details::TMat33<T>;
    using Vec3 = vec3<T>;
    using Quat = details::TQuaternion<T>;
    auto &C = Class<Mat>("Math", name, doc)
            .staticFunc(MetaFunction::Init, []() {
                return Mat{};
            })
            .staticFunc(MetaFunction::Init, [](Mat m) {
                return m;
            })
            .staticFunc(MetaFunction::Init, [](Vec3 v) {
                return Mat(v);
            })
            .staticFunc(MetaFunction::Init, [](Vec3 c0, Vec3 c1, Vec3 c2) {
                return Mat(c0, c1, c2);
            })
            .staticFunc(MetaFunction::Init, [](T m00, T m01, T m02, T m10, T m11, T m12, T m20, T m21, T m22) {
                return Mat(m00, m01, m02, m10, m11, m12, m20, m21, m22);
            })
            .staticFunc(MetaFunction::Init, [](GAny v) {
                if (v.isNumber()) {
                    return Mat(v.castAs<T>());
                }
                if (v.is<Mat>()) {
                    return Mat(*v.as<Mat>());
                }
                if (v.is<Quat>()) {
                    return Mat(*v.as<Quat>());
                }
                if (v.isUserObject()) {
                    v = v.toObject();
                }
                if (v.isArray()) {
                    if (v.size() == 9) {
                        return Mat(
                                v[0].castAs<T>(), v[1].castAs<T>(), v[2].castAs<T>(),
                                v[3].castAs<T>(), v[4].castAs<T>(), v[5].castAs<T>(),
                                v[6].castAs<T>(), v[7].castAs<T>(), v[8].castAs<T>()
                        );
                    }
                    if (v.size() == 3 && v[0].isArray() && v[1].isArray() && v[2].isArray()
                        && v[0].size() == 3 && v[1].size() == 3 && v[2].size() == 3) {
                        return Mat(
                                v[0][0].castAs<T>(), v[0][1].castAs<T>(), v[0][2].castAs<T>(),
                                v[1][0].castAs<T>(), v[1][1].castAs<T>(), v[1][2].castAs<T>(),
                                v[2][0].castAs<T>(), v[2][1].castAs<T>(), v[2][2].castAs<T>()
                        );
                    }
                }
                if (v.isObject()) {
                    return *caster<Mat>::castTo(v).template as<Mat>();
                }
                return Mat{};
            })
            .func(MetaFunction::ToString, [](Mat &self) {
                std::stringstream ss;
                ss << "{"
                   << "{" << self[0][0] << "," << self[0][1] << "," << self[0][2] << "},"
                   << "{" << self[1][0] << "," << self[1][1] << "," << self[1][2] << "},"
                   << "{" << self[2][0] << "," << self[2][1] << "," << self[2][2] << "}"
                   << "}";
                return ss.str();
            })
            .func(MetaFunction::ToObject, [](Mat &self) {
                GAny obj = {
                        {self[0][0], self[0][1], self[0][2]},
                        {self[1][0], self[1][1], self[1][2]}
                };
                return obj;
            })
            .func(MetaFunction::GetItem, [](Mat &self, int32_t column) {
                return self[column];
            })
            .func(MetaFunction::SetItem, [](Mat &self, int32_t column, Vec3 v) {
                self[column] = v;
            })
            .func(MetaFunction::Negate, [](Mat &self) {
                return -self;
            })
            .func(MetaFunction::EqualTo, [](Mat &self, const Mat &b) {
                return self == b;
            })
            .func(MetaFunction::Addition, [](Mat &self, const Mat &b) {
                return self + b;
            })
            .func(MetaFunction::Subtraction, [](Mat &self, const Mat &b) {
                return self - b;
            })
            .func(MetaFunction::Multiplication, [](Mat &self, const Vec3 &b) {
                return self * b;
            })
            .func(MetaFunction::Multiplication, [](Mat &self, const Mat &b) {
                return self * b;
            })
            .func(MetaFunction::Multiplication, [](Mat &self, T b) {
                return self * b;
            })
            .func(MetaFunction::Division, [](Mat &self, T b) {
                return self / b;
            })
            .func("orthogonalize", [](const Mat &self) {
                return orthogonalize(self);
            })
            .staticFunc("getTransformForNormals", [](const Mat &m) {
                return Mat::getTransformForNormals(m);
            })
            .staticFunc("lookTo", [](const Vec3 &direction, const Vec3 &up) {
                return Mat::lookTo(direction, up);
            })
            .staticFunc("packTangentFrame", [](const Mat &direction, size_t storageSize) {
                return Mat::packTangentFrame(direction, storageSize);
            })
            .staticFunc("translation", [](const Vec3 &t) {
                return Mat::translation(t);
            })
            .staticFunc("scaling", [](const Vec3 &s) {
                return Mat::scaling(s);
            })
            .staticFunc("scaling", [](T s) {
                return Mat::scaling(s);
            })
            .staticFunc("rotation", [](T radian, Vec3 about) {
                return Mat::rotation(radian, about);
            })
            .staticFunc("eulerYXZ", [](T yaw, T pitch, T roll) {
                return Mat::eulerYXZ(yaw, pitch, roll);
            })
            .staticFunc("eulerZYX", [](T yaw, T pitch, T roll) {
                return Mat::eulerZYX(yaw, pitch, roll);
            })
            .staticFunc("prescaleForNormals", [](const Mat &m) {
                return prescaleForNormals(m);
            })
            .func("toQuaternion", [](Mat &m) {
                return m.toQuaternion();
            })
            .property("m00",
                      [](Mat &self) {
                          return self[0][0];
                      },
                      [](Mat &self, T v) {
                          self[0][0] = v;
                      })
            .property("m01",
                      [](Mat &self) {
                          return self[0][1];
                      },
                      [](Mat &self, T v) {
                          self[0][1] = v;
                      })
            .property("m02",
                      [](Mat &self) {
                          return self[0][2];
                      },
                      [](Mat &self, T v) {
                          self[0][2] = v;
                      })
            .property("m10",
                      [](Mat &self) {
                          return self[1][0];
                      },
                      [](Mat &self, T v) {
                          self[1][0] = v;
                      })
            .property("m11",
                      [](Mat &self) {
                          return self[1][1];
                      },
                      [](Mat &self, T v) {
                          self[1][1] = v;
                      })
            .property("m12",
                      [](Mat &self) {
                          return self[1][2];
                      },
                      [](Mat &self, T v) {
                          self[1][2] = v;
                      })
            .property("m22",
                      [](Mat &self) {
                          return self[2][2];
                      },
                      [](Mat &self, T v) {
                          self[2][2] = v;
                      })
            .func("toQuaternion", [](Mat &self) {
                return self.toQuaternion();
            });
    registerMatFunc<Mat, T>(C);
}

template<typename T>
static void registerMat4(const std::string &name, const std::string &doc)
{
    using Mat = details::TMat44<T>;
    using Mat3 = details::TMat33<T>;
    using Vec4 = vec4<T>;
    using Vec3 = vec3<T>;
    using Quat = details::TQuaternion<T>;
    auto &C = Class<Mat>("Math", name, doc)
            .staticFunc(MetaFunction::Init, []() {
                return Mat{};
            })
            .staticFunc(MetaFunction::Init, [](Mat m) {
                return m;
            })
            .staticFunc(MetaFunction::Init, [](Vec4 v) {
                return Mat(v);
            })
            .staticFunc(MetaFunction::Init, [](Vec4 c0, Vec4 c1, Vec4 c2, Vec4 c3) {
                return Mat(c0, c1, c2, c3);
            })
            .staticFunc(MetaFunction::Init, [](
                    T m00, T m01, T m02, T m03,
                    T m10, T m11, T m12, T m13,
                    T m20, T m21, T m22, T m23,
                    T m30, T m31, T m32, T m33) {
                return Mat(
                        m00, m01, m02, m03,
                        m10, m11, m12, m13,
                        m20, m21, m22, m23,
                        m30, m31, m32, m33
                );
            })
            .staticFunc(MetaFunction::Init, [](const Mat3 &m, const Vec3 &translation) {
                return Mat(m, translation);
            })
            .staticFunc(MetaFunction::Init, [](const Mat3 &m, const Vec4 &column3) {
                return Mat(m, column3);
            })
            .staticFunc(MetaFunction::Init, [](GAny v) {
                if (v.isNumber()) {
                    return Mat(v.castAs<T>());
                }
                if (v.is<Mat>()) {
                    return Mat(*v.as<Mat>());
                }
                if (v.is<Quat>()) {
                    return Mat(*v.as<Quat>());
                }
                if (v.is<Mat3>()) {
                    return Mat(*v.as<Mat3>());
                }
                if (v.isUserObject()) {
                    v = v.toObject();
                }
                if (v.isArray()) {
                    if (v.size() == 16) {
                        return Mat(
                                v[0].castAs<T>(), v[1].castAs<T>(), v[2].castAs<T>(), v[3].castAs<T>(),
                                v[4].castAs<T>(), v[5].castAs<T>(), v[6].castAs<T>(), v[7].castAs<T>(),
                                v[8].castAs<T>(), v[9].castAs<T>(), v[10].castAs<T>(), v[11].castAs<T>(),
                                v[12].castAs<T>(), v[13].castAs<T>(), v[14].castAs<T>(), v[15].castAs<T>()
                        );
                    }
                    if (v.size() == 4 && v[0].isArray() && v[1].isArray() && v[2].isArray() && v[3].isArray()
                        && v[0].size() == 4 && v[1].size() == 4 && v[2].size() == 4 && v[3].size() == 4) {
                        return Mat(
                                v[0][0].castAs<T>(), v[0][1].castAs<T>(), v[0][2].castAs<T>(), v[0][3].castAs<T>(),
                                v[1][0].castAs<T>(), v[1][1].castAs<T>(), v[1][2].castAs<T>(), v[1][3].castAs<T>(),
                                v[2][0].castAs<T>(), v[2][1].castAs<T>(), v[2][2].castAs<T>(), v[2][3].castAs<T>(),
                                v[3][0].castAs<T>(), v[3][1].castAs<T>(), v[3][2].castAs<T>(), v[3][3].castAs<T>()
                        );
                    }
                }
                if (v.isObject()) {
                    return *caster<Mat>::castTo(v).template as<Mat>();
                }
                return Mat{};
            })
            .func(MetaFunction::ToString, [](Mat &self) {
                std::stringstream ss;
                ss << "{"
                   << "{" << self[0][0] << "," << self[0][1] << "," << self[0][2] << "," << self[0][3] << "},"
                   << "{" << self[1][0] << "," << self[1][1] << "," << self[1][2] << "," << self[1][3] << "},"
                   << "{" << self[2][0] << "," << self[2][1] << "," << self[2][2] << "," << self[2][3] << "},"
                   << "{" << self[3][0] << "," << self[3][1] << "," << self[3][2] << "," << self[3][3] << "}"
                   << "}";
                return ss.str();
            })
            .func(MetaFunction::ToObject, [](Mat &self) {
                GAny obj = {
                        {self[0][0], self[0][1], self[0][2], self[0][3]},
                        {self[1][0], self[1][1], self[1][2], self[1][3]},
                        {self[2][0], self[2][1], self[2][2], self[2][3]},
                        {self[3][0], self[3][1], self[3][2], self[3][3]}
                };
                return obj;
            })
            .func(MetaFunction::GetItem, [](Mat &self, int32_t column) {
                return self[column];
            })
            .func(MetaFunction::SetItem, [](Mat &self, int32_t column, Vec4 v) {
                self[column] = v;
            })
            .func(MetaFunction::Negate, [](Mat &self) {
                return -self;
            })
            .func(MetaFunction::EqualTo, [](Mat &self, const Mat &b) {
                return self == b;
            })
            .func(MetaFunction::Addition, [](Mat &self, const Mat &b) {
                return self + b;
            })
            .func(MetaFunction::Subtraction, [](Mat &self, const Mat &b) {
                return self - b;
            })
            .func(MetaFunction::Multiplication, [](Mat &self, const Vec3 &b) {
                return self * b;
            })
            .func(MetaFunction::Multiplication, [](Mat &self, const Vec4 &b) {
                return self * b;
            })
            .func(MetaFunction::Multiplication, [](Mat &self, const Mat &b) {
                return self * b;
            })
            .func(MetaFunction::Multiplication, [](Mat &self, T b) {
                return self * b;
            })
            .func(MetaFunction::Division, [](Mat &self, T b) {
                return self / b;
            })
            .defEnum({
                    {"HORIZONTAL", Mat::Fov::HORIZONTAL},
                    {"VERTICAL",   Mat::Fov::VERTICAL},
            })
            .staticFunc("ortho", [](T left, T right, T bottom, T top, T near, T far) {
                return Mat::ortho(left, right, bottom, top, near, far);
            })
            .staticFunc("frustum", [](T left, T right, T bottom, T top, T near, T far) {
                return Mat::frustum(left, right, bottom, top, near, far);
            })
            .staticFunc("perspective", [](T fov, T aspect, T near, T far, typename Mat::Fov direction) {
                return Mat::perspective(fov, aspect, near, far, direction);
            })
            .staticFunc("perspective", [](T fov, T aspect, T near, T far) {
                return Mat::perspective(fov, aspect, near, far);
            })
            .staticFunc("lookAt", [](const Vec3 &eye, const Vec3 &center, const Vec3 &up) {
                return Mat::lookAt(eye, center, up);
            })
            .staticFunc("lookTo", [](const Vec3 &direction, const Vec3 &position, const Vec3 &up) {
                return Mat::lookTo(direction, position, up);
            })
            .staticFunc("project", [](const Mat &projectionMatrix, const Vec3 &vertex) {
                return Mat::project(projectionMatrix, vertex);
            })
            .staticFunc("project", [](const Mat &projectionMatrix, const Vec4 &vertex) {
                return Mat::project(projectionMatrix, vertex);
            })
            .func("upperLeft", [](const Mat &self) {
                return self.upperLeft();
            })
            .staticFunc("translation", [](const Vec3 &t) {
                return Mat::translation(t);
            })
            .staticFunc("scaling", [](const Vec3 &s) {
                return Mat::scaling(s);
            })
            .staticFunc("scaling", [](T s) {
                return Mat::scaling(s);
            })
            .staticFunc("rotation", [](T radian, Vec4 about) {
                return Mat::rotation(radian, about);
            })
            .staticFunc("rotation", [](T radian, Vec3 about) {
                return Mat::rotation(radian, about);
            })
            .staticFunc("eulerYXZ", [](T yaw, T pitch, T roll) {
                return Mat::eulerYXZ(yaw, pitch, roll);
            })
            .staticFunc("eulerZYX", [](T yaw, T pitch, T roll) {
                return Mat::eulerZYX(yaw, pitch, roll);
            })
            .func("toQuaternion", [](const Mat &m) {
                return m.toQuaternion();
            })
            .property("m00",
                      [](Mat &self) {
                          return self[0][0];
                      },
                      [](Mat &self, T v) {
                          self[0][0] = v;
                      })
            .property("m01",
                      [](Mat &self) {
                          return self[0][1];
                      },
                      [](Mat &self, T v) {
                          self[0][1] = v;
                      })
            .property("m02",
                      [](Mat &self) {
                          return self[0][2];
                      },
                      [](Mat &self, T v) {
                          self[0][2] = v;
                      })
            .property("m03",
                      [](Mat &self) {
                          return self[0][3];
                      },
                      [](Mat &self, T v) {
                          self[0][3] = v;
                      })
            .property("m10",
                      [](Mat &self) {
                          return self[1][0];
                      },
                      [](Mat &self, T v) {
                          self[1][0] = v;
                      })
            .property("m11",
                      [](Mat &self) {
                          return self[1][1];
                      },
                      [](Mat &self, T v) {
                          self[1][1] = v;
                      })
            .property("m12",
                      [](Mat &self) {
                          return self[1][2];
                      },
                      [](Mat &self, T v) {
                          self[1][2] = v;
                      })
            .property("m13",
                      [](Mat &self) {
                          return self[1][3];
                      },
                      [](Mat &self, T v) {
                          self[1][3] = v;
                      })
            .property("m20",
                      [](Mat &self) {
                          return self[2][0];
                      },
                      [](Mat &self, T v) {
                          self[2][0] = v;
                      })
            .property("m21",
                      [](Mat &self) {
                          return self[2][1];
                      },
                      [](Mat &self, T v) {
                          self[2][1] = v;
                      })
            .property("m22",
                      [](Mat &self) {
                          return self[2][2];
                      },
                      [](Mat &self, T v) {
                          self[2][2] = v;
                      })
            .property("m23",
                      [](Mat &self) {
                          return self[2][3];
                      },
                      [](Mat &self, T v) {
                          self[2][3] = v;
                      })
            .property("m30",
                      [](Mat &self) {
                          return self[3][0];
                      },
                      [](Mat &self, T v) {
                          self[3][0] = v;
                      })
            .property("m31",
                      [](Mat &self) {
                          return self[3][1];
                      },
                      [](Mat &self, T v) {
                          self[3][1] = v;
                      })
            .property("m32",
                      [](Mat &self) {
                          return self[3][2];
                      },
                      [](Mat &self, T v) {
                          self[3][2] = v;
                      })
            .property("m33",
                      [](Mat &self) {
                          return self[3][3];
                      },
                      [](Mat &self, T v) {
                          self[3][3] = v;
                      })
            .func("toQuaternion", [](Mat &self) {
                return self.toQuaternion();
            });
    registerMatFunc<Mat, T>(C);
}

void refMathMat()
{
    registerMat2<float>("Mat2f", "Math Mat2f");
    registerMat2<double>("Mat2", "Math Mat2");

    registerMat3<float>("Mat3f", "Math Mat3f");
    registerMat3<double>("Mat3", "Math Mat3");

    registerMat4<float>("Mat4f", "GxMath Mat4f");
    registerMat4<double>("Mat4", "GxMath Mat4");
}