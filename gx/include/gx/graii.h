//
// Created by Gxin on 2022/10/19.
//

#ifndef GX_GRAII_H
#define GX_GRAII_H

#include <gx/gglobal.h>

#include <functional>
#include <cassert>


/**
 * 生产与释放的RAII
 * release  为释放资源的函数
 * execute  为申请资源的函数
 * commit   为是否在析构时执行释放资源的控制变量
 */
class GRaii
{
public:
    using Func = std::function<void()>;

public:
    explicit GRaii(Func release, const Func &execute = [] {
                   }, bool commit = true)
        : mIsCommit(commit),
          mRelease(std::move(release))
    {
        execute();
    }

    GRaii(GRaii &&other) noexcept
        : mIsCommit(other.mIsCommit),
          mRelease(std::move(other.mRelease))
    {
        other.mIsCommit = false;
    }

    GRaii(const GRaii &) = delete;

    GRaii &operator=(const GRaii &) = delete;

    ~GRaii()
    {
        if (mIsCommit && mRelease) {
            mRelease();
        }
    }

public:
    GRaii &commit(bool c = true)
    {
        mIsCommit = c;
        return *this;
    }

private:
    bool mIsCommit = false;
    Func mRelease;
};


/**
 * 用于实体资源管理的RAII
 * release 为释放资源的函数
 * execute 为申请资源的函数
 *
 * @tparam T
 */
template<typename T>
class GRaiiVar
{
public:
    using Self = GRaiiVar<T>;
    using ExecFunc = std::function<T()>;
    using RelFunc = std::function<void(T &)>;

public:
    explicit GRaiiVar(const ExecFunc &execute, RelFunc release, bool commit = true)
        : mIsCommit(commit),
          mResource(execute()),
          mRelease(std::move(release))
    {
    }

    GRaiiVar(GRaiiVar &&other) noexcept
        : mIsCommit(other.mIsCommit),
          mResource(std::move(other.mResource)),
          mRelease(std::move(other.mRelease))
    {
        other.mIsCommit = false;
    }

    GRaiiVar(const GRaiiVar &) = delete;

    GRaiiVar &operator=(const GRaiiVar &) = delete;

    ~GRaiiVar()
    {
        if (mIsCommit && mRelease) {
            mRelease(mResource);
        }
    }

public:
    Self &commit(bool c = true)
    {
        mIsCommit = c;
        return *this;
    }

    T &get()
    {
        return mResource;
    }

    T &operator*()
    {
        return get();
    }

    template<typename _T=T>
    std::enable_if_t<std::is_pointer_v<_T>, _T> operator->() const
    {
        return mResource;
    }

    template<typename _T=T>
    std::enable_if_t<std::is_class_v<_T>, _T *> operator->()
    {
        return std::addressof(mResource);
    }

private:
    bool mIsCommit = true;
    T mResource;
    RelFunc mRelease;
};

namespace gx
{
template<typename T>
struct no_const
{
    using type = typename std::conditional<std::is_const<T>::value, typename std::remove_const<T>::type, T>::type;
};

/**
 * 创建RAII
 *
 * @tparam RES          资源类型
 * @tparam F_REL        释放资源的成员函数地址
 * @tparam F_EXEC       申请资源的成员函数地址
 * @param res           资源
 * @param releaseFunc   释放资源的成员函数
 * @param execFunc      申请资源的成员函数
 * @param defCommit     默认回收方式
 * @return
 */
template<typename RES, typename F_REL, typename F_EXEC>
GRaii makeRAII(RES &res, F_REL releaseFunc, F_EXEC execFunc, bool defCommit = true)
{
    static_assert(std::is_class_v<RES>, "RES is not a class or struct type.");
    static_assert(std::is_member_function_pointer_v<F_REL>, "F_REL is not a member function.");
    static_assert(std::is_member_function_pointer_v<F_EXEC>, "F_EXEC is not a member function.");
    assert(releaseFunc != nullptr && execFunc != nullptr);

    auto pRes = std::addressof(const_cast<typename no_const<RES>::type &>(res));
    return GRaii(std::bind(releaseFunc, pRes), std::bind(execFunc, pRes), defCommit);
}

/**
 * 创建RAII
 * 无创建资源的版本
 *
 * @tparam RES          资源类型
 * @tparam F_REL        释放资源的成员函数地址
 * @param res           资源
 * @param releaseFunc   释放资源的成员函数
 * @param defCommit     默认回收方式
 * @return
 */
template<typename RES, typename F_REL>
GRaii makeRAII(RES &res, F_REL releaseFunc, bool defCommit = true)
{
    static_assert(std::is_class_v<RES>, "RES is not a class or struct type.");
    static_assert(std::is_member_function_pointer_v<F_REL>, "F_REL is not a member function.");
    assert(releaseFunc != nullptr);

    auto pRes = std::addressof(const_cast<typename no_const<RES>::type &>(res));
    return GRaii(std::bind(releaseFunc, pRes), [] {
    }, defCommit);
}
} // gx

#endif //GX_GRAII_H
