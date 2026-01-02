//
// Created by Gxin on 2020/10/22.
//

#ifndef GX_COMMON_H
#define GX_COMMON_H

#include <type_traits>


namespace gx
{
/**
 * 按位比较两个同类型变量是否相等
 *
 * @tparam T
 * @param l
 * @param r
 * @return
 */
template<typename T>
bool bitwiseEqual(const T &l, const T &r)
{
    using U =
            std::conditional_t<alignof(T) == 1,
                    std::uint8_t,
                    std::conditional_t<alignof(T) == 2,
                            std::uint16_t,
                            std::conditional_t<alignof(T) == 4,
                                    std::uint32_t,
                                    std::uint64_t
                            >
                    >
            >;

    const U *uL = reinterpret_cast<const U *>(&l);
    const U *uR = reinterpret_cast<const U *>(&r);
    for (size_t i = 0; i < sizeof(T) / sizeof(U); i++, ++uL, ++uR) {
        if (*uL != *uR) {
            return false;
        }
    }

    return true;
}


template<typename T>
void hashCombine(size_t &s, const T &v)
{
    std::hash<T> h;
    s ^= h(v) + 0x9e3779b9u + (s << 6u) + (s >> 2u);
}

/**
 * 计算Hash并合并到上一hash的结果中
 * @tparam T
 * @param last  前一个hash
 * @param v
 * @return
 */
template<typename T>
size_t hashOf(size_t last, const T &v)
{
    using U =
            std::conditional_t<alignof(T) == 1,
                    std::uint8_t,
                    std::conditional_t<alignof(T) == 2,
                            std::uint16_t,
                            std::conditional_t<alignof(T) == 4,
                                    std::uint32_t,
                                    std::uint64_t
                            >
                    >
            >;

    for (size_t i = 0; i < sizeof(T) / sizeof(U); i++) {
        hashCombine(last, reinterpret_cast<const U *>(&v)[i]);
    }

    return last;
}

/**
 * 计算hash
 * @tparam T
 * @param v
 * @return
 */
template<typename T>
size_t hashOf(const T &v)
{
    return hashOf(0, v);
}

/**
 * 以1字节对齐计算块的Hash值
 *
 * @param data  头指针
 * @param len   块长度
 * @return
 */
inline size_t hashOfByte(const void *data, size_t len)
{
    const auto *tData = static_cast<const uint8_t *>(data);
    size_t hash = 0;
    for (size_t i = 0; i < len; i++) {
        hashCombine(hash, tData[i]);
    }
    return hash;
}

/**
 * 合并hash值
 * @param master    主Hash值
 * @param branch    分支Hash值
 * @return  返回合并后的Hash值
 */
inline size_t hashMerge(size_t master, size_t branch)
{
    master ^= branch + 0x9e3779b9 + (master << 6) + (master >> 2);
    return master;
}
} // gx

#endif //GX_COMMON_H
