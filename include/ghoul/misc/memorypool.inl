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

namespace {
    constexpr const bool InjectDebugMemory = false;
} // namespace

namespace ghoul {

template <int BucketSize>
MemoryPool<BucketSize>::MemoryPool(int nBuckets)
    : _originalBucketSize(nBuckets)
{
    // Try to allocate as contiguous memory as possible
    void* memory = malloc(nBuckets * BucketSize);
    _buckets.reserve(nBuckets);
    for (int i = 0; i < nBuckets; ++i) {
        void* memPtr = reinterpret_cast<std::byte*>(memory) + (BucketSize * i);
        Bucket* b = new (memPtr) Bucket;
        _buckets.push_back(b);
    }
}

template <int BucketSize>
MemoryPool<BucketSize>::~MemoryPool() {
    for (Bucket* b : _buckets) {
        delete b;
    }
}

template <int BucketSize>
void MemoryPool<BucketSize>::reset() {
    for (Bucket* b : _buckets) {
        delete b;
    }
    _buckets.resize(_originalBucketSize);
}

template <int BucketSize>
void* MemoryPool<BucketSize>::alloc(int bytes) {
    ghoul_assert(
        bytes <= BucketSize,
        "Cannot allocate larger memory blocks than available in a bucket"
    );

    // Find the first bucket that has enough space left for the number of items
    auto it = std::find_if(
        _buckets.begin(),
        _buckets.end(),
        [bytes](Bucket* i) {
            return i->usage + bytes <= BucketSize;
        }
    );

    // No bucket had enough space, so we have to create a new one
    if (it == _buckets.end()) {
        _buckets.push_back(new Bucket);
        it = _buckets.end() - 1;
    }


    Bucket* b = *it;
    void* ptr = reinterpret_cast<std::byte*>(b->payload.data()) + b->usage;
    b->usage += bytes;

    if (InjectDebugMemory) {
        for (int ii = 0; ii < bytes; ++ii) {
            std::memset(reinterpret_cast<std::byte*>(ptr) + ii, 0xAB, 1);
        }
    }

    return ptr;
}


template <typename T, int BucketSizeItems>
std::vector<void*> TypedMemoryPool<T, BucketSizeItems>::allocate(int n) {
    // Hackish implementation to support larger allocations than number of items in a
    // bucket; probably not likely to happen
    if (n > BucketSizeItems) {
        std::vector<void*> res;
        res.reserve(n);
        while (n > BucketSizeItems) {
            std::vector<void*> r = allocate(BucketSizeItems);
            res.insert(res.end(), r.begin(), r.end());
            n -= BucketSizeItems;
        }
        std::vector<void*> r = allocate(n);
        res.insert(res.end(), r.begin(), r.end());
        return res;
    }

    std::vector<void*> res(n);
    void* ptr = MemoryPool<BucketSizeItems * sizeof(T)>::alloc(n * sizeof(T));
    for (int i = 0; i < n; ++i) {
        res[i] = reinterpret_cast<std::byte*>(ptr) + (i * sizeof(T));
    }
    return res;
}

template <typename T, int BucketSizeItems>
ReusableTypedMemoryPool<T, BucketSizeItems>::ReusableTypedMemoryPool(int nBuckets)
    : _originalNBuckets(nBuckets)
{
    _buckets.reserve(nBuckets);
    for (int i = 0; i < nBuckets; ++i) {
        _buckets.push_back(std::make_unique<Bucket>());
    }
}

template <typename T, int BucketSizeItems>
void ReusableTypedMemoryPool<T, BucketSizeItems>::reset() {
    for (Bucket* b : _buckets) {
        delete b;
    }
    _buckets.resize(_originalNBuckets);
}

template <typename T, int BucketSizeItems>
std::vector<void*> ReusableTypedMemoryPool<T, BucketSizeItems>::allocate(int n) {
    // Hackish implementation to support larger allocations than number of items in a
    // bucket; probably not likely to happen
    if (n > BucketSizeItems) {
        std::vector<void*> res;
        res.reserve(n);
        while (n > BucketSizeItems) {
            std::vector<void*> r = allocate(BucketSizeItems);
            res.insert(res.end(), r.begin(), r.end());
            n -= BucketSizeItems;
        }
        std::vector<void*> r = allocate(n);
        res.insert(res.end(), r.begin(), r.end());
        return res;
    }

    // First check if there are items in the free list, if so, return those
    if (_freeList.size() >= n) {
        std::vector<void*> res(n);
        size_t startIndex = _freeList.size() - n;
        for (int i = 0; i < n; ++i) {
            res[i] = _freeList[startIndex + i];
        }
        _freeList.erase(_freeList.begin() + startIndex, _freeList.end());
        return res;
    }

    // Find the first bucket that has enough space left for the number of items
    auto it = std::find_if(
        _buckets.begin(),
        _buckets.end(),
        [n](const std::unique_ptr<Bucket>& i) {
            return i->usage + n * sizeof(T) <= BucketSizeItems * sizeof(T);
        }
    );

    // No bucket had enough space, so we have to create a new one
    if (it == _buckets.end()) {
        _buckets.push_back(std::make_unique<Bucket>());
        it = _buckets.end() - 1;
    }


    Bucket* b = it->get();
    void* ptr = reinterpret_cast<std::byte*>(b->payload.data()) + b->usage;
    b->usage += n * sizeof(T);

    std::vector<void*> res(n);
    for (int i = 0; i < n; ++i) {
        res[i] = reinterpret_cast<std::byte*>(ptr) + (i * sizeof(T));

        if (InjectDebugMemory) {
            for (int ii = 0; ii < sizeof(T) / sizeof(std::byte); ++ii) {
                std::byte* bptr = reinterpret_cast<std::byte*>(res[i]);
                std::memset(bptr + ii, 0xAB, 1);
            }
        }
    }
    return res;
}

template <typename T, int BucketSizeItems>
void ReusableTypedMemoryPool<T, BucketSizeItems>::free(T* ptr) {
    if (ptr) {
        _freeList.push_back(ptr);
    }
}

} // namespace ghoul
