//
// Created by Gxin on 2021/2/28.
//

#ifndef GX_WORK_STEALING_DEQUEUE_H
#define GX_WORK_STEALING_DEQUEUE_H

#include "gx/gglobal.h"

#include "debug.h"

#include <atomic>


/*
 * 模板化、无锁、固定大小的StealingDequeue
 *
 *
 *     top                          bottom
 *      v                             v
 *      |----|----|----|----|----|----|
 *    steal()                      push(), pop()
 *  any thread                     main thread
 *
 */
template<typename TYPE, size_t COUNT>
class GWorkStealingDequeue
{
public:
    using value_type = TYPE;

    void push(TYPE item) noexcept;

    TYPE pop() noexcept;

    TYPE steal() noexcept;

    size_t getSize() const noexcept
    {
        return COUNT;
    }

    size_t getCount() const noexcept
    {
        const index_t bottom = mBottom.load(std::memory_order_relaxed);
        const index_t top = mTop.load(std::memory_order_relaxed);
        return bottom - top;
    }

private:
    static_assert(!(COUNT & (COUNT - 1)), "COUNT must be a power of two");
    constexpr static size_t MASK = COUNT - 1;

    using index_t = int64_t;

    std::atomic<index_t> mTop = {0};    // written/read in pop()/steal()
    std::atomic<index_t> mBottom = {0}; // written only in pop(), read in push(), steal()

    TYPE mItems[COUNT];

    // 直接返回引用是不安全的，所以返回拷贝
    TYPE getItemAt(index_t index) noexcept
    {
        return mItems[index & MASK];
    }

    void setItemAt(index_t index, TYPE item) noexcept
    {
        mItems[index & MASK] = item;
    }
};

/**
 * 增加一项到队列尾部
 *
 * 必须主线程(指定工作线程)执行
 */
template<typename TYPE, size_t COUNT>
void GWorkStealingDequeue<TYPE, COUNT>::push(TYPE item) noexcept
{
    const index_t bottom = mBottom.load(std::memory_order_relaxed);
    setItemAt(bottom, item);

    mBottom.store(bottom + 1, std::memory_order_release);
}

/**
 * 从尾部移除一项
 *
 * 必须主线程(指定工作线程)执行
 */
template<typename TYPE, size_t COUNT>
TYPE GWorkStealingDequeue<TYPE, COUNT>::pop() noexcept
{
    const index_t bottom = mBottom.fetch_sub(1, std::memory_order_seq_cst) - 1;

    // 如果我们尝试从空队列pop()， bottom可能是-1，后面会纠正这一点
    GX_ASSERT(bottom >= -1);

    index_t top = mTop.load(std::memory_order_seq_cst);

    if (top < bottom) {
        // 一般情况：队列非空，也不是最后一个元素时
        return getItemAt(bottom);
    }

    TYPE item{};
    if (top == bottom) {
        // 直接拿走最后一项
        item = getItemAt(bottom);

        // 因为弹出了最后一项，此时需要和steal()竞态
        if (mTop.compare_exchange_strong(top, top + 1,
                                         std::memory_order_seq_cst,
                                         std::memory_order_relaxed)) {
            // 竞态成功，我们偷到了最后一项，也就意味着steal()偷取失败了，此时只需要把top+1，将队列置空
            top++;
        } else {
            // 竞态失败，说明steal()偷取成功了，队列已经是空的了，我们只能返回一个空数据
            item = TYPE();
        }
    } else {
        GX_ASSERT(top - bottom == 1);
    }

    // mBottom的写入其实总是安全的
    mBottom.store(top, std::memory_order_relaxed);
    return item;
}

/**
 * 从队列头部窃取一个项目
 * 可以和push()、pop()同时执行
 *
 * 窃取操作一定会成功，如果队列中有项，则返回一项，否则返回空
 */
template<typename TYPE, size_t COUNT>
TYPE GWorkStealingDequeue<TYPE, COUNT>::steal() noexcept
{
    while (true) {
        // 注意：这个算法会出现_top再_bottom之前被读取(所有线程中都会如此)

        // std::memory_order_seq_cst 需要保证pop()中的顺序
        // 这不是一个典型的读取操作，其他线程中同时发生的写操作不会被发布
        index_t top = mTop.load(std::memory_order_seq_cst);

        // std::memory_order_acquire 是必须保证我们正在获取push()中发布的数据
        // std::memory_order_seq_cst 是必须保证pop()中的排序
        const index_t bottom = mBottom.load(std::memory_order_seq_cst);

        if (top >= bottom) {
            return TYPE();
        }

        TYPE item(getItemAt(top));
        if (mTop.compare_exchange_strong(top, top + 1,
                                         std::memory_order_seq_cst,
                                         std::memory_order_relaxed)) {
            return item;
        }
    }
}

#endif //GX_WORK_STEALING_DEQUEUE_H
