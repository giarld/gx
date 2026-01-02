//
// Created by Gxin on 2020/8/18.
//

#ifndef GX_GIDALLOCATOR_H
#define GX_GIDALLOCATOR_H

#include "gmutex.h"

#include <limits>
#include <deque>


/**
 * ID allocator
 * ID fast allocation algorithm
 */
template<typename ID_TYPE, ID_TYPE MaxNum = std::numeric_limits<ID_TYPE>::max(), typename MUTEX = GNoLock>
class GIDAllocator final
{
public:
    explicit GIDAllocator()
        : mGens(new uint8_t[MaxNum])
    {
        reset();
    }

    ~GIDAllocator()
    {
        delete[] mGens;
    }

public:
    void reset()
    {
        std::lock_guard locker(mMutex);

        mCurrentId = 1;
        std::fill_n(mGens, MaxNum, 0);
        mFreeList.clear();
    }

    ID_TYPE alloc()
    {
        std::lock_guard locker(mMutex);

        if (!mFreeList.empty()) {
            ID_TYPE id = mFreeList.front();
            mFreeList.pop_front();
            mGens[id] = 1;
            return id;
        }
        if (mCurrentId >= MaxNum) {
            return 0;
        }
        ID_TYPE id = mCurrentId++;
        mGens[id] = 1;
        return id;
    }

    void free(ID_TYPE id)
    {
        if (!isValid(id)) {
            return;
        }

        std::lock_guard locker(mMutex);

        mFreeList.push_back(id);
        mGens[id] = 0;
    }

    bool isValid(ID_TYPE id)
    {
        if (id <= 0 || id >= MaxNum) {
            return false;
        }
        return mGens[id] == 1;
    }

private:
    ID_TYPE mCurrentId = 1;
    uint8_t *const mGens;

    std::deque<ID_TYPE> mFreeList;

    MUTEX mMutex;
};

#endif //GX_GIDALLOCATOR_H
