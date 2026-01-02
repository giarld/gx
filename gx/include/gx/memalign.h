//
// Created by Gxin on 2021/2/28.
//

#ifndef GX_MEMALIGN_H
#define GX_MEMALIGN_H

#include "gx/platform.h"

#include "debug.h"

#include <type_traits>

#include <cstddef>
#include <cstdlib>

#if GX_PLATFORM_WINDOWS

#include <malloc.h>

#endif


namespace gx
{
inline void *alignedAlloc(size_t size, size_t align) noexcept
{
    align = (align < sizeof(void *)) ? sizeof(void *) : align;
    GX_ASSERT(align && !(align & align - 1));
    GX_ASSERT((align % sizeof(void*)) == 0);

    void *p = nullptr;

#if GX_PLATFORM_WINDOWS
    p = ::_aligned_malloc(size, align);
#else
    ::posix_memalign(&p, align, size);
#endif
    return p;
}

inline void alignedFree(void *p) noexcept
{
#if GX_PLATFORM_WINDOWS
    ::_aligned_free(p);
#else
    ::free(p);
#endif
}
} // gx

/*
 * This allocator can be used with std::vector to ensure that all projects are aligned with their alias()
 * E.g
 *
 *     template<typename T>
 *     using aligned_vector = std::vector<T, gx::STLAlignedAllocator<T>>;
 *
 *     aligned_vector<Foo> foos;
 *
 */
template<typename TYPE>
class GSTLAlignedAllocator
{
    static_assert(!(alignof(TYPE) & (alignof(TYPE) - 1)), "alignof(T) must be a power of two");

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

    template<typename T>
    struct rebind
    {
        using other = GSTLAlignedAllocator<T>;
    };

    GSTLAlignedAllocator() noexcept = default;

    template<typename T>
    explicit GSTLAlignedAllocator(const GSTLAlignedAllocator<T> &) noexcept
    {
    }

    ~GSTLAlignedAllocator() noexcept = default;

    pointer allocate(size_type n) noexcept
    {
        return (pointer) gx::alignedAlloc(n * sizeof(value_type), alignof(TYPE));
    }

    void deallocate(pointer p, size_type)
    {
        gx::alignedFree(p);
    }

    template<typename T>
    bool operator==(const GSTLAlignedAllocator<T> &) const noexcept
    {
        return true;
    }

    template<typename T>
    bool operator!=(const GSTLAlignedAllocator<T> &) const noexcept
    {
        return false;
    }
};

#endif //GX_MEMALIGN_H
