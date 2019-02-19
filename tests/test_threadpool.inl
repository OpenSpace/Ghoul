/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2019                                                               *
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

namespace {
    const int Epsilon = 50;

    const std::chrono::milliseconds SchedulingWaitTime(25);

    void threadSleep(std::chrono::microseconds waitTime) {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = start + waitTime;
        do {
            std::this_thread::yield();
        } while (std::chrono::high_resolution_clock::now() < end);
    }
    
    void pushWait(ghoul::ThreadPool& pool, int ms) {
        pool.queue([ms](){
            threadSleep(std::chrono::milliseconds(ms));
        });
    }
    void pushWait(ghoul::ThreadPool& pool, int ms, std::atomic_int& counter) {
        pool.queue([&counter, ms](){
            threadSleep(std::chrono::milliseconds(ms));
            ++counter;
        });
    }
} // namespace

class ThreadPoolTest : public testing::Test {};

TEST_F(ThreadPoolTest, Invariants) {
    ghoul::ThreadPool pool(1);
    
    // We have to wait for a short moment for the thread in the pool to be created
    // and scheduled so that it registers itself as waiting
    threadSleep(SchedulingWaitTime);
    
    ASSERT_EQ(1, pool.idleThreads());
    ASSERT_EQ(1, pool.size());
    ASSERT_EQ(0, pool.remainingTasks());
    ASSERT_TRUE(pool.isRunning());
}

TEST_F(ThreadPoolTest, CustomInitializer) {
    {
        std::atomic_int counter(0);

        ghoul::ThreadPool pool(5, [&counter]() { ++counter; });
        threadSleep(SchedulingWaitTime);
        EXPECT_EQ(5, counter);
    }

    {
        std::atomic_int counter(0);

        ghoul::ThreadPool pool(2, [&counter]() { ++counter; });
        threadSleep(SchedulingWaitTime);
        EXPECT_EQ(2, counter);
    }
}

TEST_F(ThreadPoolTest, CustomDeinitializer) {
    std::atomic_int counter(0);
    {
        ghoul::ThreadPool pool(5, [](){}, [&counter]() { ++counter; });
    }
    EXPECT_EQ(5, counter);

    counter = 0;
    {
        ghoul::ThreadPool pool(2, [](){}, [&counter]() { ++counter; });
    }
    EXPECT_EQ(2, counter);
}

TEST_F(ThreadPoolTest, CustomInitDeinit) {
    std::atomic_int counter(0);
    {
        ghoul::ThreadPool pool(5, [&counter](){++counter; }, [&counter]() { --counter; });
    }
    EXPECT_EQ(0, counter);

    counter = 0;
    {
        ghoul::ThreadPool pool(2, [&counter]() { ++counter; }, [&counter]() { --counter; });
    }
    EXPECT_EQ(0, counter);
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

TEST_F(ThreadPoolTest, CorrectSizes) {
    ghoul::ThreadPool pool(5);
    EXPECT_EQ(5, pool.size());

    pool.stop();
    EXPECT_EQ(5, pool.size());

    pool.start();
    EXPECT_EQ(5, pool.size());
}

TEST_F(ThreadPoolTest, IdleThreads) {
    ghoul::ThreadPool pool(2);

    // We have to wait for a short moment for the thread in the pool to be created
    // and scheduled so that it registers itself as waiting
    threadSleep(SchedulingWaitTime);
    EXPECT_EQ(2, pool.idleThreads());
    
    pushWait(pool, 100);
    threadSleep(SchedulingWaitTime);
    EXPECT_EQ(1, pool.idleThreads());

    pushWait(pool, 250);
    threadSleep(SchedulingWaitTime);
    EXPECT_EQ(0, pool.idleThreads());
    
    threadSleep(std::chrono::milliseconds(110));
    EXPECT_EQ(1, pool.idleThreads());
    
    threadSleep(std::chrono::milliseconds(260));
    EXPECT_EQ(2, pool.idleThreads());
}

TEST_F(ThreadPoolTest, RemainingTasks) {
    ghoul::ThreadPool pool(1);
    ASSERT_TRUE(pool.isRunning());
    
    ASSERT_EQ(0, pool.remainingTasks());
    pushWait(pool, 100);
    pushWait(pool, 100);
    pushWait(pool, 100);
    
    // Wait for the scheduler to pick up one of the threads
    threadSleep(SchedulingWaitTime);
    ASSERT_EQ(2, pool.remainingTasks());
    
    threadSleep(std::chrono::milliseconds(110));
    ASSERT_EQ(1, pool.remainingTasks());
    
    threadSleep(std::chrono::milliseconds(110));
    ASSERT_EQ(0, pool.remainingTasks());
}

TEST_F(ThreadPoolTest, ClearQueue) {
    ghoul::ThreadPool pool(1);
    ASSERT_TRUE(pool.isRunning());
    
    ASSERT_EQ(0, pool.remainingTasks());
    pushWait(pool, 100);
    pushWait(pool, 100);
    pushWait(pool, 100);

    // Wait for the scheduler to pick up one of the threads
    threadSleep(SchedulingWaitTime);
    ASSERT_EQ(2, pool.remainingTasks());
    
    pool.clearRemainingTasks();
    ASSERT_EQ(0, pool.remainingTasks());
    
    auto start = std::chrono::high_resolution_clock::now();

    pool.stop(
        ghoul::ThreadPool::RunRemainingTasks::Yes,
        ghoul::ThreadPool::DetachThreads::No
    );

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    EXPECT_GE(100 + Epsilon, ms);
    EXPECT_LE(100 - Epsilon, ms);
}

TEST_F(ThreadPoolTest, StartStopWithRemaining) {
    ghoul::ThreadPool pool(1);
    
    ASSERT_TRUE(pool.isRunning());

    std::atomic_int counter(0);
    pushWait(pool, 100, counter);
    threadSleep(SchedulingWaitTime);
    pool.stop(ghoul::ThreadPool::RunRemainingTasks::Yes);
    ASSERT_FALSE(pool.isRunning());
    EXPECT_EQ(1, counter);
    
    pool.start();
    ASSERT_TRUE(pool.isRunning());
    pushWait(pool, 100, counter);
    pool.stop();
    EXPECT_EQ(2, counter);
}

TEST_F(ThreadPoolTest, Basic) {
    ghoul::ThreadPool pool(5);

    std::atomic_int counter(0);

    for (int i = 0; i < 10; ++i) {
        pushWait(pool, 20 + 10 * i, counter);
    }

    threadSleep(std::chrono::milliseconds(500));
    EXPECT_EQ(10, counter);
}

TEST_F(ThreadPoolTest, ReturnValue) {
    // Checking whether the return value is set correctly
    
    ghoul::ThreadPool pool(1);
    
    std::future<int> f = pool.queue([]() {
        return 1337;
    });
    ASSERT_TRUE(f.valid());

    f.wait();

    EXPECT_TRUE(f.valid());
    EXPECT_EQ(1337, f.get());

    auto g = pool.queue([]() {
        return std::string("foobar");
    });

    ASSERT_TRUE(g.valid());
    g.wait();

    EXPECT_TRUE(g.valid());
    EXPECT_EQ("foobar", g.get());
}

TEST_F(ThreadPoolTest, VarArgs) {
    ghoul::ThreadPool pool(1);

    auto func = [](int i, float f, std::string s) {
        return std::make_tuple(s, f, i);
    };

    std::future<std::tuple<std::string, float, int>> ret = pool.queue(func, 1, 2.f, "3");
    ASSERT_TRUE(ret.valid());
    ret.wait();
    ASSERT_TRUE(ret.valid());
    std::tuple<std::string, float, int> val = ret.get();
    EXPECT_EQ("3", std::get<0>(val));
    EXPECT_EQ(2.f, std::get<1>(val));
    EXPECT_EQ(1, std::get<2>(val));
}

TEST_F(ThreadPoolTest, TaskOrdering) {
    // Tests whether the pushed tasks are performed in the correct FIFO order

    ghoul::ThreadPool pool(1);

    std::vector<int> res;
    auto func = [&res](int i) {
        res.push_back(i);
    };

    pool.queue(func, 4);
    pool.queue(func, 3);
    pool.queue(func, 2);
    pool.queue(func, 1);
    pool.queue(func, 0);

    auto start = std::chrono::high_resolution_clock::now();

    pool.stop();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    EXPECT_GE(Epsilon, ms);

    ASSERT_EQ(5, res.size());
    EXPECT_EQ(4, res[0]);
    EXPECT_EQ(3, res[1]);
    EXPECT_EQ(2, res[2]);
    EXPECT_EQ(1, res[3]);
    EXPECT_EQ(0, res[4]);
}

TEST_F(ThreadPoolTest, Parallelism) {
    // Queueing 5 tasks that take 100 milliseconds each on a thread pool with five
    // workers should take about 100 milliseconds
    
    ghoul::ThreadPool pool(5);
    
    std::atomic_int counter(0);
    auto start = std::chrono::high_resolution_clock::now();
    
    pushWait(pool, 100, counter);
    pushWait(pool, 100, counter);
    pushWait(pool, 100, counter);
    pushWait(pool, 100, counter);
    pushWait(pool, 100, counter);

    pool.stop(ghoul::ThreadPool::RunRemainingTasks::Yes);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_EQ(5, counter);
    EXPECT_GE(100 + Epsilon, ms);
    EXPECT_LE(100 - Epsilon, ms);
}

TEST_F(ThreadPoolTest, MissingParallelismWithWait) {
    // Queueing 2 tasks that take 100 milliseconds each on a thread pool with only one
    // workers should take about 200 milliseconds
    
    ghoul::ThreadPool pool(1);
    
    std::atomic_int counter(0);
    auto start = std::chrono::high_resolution_clock::now();
    
    pushWait(pool, 100, counter);
    pushWait(pool, 100, counter);
    
    pool.stop(ghoul::ThreadPool::RunRemainingTasks::Yes);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_EQ(2, counter);
    EXPECT_GE(200 + Epsilon, ms);
    EXPECT_LE(200 - Epsilon, ms);
}

TEST_F(ThreadPoolTest, MissingParallelismWithoutWait) {
    // Queueing 2 tasks that take 100 milliseconds each on a thread pool with only one
    // workers and aborting immediately after should only take 100 milliseconds
    
    ghoul::ThreadPool pool(1);
    
    std::atomic_int counter(0);
    auto start = std::chrono::high_resolution_clock::now();
    
    pushWait(pool, 100, counter);
    pushWait(pool, 100, counter);

    // We have to wait for a short moment to give one of the threads the chance to
    // wake up and grab one of the tasks. Otherwise, the ThreadPool might be stopped
    // before the first thread had the chance be scheduled
    threadSleep(std::chrono::milliseconds(25));
    
    pool.stop(ghoul::ThreadPool::RunRemainingTasks::No);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    EXPECT_EQ(1, counter);
    EXPECT_GE(100 + Epsilon, ms);
    EXPECT_LE(100 - Epsilon, ms);
}

TEST_F(ThreadPoolTest, BlockingStop) {
    ghoul::ThreadPool pool(1);
    
    std::atomic_int counter(0);
    pushWait(pool, 100, counter);
    pushWait(pool, 100, counter);

    auto start = std::chrono::high_resolution_clock::now();
    
    pool.stop(
        ghoul::ThreadPool::RunRemainingTasks::Yes,
        ghoul::ThreadPool::DetachThreads::No
    );
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    EXPECT_EQ(2, counter);
    EXPECT_GE(200 + Epsilon, ms);
    EXPECT_LE(200 - Epsilon, ms);
}

TEST_F(ThreadPoolTest, DetachingStop) {
    ghoul::ThreadPool pool(2);
    
    std::atomic_int counter(0);
    pushWait(pool, 100, counter);
    pushWait(pool, 100, counter);
    threadSleep(SchedulingWaitTime);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    pool.stop(
        ghoul::ThreadPool::RunRemainingTasks::No,
        ghoul::ThreadPool::DetachThreads::Yes
    );
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // We have to wait for the detached thread to finish
    
    threadSleep(std::chrono::milliseconds(250));
    
    EXPECT_EQ(2, counter);
    // As it is not blocking, the operation shouldn't take any time at all
    EXPECT_GE(Epsilon, ms);
}
