/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __GHOUL___MEMORYPOOL___H__
#define __GHOUL___MEMORYPOOL___H__

#include <ghoul/misc/assert.h>
#include <array>
#include <cstddef>
#include <memory>
#include <vector>

namespace ghoul {

/**
 * This class represents a MemoryPool with a specific size from which individual memory
 * blocks can be requested. The MemoryPool is organized into multiple separate buckets
 * with a specific size. The number of buckets in the MemoryPool will increase until
 * the MemoryPool is destroyed or the reset method is called.
 * OBS: If the MemoryPool is destroyed, all memory that was returned from the alloc method
 *      is freed, but if the memory was used to create objects, their destructors are not
 *      called.
 *
 * \tparam BucketSize The size of each bucket in bytes
 */
template <int BucketSize = 4096>
class MemoryPool {
public:
    /**
     * Creates the MemoryBool with the specified number of buckets already created

     * \param nBuckets the number of buckets that should be created at creation time
     */
    MemoryPool(int nBuckets = 1);

    /**
     * Frees the memory that was allocated during the existence of this MemoryPool or the
     * last call of reset.
     */
    ~MemoryPool();

    /**
     * Frees the memory that was allocated during the existence of this MemoryPool or the
     * last call of reset and returns the number of buckets to the initial number of
     * buckets as requested in the constructor.
     */
    void reset();

    /**
     * Returns a pointer to a block of memory in a bucket that is big enough to hold the
     * provided number of \p bytes. This method only calls the global allocator if the
     * existing number of blocks are not sufficient to provide the desired number of
     * bytes.
     *
     * \param bytes The number of bytes that should be reserved
     *
     * \return The pointer to the reserved memory block of \p bytes size
     *
     * \pre bytes must not be bigger than BucketSize
     */
    void* alloc(int bytes);

private:
    struct Bucket {
        std::array<std::byte, BucketSize> payload; ///< The data storage of this bucket
        int usage = 0; ///< The number of bytes that have been used in this Bucket
    };

    std::vector<Bucket*> _buckets; ///< The number of allocated buckets
    int _originalBucketSize; ///< The original desired number of buckets
};


/**
 * Similar to the MemoryPool, but instead of requesting individual bytes, this MemoryPool
 * operates instances of \tparam T. The TypedMemoryPool does not utilize any methods from
 * the provided type, but only uses the size of the type to provide a simplified interface
 * to the allocate method and BucketSizeItems parameter that operate on the number of
 * instances, rather than bytes.
 *
 * \tparam T The type for which the MemoryPool should operate
 * \tparam BucketSizeItems The number of Ts that should be stored in a single Bucket
 */
template <typename T, int BucketSizeItems = 128>
class TypedMemoryPool : private MemoryPool<BucketSizeItems * sizeof(T)> {
public:
    /**
     * Reserves a memory block that can fit \p n instances of T. Each entry in the
     * returned vector points to the memory location that is big enough to fit a single
     * instance of T.
     *
     * \param n The number of Ts that determine the size of the reserved memory block
     *
     * \return A list of pointers that each are big enough to hold a single T. These are
     *         not guaranteed to be contiguous.
     */
    std::vector<void*> allocate(int n);
};


/**
 * This memory pool works similar to the \see TypedMemoryPool execept that instances of
 * the returned pointers can be returned to make them available again for future calls of
 * the allocate method.
 *
 * \tparam T The type for which the MemoryPool should operate
 * \tparam BucketSizeItems The number of Ts that should be stored in a single Bucket
 *
 */
template <typename T, int BucketSizeItems = 128>
class ReusableTypedMemoryPool {
public:
    /**
     * Creates the MemoryBool with the specified number of buckets already created

     * \param nBuckets the number of buckets that should be created at creation time
     */
    ReusableTypedMemoryPool(int nBuckets = 1);

    /**
     * Frees the memory that was allocated during the existence of this MemoryPool or the
     * last call of reset.
     */
    ~ReusableTypedMemoryPool() = default;

    /**
     * Frees the memory that was allocated during the existence of this MemoryPool or the
     * last call of reset and returns the number of buckets to the initial number of
     * buckets as requested in the constructor.
     */
    void reset();

    /**
     * Reserves a memory block that can fit \p n instances of T. Each entry in the
     * returned vector points to the memory location that is big enough to fit a single
     * instance of T.
     *
     * \param n The number of Ts that determine the size of the reserved memory block
     *
     * \return A list of pointers that each are big enough to hold a single T. These are
     *         not guaranteed to be contiguous.
     */
    std::vector<void*> allocate(int n);

    /**
     * Returns ownership of the pointer \p ptr back to the ReusableTypedMemoryPool. This
     * pointer will be returned in a future allocate call.
     *
     * \param ptr The pointer that should be returned and marked for reuse. This pointer
     *            must be a pointer that has previously been returned by the allocate
     *            method.
     */
    void free(T* ptr);

private:
    struct Bucket {
        /// The data storage of this bucket
        std::array<std::byte, BucketSizeItems * sizeof(T)> payload;
        int usage = 0;  ///< The number of bytes that have been used in this Bucket
    };

    std::vector<T*> _freeList; ///< The list of pointers that have been returned
    std::vector<std::unique_ptr<Bucket>> _buckets;  ///< The number of allocated buckets
    int _originalNBuckets;  ///< The original desired number of buckets
};

} // namespace ghoul

#include "memorypool.inl"

#endif // __GHOUL___MEMORYPOOL___H__
