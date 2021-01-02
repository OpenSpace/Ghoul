/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2021                                                               *
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

#include "catch2/catch.hpp"

#include <ghoul/misc/memorypool.h>

TEST_CASE("MemoryPool: MemoryPool Default", "[memorypool]") {
    ghoul::MemoryPool<> pool;
    void* p1 = pool.allocate(1024);
    std::memset(p1, 0xB0, 1024);
    REQUIRE(p1);
    REQUIRE(pool.nBuckets() == 1);
    REQUIRE(pool.occupancies().size() == 1);
    REQUIRE(pool.occupancies()[0] == 1024);
    REQUIRE(pool.totalOccupancy() == 1024);

    void* p2 = pool.allocate(1024);
    std::memset(p2, 0xB1, 1024);
    REQUIRE(p2);
    REQUIRE(p2 != p1);
    REQUIRE(pool.nBuckets() == 1);
    REQUIRE(pool.occupancies().size() == 1);
    REQUIRE(pool.occupancies()[0] == 2048);
    REQUIRE(pool.totalOccupancy() == 2048);

    void* p3 = pool.allocate(1024);
    std::memset(p3, 0xB2, 1024);
    REQUIRE(p3);
    REQUIRE(p3 != p1);
    REQUIRE(p3 != p2);
    REQUIRE(pool.nBuckets() == 1);
    REQUIRE(pool.occupancies().size() == 1);
    REQUIRE(pool.occupancies()[0] == 3072);
    REQUIRE(pool.totalOccupancy() == 3072);

    void* p4 = pool.allocate(1024);
    std::memset(p4, 0xB3, 1024);
    REQUIRE(p4);
    REQUIRE(p4 != p1);
    REQUIRE(p4 != p2);
    REQUIRE(p4 != p3);
    REQUIRE(pool.nBuckets() == 1);
    REQUIRE(pool.occupancies().size() == 1);
    REQUIRE(pool.occupancies()[0] == 4096);
    REQUIRE(pool.totalOccupancy() == 4096);
}

TEST_CASE("MemoryPool: MemoryPool 2048 Bucket", "[memorypool]") {
    ghoul::MemoryPool<2048> pool;
    void* p1 = pool.allocate(1024);
    std::memset(p1, 0xB0, 1024);
    REQUIRE(p1);
    REQUIRE(pool.nBuckets() == 1);
    REQUIRE(pool.occupancies().size() == 1);
    REQUIRE(pool.occupancies()[0] == 1024);
    REQUIRE(pool.totalOccupancy() == 1024);

    void* p2 = pool.allocate(1024);
    std::memset(p2, 0xB1, 1024);
    REQUIRE(p2);
    REQUIRE(p2 != p1);
    REQUIRE(pool.nBuckets() == 1);
    REQUIRE(pool.occupancies().size() == 1);
    REQUIRE(pool.occupancies()[0] == 2048);
    REQUIRE(pool.totalOccupancy() == 2048);

    void* p3 = pool.allocate(1024);
    std::memset(p3, 0xB2, 1024);
    REQUIRE(p3);
    REQUIRE(p3 != p1);
    REQUIRE(p3 != p2);
    REQUIRE(pool.nBuckets() == 2);
    REQUIRE(pool.occupancies().size() == 2);
    REQUIRE(pool.occupancies()[0] == 2048);
    REQUIRE(pool.occupancies()[1] == 1024);
    REQUIRE(pool.totalOccupancy() == 2048 + 1024);

    void* p4 = pool.allocate(1024);
    std::memset(p4, 0xB3, 1024);
    REQUIRE(p4);
    REQUIRE(p4 != p1);
    REQUIRE(p4 != p2);
    REQUIRE(p4 != p3);
    REQUIRE(pool.nBuckets() == 2);
    REQUIRE(pool.occupancies().size() == 2);
    REQUIRE(pool.occupancies()[0] == 2048);
    REQUIRE(pool.occupancies()[1] == 2048);
    REQUIRE(pool.totalOccupancy() == 2048 + 2048);
}

TEST_CASE("MemoryPool: MemoryPool 2048 Bucket Pre-Alloc", "[memorypool]") {
    ghoul::MemoryPool<2048> pool(2);
    void* p1 = pool.allocate(1024);
    std::memset(p1, 0xB0, 1024);
    REQUIRE(p1);
    REQUIRE(pool.nBuckets() == 2);
    REQUIRE(pool.occupancies().size() == 2);
    REQUIRE(pool.occupancies()[0] == 1024);
    REQUIRE(pool.occupancies()[1] == 0);
    REQUIRE(pool.totalOccupancy() == 1024);

    void* p2 = pool.allocate(1024);
    std::memset(p2, 0xB1, 1024);
    REQUIRE(p2);
    REQUIRE(p2 != p1);
    REQUIRE(pool.nBuckets() == 2);
    REQUIRE(pool.occupancies().size() == 2);
    REQUIRE(pool.occupancies()[0] == 2048);
    REQUIRE(pool.occupancies()[1] == 0);
    REQUIRE(pool.totalOccupancy() == 2048);

    void* p3 = pool.allocate(1024);
    std::memset(p3, 0xB2, 1024);
    REQUIRE(p3);
    REQUIRE(p3 != p1);
    REQUIRE(p3 != p2);
    REQUIRE(pool.nBuckets() == 2);
    REQUIRE(pool.occupancies().size() == 2);
    REQUIRE(pool.occupancies()[0] == 2048);
    REQUIRE(pool.occupancies()[1] == 1024);
    REQUIRE(pool.totalOccupancy() == 2048 + 1024);

    void* p4 = pool.allocate(1024);
    std::memset(p4, 0xB3, 1024);
    REQUIRE(p4);
    REQUIRE(p4 != p1);
    REQUIRE(p4 != p2);
    REQUIRE(p4 != p3);
    REQUIRE(pool.nBuckets() == 2);
    REQUIRE(pool.occupancies().size() == 2);
    REQUIRE(pool.occupancies()[0] == 2048);
    REQUIRE(pool.occupancies()[1] == 2048);
    REQUIRE(pool.totalOccupancy() == 2048 + 2048);
}

TEST_CASE("MemoryPool: MemoryPool 2048 Reusing pointers", "[memorypool]") {
    ghoul::MemoryPool<2048> pool;

    void* p1 = pool.allocate(16);
    void* p2 = pool.allocate(8);

    pool.deallocate(p1, 16);
    void* p3 = pool.allocate(8);
    void* p4 = pool.allocate(8);

    void* p5 = pool.allocate(8);

    REQUIRE(pool.totalOccupancy() == 32);
    REQUIRE(reinterpret_cast<intptr_t>(p1) == reinterpret_cast<intptr_t>(p3));
    REQUIRE(reinterpret_cast<intptr_t>(p4) == reinterpret_cast<intptr_t>(p3) + 8);
    REQUIRE(reinterpret_cast<intptr_t>(p2) > reinterpret_cast<intptr_t>(p4));
    REQUIRE(reinterpret_cast<intptr_t>(p5) > reinterpret_cast<intptr_t>(p2));
}

TEST_CASE("MemoryPool: MemoryPool Reusing pointers w/o fragmentation", "[memorypool]") {
    ghoul::MemoryPool<32> pool;

    REQUIRE(pool.occupancies().size() == 1);
    REQUIRE(pool.occupancies()[0] == 0);
    void* p1 = pool.allocate(8);
    void* p2 = pool.allocate(8);
    [[ maybe_unused ]] void* p3 = pool.allocate(16);
    REQUIRE(pool.occupancies().size() == 1);
    REQUIRE(pool.occupancies()[0] == 32);

    pool.deallocate(p1, 8);
    pool.deallocate(p2, 8);

    // The call to housekeeping will merge those two deallocated and make it possible to
    // allocate a 16 byte block in the place of the 2 8-byte blocks
    pool.housekeeping();

    void* p5 = pool.allocate(16);
    REQUIRE(reinterpret_cast<intptr_t>(p1) == reinterpret_cast<intptr_t>(p5));
    REQUIRE(pool.occupancies().size() == 1);
    REQUIRE(pool.occupancies()[0] == 32);
}

TEST_CASE("MemoryPool: Reusable Typed MemoryPool", "[memorypool]") {
    ghoul::ReusableTypedMemoryPool<int> pool;
    std::vector<void*> p1 = pool.allocate(2);
    REQUIRE(p1.size() == 2);
    REQUIRE(p1[0] != p1[1]);

    std::vector<void*> p2 = pool.allocate(2);
    REQUIRE(p2.size() == 2);
    REQUIRE(p2[0] != p2[1]);

    std::vector<void*> p3 = pool.allocate(2);
    REQUIRE(p3.size() == 2);
    REQUIRE(p3[0] != p3[1]);

    std::vector<void*> p4 = pool.allocate(2);
    REQUIRE(p4.size() == 2);
    REQUIRE(p4[0] != p4[1]);

    pool.free(reinterpret_cast<int*>(p1[0]));
}

TEST_CASE("MemoryPool: Reusable Typed MemoryPool Reuse", "[memorypool]") {
    ghoul::ReusableTypedMemoryPool<int> pool;
    std::vector<void*> p1 = pool.allocate(2);
    REQUIRE(p1.size() == 2);
    REQUIRE(p1[0] != p1[1]);

    std::vector<void*> p2 = pool.allocate(2);
    REQUIRE(p2.size() == 2);
    REQUIRE(p2[0] != p2[1]);

    pool.free(reinterpret_cast<int*>(p1[0]));
    pool.free(reinterpret_cast<int*>(p1[1]));
    pool.free(reinterpret_cast<int*>(p2[0]));
    pool.free(reinterpret_cast<int*>(p2[1]));

    std::vector<void*> p3 = pool.allocate(2);
    REQUIRE(p3.size() == 2);
    REQUIRE(p3[0] != p3[1]);

    std::vector<void*> p4 = pool.allocate(2);
    REQUIRE(p4.size() == 2);
    REQUIRE(p4[0] != p4[1]);

    REQUIRE(p3[0] == p2[0]);
    REQUIRE(p3[1] == p2[1]);
    REQUIRE(p4[0] == p1[0]);
    REQUIRE(p4[1] == p1[1]);
}
