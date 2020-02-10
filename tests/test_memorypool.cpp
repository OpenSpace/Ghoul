/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2020                                                               *
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

// @TODO(abock, 2020-01-06) The MemoryPool causes a heap corruption (see issue #43) which
// needs to be fixed first

#if 0

TEST_CASE("MemoryPool: MemoryPool", "[memorypool]") {
    ghoul::MemoryPool<> pool1;
    void* p1 = pool1.alloc(1024);
    REQUIRE(p1 != nullptr);

    void* p2 = pool1.alloc(1024);
    REQUIRE(p2 != nullptr);

    void* p3 = pool1.alloc(1024);
    REQUIRE(p3 != nullptr);

    void* p4 = pool1.alloc(1024);
    REQUIRE(p4 != nullptr);

    ghoul::MemoryPool<2048> pool2;
    void* q1 = pool2.alloc(1024);
    REQUIRE(q1 != nullptr);
    void* q2 = pool2.alloc(1024);
    REQUIRE(q2 != nullptr);
    void* q3 = pool2.alloc(1024);
    REQUIRE(q3 != nullptr);
    void* q4 = pool2.alloc(1024);
    REQUIRE(q4 != nullptr);
}

TEST_CASE("MemoryPool: Typed MemoryPool", "[memorypool]") {
    ghoul::TypedMemoryPool<int> pool1;
    std::vector<void*> p1 = pool1.allocate(2);
    REQUIRE(p1.size() == 2);
    REQUIRE(p1[0] != p1[1]);

    std::vector<void*> p2 = pool1.allocate(2);
    REQUIRE(p2.size() == 2);
    REQUIRE(p2[0] != p2[1]);

    std::vector<void*> p3 = pool1.allocate(2);
    REQUIRE(p3.size() == 2);
    REQUIRE(p3[0] != p3[1]);
    std::vector<void*> p4 = pool1.allocate(2);
    REQUIRE(p4.size() == 2);
    REQUIRE(p4[0] != p4[1]);

    ghoul::TypedMemoryPool<int, 8> pool2;
    std::vector<void*> q1 = pool2.allocate(2);
    REQUIRE(q1.size() == 2);
    REQUIRE(q1[0] != q1[1]);

    std::vector<void*> q2 = pool2.allocate(2);
    REQUIRE(q2.size() == 2);
    REQUIRE(q2[0] != q2[1]);

    std::vector<void*> q3 = pool2.allocate(2);
    REQUIRE(q3.size() == 2);
    REQUIRE(q3[0] != q3[1]);

    std::vector<void*> q4 = pool2.allocate(2);
    REQUIRE(q4.size() == 2);
    REQUIRE(q4[0] != q4[1]);
}

TEST_CASE("MemoryPool: Reusable Typed MemoryPool", "[memorypool]") {
    ghoul::ReusableTypedMemoryPool<int> pool1;
    std::vector<void*> p1 = pool1.allocate(2);
    REQUIRE(p1.size() == 2);
    REQUIRE(p1[0] != p1[1]);

    std::vector<void*> p2 = pool1.allocate(2);
    REQUIRE(p2.size() == 2);
    REQUIRE(p2[0] != p2[1]);

    std::vector<void*> p3 = pool1.allocate(2);
    REQUIRE(p3.size() == 2);
    REQUIRE(p3[0] != p3[1]);

    std::vector<void*> p4 = pool1.allocate(2);
    REQUIRE(p4.size() == 2);
    REQUIRE(p4[0] != p4[1]);

    pool1.free(reinterpret_cast<int*>(p1[0]));
}

TEST_CASE("MemoryPool: Reusable Typed MemoryPool Reuse", "[memorypool]") {
    ghoul::ReusableTypedMemoryPool<int> pool1;
    std::vector<void*> p1 = pool1.allocate(2);
    REQUIRE(p1.size() == 2);
    REQUIRE(p1[0] != p1[1]);

    std::vector<void*> p2 = pool1.allocate(2);
    REQUIRE(p2.size() == 2);
    REQUIRE(p2[0] != p2[1]);

    pool1.free(reinterpret_cast<int*>(p1[0]));
    pool1.free(reinterpret_cast<int*>(p1[1]));
    pool1.free(reinterpret_cast<int*>(p2[0]));
    pool1.free(reinterpret_cast<int*>(p2[1]));

    std::vector<void*> p3 = pool1.allocate(2);
    REQUIRE(p3.size() == 2);
    REQUIRE(p3[0] != p3[1]);

    std::vector<void*> p4 = pool1.allocate(2);
    REQUIRE(p4.size() == 2);
    REQUIRE(p4[0] != p4[1]);

    REQUIRE(p3[0] == p2[0]);
    REQUIRE(p3[1] == p2[1]);
    REQUIRE(p4[0] == p1[0]);
    REQUIRE(p4[1] == p1[1]);
}

#endif
