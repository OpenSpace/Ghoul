/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
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

#include "gtest/gtest.h"

#include <ghoul/misc/threadpool.h>

class ThreadPoolTest : public testing::Test {};

TEST_F(ThreadPoolTest, Invariants) {
    ghoul::ThreadPool pool(1);
    
    ASSERT_EQ(1, pool.nIdle());
    ASSERT_EQ(1, pool.size());
}

TEST_F(ThreadPoolTest, ResizeExpand) {
    ghoul::ThreadPool pool(1);
    
    ASSERT_EQ(1, pool.size());
    
    pool.resize(5);
    ASSERT_EQ(5, pool.size());
}

TEST_F(ThreadPoolTest, ResizeShrink) {
    ghoul::ThreadPool pool(5);
    
    pool.resize(1);
    ASSERT_EQ(1, pool.size());
}

TEST_F(ThreadPoolTest, Basic) {
    ghoul::ThreadPool pool(5);

    std::atomic_int val(0);

    for (int i = 0; i < 10; ++i) {
        pool.push([&val, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100 + 10*i));
            val++;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EXPECT_EQ(10, val) << "10 tasks taking 100 to 190 ms on 5 threads should take less than 1000 ms";
}

TEST_F(ThreadPoolTest, ReturnValue) {
    // Checking whether the return value is set correctly
    
    ghoul::ThreadPool pool(1);
    
    std::future<int> f = pool.push([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 1337;
    });
    ASSERT_EQ(true, f.valid());
    
    f.wait();
    
    EXPECT_EQ(true, f.valid());
    EXPECT_EQ(1337, f.get());
}

TEST_F(ThreadPoolTest, Parallelism) {
    // Queueing 5 tasks that take a second each on a thread pool with five workers
    // should take about 1 second
    
    const int Epsilon = 10;
    
    ghoul::ThreadPool pool(5);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    pool.push([](){ std::this_thread::sleep_for(std::chrono::seconds(1)); });
    pool.push([](){ std::this_thread::sleep_for(std::chrono::seconds(1)); });
    pool.push([](){ std::this_thread::sleep_for(std::chrono::seconds(1)); });
    pool.push([](){ std::this_thread::sleep_for(std::chrono::seconds(1)); });
    pool.push([](){ std::this_thread::sleep_for(std::chrono::seconds(1)); });
    
    pool.stop(ghoul::ThreadPool::Waiting::Yes);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(1000 + Epsilon, ms) << "Queuing operation took longer than 1000 ms";
    EXPECT_LE(1000 - Epsilon, ms) << "Queuing operation took less than 1000 ms";
}

TEST_F(ThreadPoolTest, MissingParallelism) {
    // Queueing 2 tasks that take a second each on a thread pool with only one
    // workers should take about 2 seconds
    
    const int Epsilon = 10;

    ghoul::ThreadPool pool(1);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    pool.push([](){ std::this_thread::sleep_for(std::chrono::seconds(1)); });
    pool.push([](){ std::this_thread::sleep_for(std::chrono::seconds(1)); });
    
    pool.stop(ghoul::ThreadPool::Waiting::Yes);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_GE(2000 + Epsilon, ms) << "Queuing operation took longer than 5 ms";
    EXPECT_LE(2000 - Epsilon, ms) << "Queuing operation took longer than 5 ms";
}
