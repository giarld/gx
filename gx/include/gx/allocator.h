//
// Created by Gxin on 2021/2/28.
//

#ifndef GX_ALLOCATOR_H
#define GX_ALLOCATOR_H

#include "gx/gglobal.h"

#include "memalign.h"
#include "gmutex.h"
#include "debug.h"

#include <memory.h>
#include <atomic>
#include <type_traits>


namespace pointer
{

static inline size_t alignSize(size_t size, size_t alignment)
{
    return (size + (alignment - 1)) & ~(alignment - 1);
}

template<typename P, typename T>
static P *add(P *a, T b) noexcept
{
    return (P *) (uintptr_t(a) + uintptr_t(b));
}

template<typename P>
static P *align(P *p, size_t alignment) noexcept
{
    // Alignment must be a power of 2
    GX_ASSERT(alignment && !(alignment & alignment - 1));
    return (P *) ((uintptr_t(p) + alignment - 1) & ~(alignment - 1));
}

template<typename P>
static P *align(P *p, size_t alignment, size_t offset) noexcept
{
    P *const r = align(add(p, offset), alignment);
    GX_ASSERT(r >= add(p, offset));
    return r;
}

}


/**
 * @class GLinearAllocator
 * @brief Cannot free individual blocks.
 * There is no limit on the size of each allocation, but it cannot be larger than the pre allocated space.
 * Need to manually reset the allocation pointer, suitable for memory allocation within one frame.
 */
class GLinearAllocator
{
public:
    GLinearAllocator() = default;

    /**
     * @brief Use the provided buffer.
     * @param begin
     * @param end
     */
    GLinearAllocator(void *begin, void *end) noexcept
            : mBegin(begin), mSize(reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(begin))
    {
    }

    template<typename AREA>
    explicit GLinearAllocator(const AREA &area)
            : GLinearAllocator(area.begin(), area.end())
    {}

    /**
     * @brief Allocators can't be copied.
     * @param rhs
     */
    GLinearAllocator(const GLinearAllocator &rhs) = delete;

    GLinearAllocator &operator=(const GLinearAllocator &rhs) = delete;

    /**
     * @brief Allocators can be moved.
     * @param rhs
     */
    GLinearAllocator(GLinearAllocator &&rhs) noexcept
    {
        this->swap(rhs);
    }

    GLinearAllocator &operator=(GLinearAllocator &&rhs) noexcept
    {
        if (this != &rhs) {
            this->swap(rhs);
        }
        return *this;
    }

    ~GLinearAllocator() noexcept = default;

public:
    /**
     * our allocator concept.
     * @param size
     * @param alignment
     * @param extra
     * @return
     */
    void *alloc(size_t size, size_t alignment = alignof(std::max_align_t), size_t extra = 0)
    {
        // branch-less allocation
        void *const p = pointer::align(current(), alignment, extra);
        void *const c = pointer::add(p, size);
        const bool success = c <= end();
        set_current(success ? c : current());
        return success ? p : nullptr;
    }

    /**
     * @brief Get current header pointer.
     * Unique to this dispenser.
     *
     * @return
     */
    void *getCurrent() noexcept
    {
        return current();
    }

    /**
     * @brief Fallback memory pointer to a specific point
     *
     * @param p
     */
    void rewind(void *p) noexcept
    {
        GX_ASSERT(p >= mBegin && p < end());
        set_current(p);
    }

    /**
     * @brief Roll back memory pointer to the end
     */
    void reset() noexcept
    {
        rewind(mBegin);
    }

    size_t size() const noexcept
    {
        return mCur;
    }

    size_t capacity() const noexcept
    {
        return mSize;
    }

    void swap(GLinearAllocator &rhs) noexcept
    {
        std::swap(mBegin, rhs.mBegin);
        std::swap(mSize, rhs.mSize);
        std::swap(mCur, rhs.mCur);
    }

    void *base() noexcept
    { return mBegin; }

    void free(void *, size_t) noexcept
    {}

private:
    void *end() const noexcept
    { return pointer::add(mBegin, mSize); }

    void *current() const noexcept
    { return pointer::add(mBegin, mCur); }

    void set_current(void *p) noexcept
    { mCur = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p) - reinterpret_cast<uintptr_t>(mBegin)); }

private:
    void *mBegin = nullptr;
    uint32_t mSize = 0;
    uint32_t mCur = 0;
};


/**
 * @class GHeapAllocator
 * @brief Standard heap memory allocator.
 *
 * Using alignedAlloc to allocate memory
 * Reclaiming memory using alignedFree
 */
class GHeapAllocator
{
public:
    GHeapAllocator() noexcept = default;

    template<typename AREA>
    explicit GHeapAllocator(const AREA &)
    {}

    GHeapAllocator(const GHeapAllocator &rhs) = delete;

    GHeapAllocator &operator=(const GHeapAllocator &rhs) = delete;

    GHeapAllocator(GHeapAllocator &&rhs) noexcept = default;

    GHeapAllocator &operator=(GHeapAllocator &&rhs) noexcept = default;

    ~GHeapAllocator() noexcept = default;

public:
    void *alloc(size_t size, size_t alignment = alignof(std::max_align_t), size_t extra = 0)
    {
        // This allocator does not support 'extra'
        GX_ASSERT(extra == 0);
        return gx::alignedAlloc(size, alignment);
    }

    void free(void *p) noexcept
    {
        gx::alignedFree(p);
    }

    void free(void *p, size_t) noexcept
    {
        this->free(p);
    }

    void swap(GHeapAllocator &) noexcept
    {}

    size_t size() const noexcept
    {
        return 0;
    }

    size_t capacity() const noexcept
    {
        return 0;
    }
};

// ------------------------------------------------------------------------------------------------

class GFreeList
{
public:
    GFreeList() noexcept = delete;

    GFreeList(void *begin, void *end, size_t elementSize, size_t alignment, size_t extra) noexcept
            : mElementSize(elementSize),
              mAlignment(alignment),
              mHead(init(begin, end, elementSize, alignment, extra)),
              mUserBegin(begin),
              mUserEnd(end)
    {
        size_t nodeCount = 0;
        for (const Node *c = mHead; c != nullptr; c = c->next) {
            ++nodeCount;
        }
        mCapacity = nodeCount * mElementSize;
    }

    GFreeList(size_t elementSize, size_t alignment) noexcept
            : mElementSize(elementSize),
              mAlignment(alignment)
    {
    }

    GFreeList(const GFreeList &rhs) = delete;

    ~GFreeList()
    {
        clear();
    }

    GFreeList &operator=(const GFreeList &rhs) = delete;

    GFreeList(GFreeList &&rhs) noexcept = default;

    GFreeList &operator=(GFreeList &&rhs) noexcept = default;

public:
    void *pop() noexcept
    {
        if (mHead == nullptr) {
            Node *const head = createNode();
            CHECK_CONDITION_R(head, nullptr);
            head->next = nullptr;
            mHead = head;
            mCapacity += mElementSize;
        }
        Node *const head = mHead;
        mHead = (head ? head->next : nullptr);
        ++mAllocCount;
        return head;
    }

    void push(void *p) noexcept
    {
        CHECK_CONDITION_V(p);
        Node *const head = static_cast<Node *>(p);
        head->next = mHead;
        mHead = head;
        --mAllocCount;
    }

    void *getFirst() noexcept
    {
        return mHead;
    }

    void clear() noexcept
    {
        Node *userNodeHead = nullptr;
        while (mHead) {
            Node *next = mHead->next;
            if (mHead >= mUserBegin && mHead < mUserEnd) {
                mHead->next = userNodeHead;
                userNodeHead = mHead;
            } else {
                gx::alignedFree(mHead);
                mCapacity -= mElementSize;
            }
            mHead = next;
        }
        mHead = userNodeHead;
    }

    size_t size() const noexcept
    {
        return mAllocCount * mElementSize;
    }

    size_t capacity() const noexcept
    {
        return mCapacity;
    }

private:
    struct Node
    {
        Node *next;
    };

    static Node *init(void *begin, void *end, size_t elementSize, size_t alignment, size_t extra) noexcept
    {
        void *const p = pointer::align(begin, alignment, extra);
        void *const n = pointer::align(pointer::add(p, elementSize), alignment, extra);
        GX_ASSERT(p >= begin && p < end);
        GX_ASSERT(n >= begin && n < end && n > p);

        const size_t d = reinterpret_cast<uintptr_t>(n) - reinterpret_cast<uintptr_t>(p);
        const size_t num = (reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(p)) / d;

        Node *head = static_cast<Node *>(p);

        Node *cur = head;
        for (size_t i = 1; i < num; ++i) {
            Node *next = pointer::add(cur, d);
            cur->next = next;
            cur = next;
        }
        GX_ASSERT(cur < end);
        GX_ASSERT(pointer::add(cur, d) <= end);
        cur->next = nullptr;
        return head;
    }

    Node *createNode() const noexcept
    {
        return static_cast<Node *>(gx::alignedAlloc(mElementSize, mAlignment));
    }

private:
    size_t mElementSize = 0;
    size_t mAlignment = 0;
    Node *mHead = nullptr;
    void *mUserBegin = nullptr;
    void *mUserEnd = nullptr;
    size_t mAllocCount = 0;
    size_t mCapacity = 0;
};

// ------------------------------------------------------------------------------------------------

/**
 * @class GPoolAllocator
 *
 * For memory pool allocation of fixed large and small blocks,
 * Only memory within a fixed size can be allocated at a time,
 * A certain size of continuous memory can be pre allocated as the initial memory block of the pool,
 * The default FreeList will allocate more memory after the original memory block is exhausted,
 * These memory that has not been allocated can be reclaimed using reset, and will also be reclaimed during PoolAllocator deconstruction,
 * Before disassembling PoolAllocator, it is important to free all allocated memory blocks, otherwise it will cause memory leakage
 *
 * @tparam ELEMENT_SIZE Element size (byte) must be greater than or equal to sizeof (void *)
 * @tparam ALIGNMENT    Alignment during element memory allocation
 * @tparam OFFSET       Offset for element memory alignment
 */
template<size_t ELEMENT_SIZE,
        size_t ALIGNMENT = alignof(std::max_align_t),
        size_t OFFSET = 0>
class GPoolAllocator
{
private:
    /// At least one Node pointer size must be accommodated
    static_assert(ELEMENT_SIZE >= sizeof(void *), "ELEMENT_SIZE must accommodate at least a pointer");

public:
    GPoolAllocator() noexcept
            : mFreeList(ELEMENT_SIZE, ALIGNMENT)
    {
    }

    GPoolAllocator(void *begin, void *end) noexcept
            : mFreeList(begin, end, ELEMENT_SIZE, ALIGNMENT, OFFSET)
    {
    }

    template<typename AREA>
    explicit GPoolAllocator(const AREA &area) noexcept
            : GPoolAllocator(area.begin(), area.end())
    {
    }

    GPoolAllocator(const GPoolAllocator &rhs) = delete;

    GPoolAllocator &operator=(const GPoolAllocator &rhs) = delete;

    GPoolAllocator(GPoolAllocator &&rhs) noexcept = default;

    GPoolAllocator &operator=(GPoolAllocator &&rhs) noexcept = default;

    ~GPoolAllocator() noexcept = default;

public:
    void *alloc(size_t size = ELEMENT_SIZE,
                size_t alignment = ALIGNMENT, size_t offset = OFFSET) noexcept
    {
        GX_ASSERT(size <= ELEMENT_SIZE);
        GX_ASSERT(alignment <= ALIGNMENT);
        GX_ASSERT(offset == OFFSET);
        return mFreeList.pop();
    }

    void free(void *p, size_t = ELEMENT_SIZE) noexcept
    {
        mFreeList.push(p);
    }

    size_t size() const noexcept
    {
        return mFreeList.size();
    }

    size_t capacity() const noexcept
    {
        return mFreeList.capacity();
    }

    void *getCurrent() noexcept
    {
        return mFreeList.getFirst();
    }

    void reset() noexcept
    {
        mFreeList.clear();
    }

private:
    GFreeList mFreeList;
};

template<typename T, size_t ALIGNMENT = alignof(T), size_t OFFSET = 0>
using ObjectPoolAllocator = GPoolAllocator<sizeof(T), ALIGNMENT, OFFSET>;

// ------------------------------------------------------------------------------------------------
// Areas
// ------------------------------------------------------------------------------------------------

class GHeapArea
{
public:
    GHeapArea() noexcept = default;

    explicit GHeapArea(size_t size)
    {
        if (size) {
            mBegin = malloc(size);
            mEnd = pointer::add(mBegin, size);
        }
    }

    ~GHeapArea() noexcept
    {
        free(mBegin);
    }

    GHeapArea(const GHeapArea &rhs) = delete;

    GHeapArea &operator=(const GHeapArea &rhs) = delete;

    GHeapArea(GHeapArea &&rhs) noexcept = delete;

    GHeapArea &operator=(GHeapArea &&rhs) noexcept = delete;

public:
    void *data() const noexcept
    { return mBegin; }

    void *begin() const noexcept
    { return mBegin; }

    void *end() const noexcept
    { return mEnd; }

    size_t size() const noexcept
    { return uintptr_t(mEnd) - uintptr_t(mBegin); }

    friend void swap(GHeapArea &lhs, GHeapArea &rhs) noexcept
    {
        using std::swap;
        swap(lhs.mBegin, rhs.mBegin);
        swap(lhs.mEnd, rhs.mEnd);
    }

private:
    void *mBegin = nullptr;
    void *mEnd = nullptr;
};

class GStaticArea
{
public:
    GStaticArea() noexcept = default;

    GStaticArea(void *b, void *e) noexcept
            : mBegin(b), mEnd(e)
    {
    }

    ~GStaticArea() noexcept = default;

    GStaticArea(const GStaticArea &rhs) = default;

    GStaticArea &operator=(const GStaticArea &rhs) = default;

    GStaticArea(GStaticArea &&rhs) noexcept = default;

    GStaticArea &operator=(GStaticArea &&rhs) noexcept = default;

public:
    void *data() const noexcept
    { return mBegin; }

    void *begin() const noexcept
    { return mBegin; }

    void *end() const noexcept
    { return mEnd; }

    size_t size() const noexcept
    { return uintptr_t(mEnd) - uintptr_t(mBegin); }

    friend void swap(GStaticArea &lhs, GStaticArea &rhs) noexcept
    {
        using std::swap;
        swap(lhs.mBegin, rhs.mBegin);
        swap(lhs.mEnd, rhs.mEnd);
    }

private:
    void *mBegin = nullptr;
    void *mEnd = nullptr;
};

class GNullArea
{
public:
    void *data() const noexcept
    { return nullptr; }

    size_t size() const noexcept
    { return 0; }
};

// ------------------------------------------------------------------------------------------------
// Policies
// ------------------------------------------------------------------------------------------------

namespace GLockingPolicy
{

using NoLock = GNoLock;

using Mutex = GMutex;

using SpinLock = GSpinLock;

} // namespace LockingPolicy


// ------------------------------------------------------------------------------------------------
// Arenas
// ------------------------------------------------------------------------------------------------

using GUniquePtrDeleter = std::function<void(void *)>;

template<typename T>
using GUniquePtr = std::unique_ptr<T, GUniquePtrDeleter>;

template<typename AllocatorPolicy, typename LockingPolicy, typename AreaPolicy = GHeapArea>
class GArena
{
public:
    explicit GArena(const char *name = "")
            : mName(name)
    {
    }

    /**
     * Construct a Arena with a name and allocator parameters.
     * @tparam ARGS
     * @param name
     * @param size
     * @param args
     */
    template<typename ... ARGS>
    GArena(const char *name, size_t size, ARGS &&... args)
            : mArea(size),
              mAllocator(mArea, std::forward<ARGS>(args) ...),
              mName(name)
    {
    }

    template<typename ... ARGS>
    GArena(const char *name, AreaPolicy &&area, ARGS &&... args)
            : mArea(std::forward<AreaPolicy>(area)),
              mAllocator(mArea, std::forward<ARGS>(args) ...),
              mName(name)
    {
    }

    GArena(const GArena &rhs) noexcept = delete;

    GArena &operator=(const GArena &rhs) noexcept = delete;

    GArena &operator=(GArena &&rhs) noexcept
    {
        if (this != &rhs) {
            swap(*this, rhs);
        }
        return *this;
    }

public:
    /**
     * Allocate memory based on specified size and alignment
     * Acceptable size/alignment may depend on the provided allocator.
     * @param size
     * @param alignment
     * @param extra
     * @return
     */
    void *alloc(size_t size, size_t alignment = alignof(std::max_align_t), size_t extra = 0) noexcept
    {
        GLockerGuard guard(mLock);
        void *p = mAllocator.alloc(size, alignment, extra);
        return p;
    }

    /**
     * Assign an array of destructible objects
     * For safety reasons, if the object type is not destructible,
     * we disable the object based alloc method because free() does not call the destructor.
     * @tparam T
     * @param count
     * @param alignment
     * @param extra
     * @return
     */
    template<typename T,
            typename = std::enable_if_t<std::is_trivially_destructible_v<T>>>
    T *alloc(size_t count, size_t alignment = alignof(T), size_t extra = 0) noexcept
    {
        return static_cast<T *>(alloc(count * sizeof(T), alignment, extra));
    }

    /**
     * @brief Release memory.
     * @param p
     */
    void free(void *p) noexcept
    {
        if (p) {
            GLockerGuard guard(mLock);
            mAllocator.free(p);
        }
    }

    /**
     * @brief Release memory of specified size.
     * @param p
     * @param size
     */
    void free(void *p, size_t size) noexcept
    {
        if (p) {
            GLockerGuard guard(mLock);
            mAllocator.free(p, size);
        }
    }

    /**
     * @brief
     * Reset distributor.
     * 1 Some allocators do not have a free () call, but instead use a reset () or rewind () instead;
     * 2 PoolAllocator can reclaim allocated but unused memory by executing reset().
     */
    void reset() noexcept
    {
        GLockerGuard guard(mLock);
        mAllocator.reset();
    }

    void *getCurrent() noexcept
    { return mAllocator.getCurrent(); }

    void rewind(void *addr) noexcept
    {
        GLockerGuard guard(mLock);
        mAllocator.rewind(addr);
    }

    size_t size() const noexcept
    {
        GLockerGuard guard(mLock);
        return mAllocator.size();
    }

    size_t capacity() const noexcept
    {
        GLockerGuard guard(mLock);
        return mAllocator.capacity();
    }

    /**
     * Assign and construct an object.
     * @tparam T
     * @tparam ALIGN
     * @tparam ARGS
     * @param args
     * @return
     */
    template<typename T, size_t ALIGN = alignof(T), typename... ARGS>
    T *make(ARGS &&... args) noexcept
    {
        void *const p = this->alloc(sizeof(T), ALIGN);
        return p ? new(p) T(std::forward<ARGS>(args)...) : nullptr;
    }

    /**
     * @brief Deconstruct and release an object.
     * @tparam T
     * @param p
     */
    template<typename T>
    void destroy(T *p) noexcept
    {
        if (p) {
            p->~T();
            this->free((void *) p, sizeof(T));
        }
    }

    /**
     * @brief Allocate memory for the specified type and create an object and return shared_ Ptr,
     * the object will automatically destruct and reclaim memory after the reference count is reset to zero.
     * @tparam T
     * @tparam ALIGN
     * @tparam ARGS
     * @param args
     * @return
     */
    template<typename T, size_t ALIGN = alignof(T), typename... ARGS>
    std::shared_ptr<T> makeShared(ARGS &&... args) noexcept
    {
        void *const p = this->alloc(sizeof(T), ALIGN);
        if (p) {
            return std::shared_ptr<T>(new(p) T(std::forward<ARGS>(args)...), [this](void *ptr) {
                destroy(static_cast<T *>(ptr));
            });
        }
        return nullptr;
    }

    /**
     * @brief Allocate memory for the specified type and create an object and return unique_ Ptr,
     * the object will automatically destruct and reclaim memory after being abandoned by the owner.
     * @tparam T
     * @tparam ALIGN
     * @tparam ARGS
     * @param args
     * @return
     */
    template<typename T, size_t ALIGN = alignof(T), typename... ARGS>
    GUniquePtr<T> makeUnique(ARGS &&... args) noexcept
    {
        void *const p = this->alloc(sizeof(T), ALIGN);
        if (p) {
            GUniquePtrDeleter deleter = [this](void *ptr) {
                destroy(static_cast<T *>(ptr));
            };
            return std::unique_ptr<T, GUniquePtrDeleter>(new(p) T(std::forward<ARGS>(args)...), deleter);
        }
        return nullptr;
    }

    const char *getName() const noexcept
    { return mName; }

    AllocatorPolicy &getAllocator() noexcept
    { return mAllocator; }

    const AllocatorPolicy &getAllocator() const noexcept
    { return mAllocator; }

    AreaPolicy &getArea() noexcept
    { return mArea; }

    const AreaPolicy &getArea() const noexcept
    { return mArea; }

    friend void swap(GArena &lhs, GArena &rhs) noexcept
    {
        using std::swap;
        swap(lhs.mArea, rhs.mArea);
        swap(lhs.mAllocator, rhs.mAllocator);
        swap(lhs.mLock, rhs.mLock);
        swap(lhs.mName, rhs.mName);
    }

private:
    AreaPolicy mArea;
    AllocatorPolicy mAllocator;
    mutable LockingPolicy mLock;
    const char *mName = nullptr;
};

// ------------------------------------------------------------------------------------------------

using GHeapArena = GArena<GHeapAllocator, GLockingPolicy::NoLock>;


/**
 * Splitter packaging that can be used with STL container translators
 *
 * @tparam TYPE
 * @tparam ARENA
 */
template<typename TYPE, typename ARENA>
class GSTLAllocator
{
public:
    using value_type = TYPE;
    using pointer = TYPE *;
    using const_pointer = const TYPE *;
    using reference = TYPE &;
    using const_reference = const TYPE &;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    template<typename OTHER>
    struct rebind
    {
        using other = GSTLAllocator<OTHER, ARENA>;
    };

public:
    GSTLAllocator(ARENA &arena)
            : mArena(arena)
    {}

    template<typename U>
    explicit GSTLAllocator(const GSTLAllocator<U, ARENA> &rhs)
            : mArena(rhs.mArena)
    {}

public:
    TYPE *allocate(std::size_t n)
    {
        return static_cast<TYPE *>(mArena.alloc(n * sizeof(TYPE), alignof(TYPE)));
    }

    void deallocate(TYPE *p, std::size_t n)
    {
        mArena.free(p, n * sizeof(TYPE));
    }

    template<typename U, typename A>
    bool operator==(const GSTLAllocator<U, A> &rhs) const noexcept
    {
        return std::addressof(mArena) == std::addressof(rhs.mArena);
    }

    template<typename U, typename A>
    bool operator!=(const GSTLAllocator<U, A> &rhs) const noexcept
    {
        return !operator==(rhs);
    }

private:
    template<typename U, typename A>
    friend
    class GSTLAllocator;

    ARENA &mArena;
};

#endif //GX_ALLOCATOR_H
