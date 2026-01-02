//
// Created by Gxin on 24-5-13.
//

#ifndef GX_GSLICE_H
#define GX_GSLICE_H

#include "gx/gglobal.h"

#include "debug.h"

#include <algorithm>


template<typename T>
class GSlice
{
public:
    using iterator = T *;
    using const_iterator = const T *;
    using value_type = T;
    using reference = T &;
    using const_reference = const T &;
    using pointer = T *;
    using const_pointer = const T *;
    using size_type = size_t;

    GSlice() noexcept = default;

    GSlice(const_iterator begin, const_iterator end) noexcept
        : mBegin(const_cast<iterator>(begin)),
          mEnd(const_cast<iterator>(end))
    {
    }

    GSlice(const_pointer begin, size_type count) noexcept
        : mBegin(const_cast<iterator>(begin)),
          mEnd(mBegin + count)
    {
    }

    GSlice(const GSlice &rhs) noexcept = default;

    GSlice(GSlice &&rhs) noexcept = default;

    GSlice &operator=(const GSlice &rhs) noexcept = default;

    GSlice &operator=(GSlice &&rhs) noexcept = default;

    void set(pointer begin, size_type count) noexcept
    {
        mBegin = begin;
        mEnd = begin + count;
    }

    void set(iterator begin, iterator end) noexcept
    {
        mBegin = begin;
        mEnd = end;
    }

    void swap(GSlice &rhs) noexcept
    {
        std::swap(mBegin, rhs.mBegin);
        std::swap(mEnd, rhs.mEnd);
    }

    void clear() noexcept
    {
        mBegin = nullptr;
        mEnd = nullptr;
    }

    size_t size() const noexcept
    {
        return mEnd - mBegin;
    }

    size_t sizeInBytes() const noexcept
    {
        return size() * sizeof(T);
    }

    bool empty() const noexcept
    {
        return size() == 0;
    }

    iterator begin() noexcept
    {
        return mBegin;
    }

    const_iterator begin() const noexcept
    {
        return mBegin;
    }

    const_iterator cbegin() const noexcept
    {
        return this->begin();
    }

    iterator end() noexcept
    {
        return mEnd;
    }

    const_iterator end() const noexcept
    {
        return mEnd;
    }

    const_iterator cend() const noexcept
    {
        return this->end();
    }

    reference operator[](size_t n) noexcept
    {
        GX_ASSERT(n < size());
        return mBegin[n];
    }

    const_reference operator[](size_t n) const noexcept
    {
        GX_ASSERT(n < size());
        return mBegin[n];
    }

    reference at(size_t n) noexcept
    {
        return operator[](n);
    }

    const_reference at(size_t n) const noexcept
    {
        return operator[](n);
    }

    reference front() noexcept
    {
        GX_ASSERT(!empty());
        return *mBegin;
    }

    const_reference front() const noexcept
    {
        GX_ASSERT(!empty());
        return *mBegin;
    }

    reference back() noexcept
    {
        GX_ASSERT(!empty());
        return *(this->end() - 1);
    }

    const_reference back() const noexcept
    {
        GX_ASSERT(!empty());
        return *(this->end() - 1);
    }

    pointer data() noexcept
    {
        return this->begin();
    }

    const_pointer data() const noexcept
    {
        return this->begin();
    }

    friend bool operator==(const GSlice &lhs, const GSlice &rhs)
    {
        return lhs.begin() == rhs.begin() && lhs.end() == rhs.end() && lhs.size() == rhs.size();
    }

private:
    iterator mBegin = nullptr;
    iterator mEnd = nullptr;
};

#endif //GX_GSLICE_H
