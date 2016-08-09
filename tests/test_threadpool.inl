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
    
    // We have to wait for a short moment for the thread in the pool to be created
    // and scheduled so that it registers itself as waiting
    std::this_thread::sleep_for(std::chrono::microseconds(250));
    
    ASSERT_EQ(1, pool.nIdleThreads());
    ASSERT_EQ(1, pool.size());
    ASSERT_TRUE(pool.isRunning());
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

TEST_F(ThreadPoolTest, IdleThreads) {
    ghoul::ThreadPool pool(2);

    // We have to wait for a short moment for the thread in the pool to be created
    // and scheduled so that it registers itself as waiting
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    EXPECT_EQ(2, pool.nIdleThreads());
    
    pool.push([](){ std::this_thread::sleep_for(std::chrono::milliseconds(200)); });
    std::this_thread::sleep_for(std::chrono::microseconds(250));
    EXPECT_EQ(1, pool.nIdleThreads());
    
    pool.push([](){ std::this_thread::sleep_for(std::chrono::milliseconds(500)); });
    std::this_thread::sleep_for(std::chrono::microseconds(250));
    EXPECT_EQ(0, pool.nIdleThreads());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    EXPECT_EQ(1, pool.nIdleThreads());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(750));
    EXPECT_EQ(2, pool.nIdleThreads());
}

TEST_F(ThreadPoolTest, StartStopWithRemaining) {
    ghoul::ThreadPool pool(1);
    
    ASSERT_TRUE(pool.isRunning());
    
    pool.push([](){ std::this_thread::sleep_for(std::chrono::milliseconds(250)); });
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    pool.stop(ghoul::ThreadPool::RunRemainingTasks::Yes);
    std::this_thread::sleep_for(std::chrono::microseconds(250));
    
    ASSERT_FALSE(pool.isRunning());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_FALSE(pool.isRunning());
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
    ASSERT_TRUE(f.valid());
    
    f.wait();
    
    EXPECT_TRUE(f.valid());
    EXPECT_EQ(1337, f.get());
}

TEST_F(ThreadPoolTest, Parallelism) {
    // Queueing 5 tasks that take 500 milliseconds each on a thread pool with five
    // workers should take about 500 milliseconds
    
    const int Epsilon = 10;
    
    ghoul::ThreadPool pool(5);
    
    std::atomic_int counter(0);
    auto start = std::chrono::high_resolution_clock::now();
    
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    
    pool.stop(ghoul::ThreadPool::RunRemainingTasks::Yes);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_EQ(5, counter);
    EXPECT_GE(500 + Epsilon, ms) << "Queuing operation took longer than 500ms";
    EXPECT_LE(500 - Epsilon, ms) << "Queuing operation took less than 500ms";
}

TEST_F(ThreadPoolTest, MissingParallelismWithWait) {
    // Queueing 2 tasks that take 500 milliseconds each on a thread pool with only one
    // workers should take about 1 second
    
    const int Epsilon = 10;

    ghoul::ThreadPool pool(1);
    
    std::atomic_int counter(0);
    auto start = std::chrono::high_resolution_clock::now();
    
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    
    pool.stop(ghoul::ThreadPool::RunRemainingTasks::Yes);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    ASSERT_EQ(2, counter);
    EXPECT_GE(1000 + Epsilon, ms) << "Queuing operation took longer than 1s";
    EXPECT_LE(1000 - Epsilon, ms) << "Queuing operation took less than 1s";
}

TEST_F(ThreadPoolTest, MissingParallelismWithoutWait) {
    // Queueing 2 tasks that take 500 milliseconds each on a thread pool with only one
    // workers and aborting immediately after should only take 500 milliseconds
    
    const int Epsilon = 10;
    
    ghoul::ThreadPool pool(1);
    
    std::atomic_int counter(0);
    auto start = std::chrono::high_resolution_clock::now();
    
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    // We have to wait for a short moment to give one of the threads the chance to
    // wake up and grab one of the tasks. Otherwise, the ThreadPool might be stopped
    // before the first thread had the chance be scheduled
    std::this_thread::sleep_for(std::chrono::microseconds(250));
    
    pool.stop(ghoul::ThreadPool::RunRemainingTasks::No);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    ASSERT_EQ(1, counter);
    EXPECT_GE(500 + Epsilon, ms) << "Queuing operation took longer than 500ms";
    EXPECT_LE(500 - Epsilon, ms) << "Queuing operation took less than 500ms";
}

TEST_F(ThreadPoolTest, BlockingStop) {
    const int Epsilon = 10;

    ghoul::ThreadPool pool(1);
    
    std::atomic_int counter(0);
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });

    auto start = std::chrono::high_resolution_clock::now();
    
    pool.stop(
        ghoul::ThreadPool::RunRemainingTasks::Yes,
        ghoul::ThreadPool::DetachThreads::No
    );
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    ASSERT_EQ(2, counter);
    EXPECT_GE(1000 + Epsilon, ms);
    EXPECT_LE(1000 - Epsilon, ms);
}

TEST_F(ThreadPoolTest, DetachingStop) {
    const int Epsilon = 10;
    
    ghoul::ThreadPool pool(2);
    
    std::atomic_int counter(0);
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    pool.push([&counter](){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ++counter;
    });
    std::this_thread::sleep_for(std::chrono::microseconds(250));
    
    auto start = std::chrono::high_resolution_clock::now();
    
    pool.stop(
        ghoul::ThreadPool::RunRemainingTasks::No,
        ghoul::ThreadPool::DetachThreads::Yes
    );
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // We have to wait for the detached thread to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(750));
    
    ASSERT_EQ(2, counter);
    // As it is not blocking, the operation shouldn't take any time at all
    EXPECT_GE(Epsilon, ms);
}
