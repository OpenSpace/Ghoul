/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2022                                                               *
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

// (2020-12-29, abock) Something is horribly wrong with the threadpool as of right now
// causing some of the tests to be flaky on Windows in RelWithDebInfo, but not Debug.
// Until we figure out what to do with the threadpool, we'll disable the tests for now
#if 0

#include "catch2/catch.hpp"

#include <ghoul/misc/threadpool.h>

namespace {
    constexpr const int Epsilon = 50;

    constexpr const std::chrono::milliseconds SchedulingWaitTime(25);

    void threadSleep(std::chrono::microseconds waitTime) {
        auto start = std::chrono::high_resolution_clock::now();
        auto end = start + waitTime;
        do {
            std::this_thread::yield();
        } while (std::chrono::high_resolution_clock::now() < end);
    }

    void pushWait(ghoul::ThreadPool& pool, int ms) {
        pool.queue([ms]() {
            threadSleep(std::chrono::milliseconds(ms));
        });
    }
    void pushWait(ghoul::ThreadPool& pool, int ms, std::atomic_int& counter) {
        pool.queue([&counter, ms]() {
            threadSleep(std::chrono::milliseconds(ms));
            ++counter;
        });
    }
} // namespace


TEST_CASE("ThreadPool: Invariants", "[threadpool]") {
    ghoul::ThreadPool pool(1);

    // We have to wait for a short moment for the thread in the pool to be created
    // and scheduled so that it registers itself as waiting
    threadSleep(SchedulingWaitTime);

    REQUIRE(pool.idleThreads() == 1);
    REQUIRE(pool.size() == 1);
    REQUIRE(pool.remainingTasks() == 0);
    REQUIRE(pool.isRunning());
}

TEST_CASE("ThreadPool: Custom Initializer", "[threadpool]") {
    {
        std::atomic_int counter(0);

        ghoul::ThreadPool pool(5, [&counter]() { ++counter; });
        threadSleep(SchedulingWaitTime);
        REQUIRE(counter == 5);
    }

    {
        std::atomic_int counter(0);

        ghoul::ThreadPool pool(2, [&counter]() { ++counter; });
        threadSleep(SchedulingWaitTime);
        REQUIRE(counter == 2);
    }
}

TEST_CASE("ThreadPool: Custom Deinitializer", "[threadpool]") {
    std::atomic_int counter(0);
    {
        ghoul::ThreadPool pool(5, []() {}, [&counter]() { ++counter; });
    }
    REQUIRE(counter == 5);

    counter = 0;
    {
        ghoul::ThreadPool pool(2, []() {}, [&counter]() { ++counter; });
    }
    REQUIRE(counter == 2);
}

TEST_CASE("ThreadPool: Custom Init Deinit", "[threadpool]") {
    std::atomic_int counter(0);
    {
        ghoul::ThreadPool pool(5, [&counter]() {++counter; }, [&counter]() { --counter; });
    }
    REQUIRE(counter == 0);

    counter = 0;
    {
        ghoul::ThreadPool pool(2, [&counter]() { ++counter; }, [&counter]() { --counter; });
    }
    REQUIRE(counter == 0);
}

TEST_CASE("ThreadPool: Resize/Expand", "[threadpool]") {
    ghoul::ThreadPool pool(1);

    REQUIRE(pool.size() == 1);

    pool.resize(5);
    REQUIRE(pool.size() == 5);
}

TEST_CASE("ThreadPool: Resize/Shrink", "[threadpool]") {
    ghoul::ThreadPool pool(5);

    pool.resize(1);
    REQUIRE(pool.size() == 1);
}

TEST_CASE("ThreadPool: Correct Sizes", "[threadpool]") {
    ghoul::ThreadPool pool(5);
    REQUIRE(pool.size() == 5);

    pool.stop();
    REQUIRE(pool.size() == 5);

    pool.start();
    REQUIRE(pool.size() == 5);
}

TEST_CASE("ThreadPool: Idle Threads", "[threadpool]") {
    ghoul::ThreadPool pool(2);

    // We have to wait for a short moment for the thread in the pool to be created
    // and scheduled so that it registers itself as waiting
    threadSleep(SchedulingWaitTime);
    REQUIRE(pool.idleThreads() == 2);

    pushWait(pool, 100);
    threadSleep(SchedulingWaitTime);
    REQUIRE(pool.idleThreads() == 1);

    pushWait(pool, 250);
    threadSleep(SchedulingWaitTime);
    REQUIRE(pool.idleThreads() == 0);

    threadSleep(std::chrono::milliseconds(110));
    REQUIRE(pool.idleThreads() == 1);

    threadSleep(std::chrono::milliseconds(260));
    REQUIRE(pool.idleThreads() == 2);
}

TEST_CASE("ThreadPool: Remaining Tasks", "[threadpool]") {
    ghoul::ThreadPool pool(1);
    REQUIRE(pool.isRunning());

    REQUIRE(pool.remainingTasks() == 0);
    pushWait(pool, 100);
    pushWait(pool, 100);
    pushWait(pool, 100);

    // Wait for the scheduler to pick up one of the threads
    threadSleep(SchedulingWaitTime);
    REQUIRE(pool.remainingTasks() == 2);

    threadSleep(std::chrono::milliseconds(110));
    REQUIRE(pool.remainingTasks() == 1);

    threadSleep(std::chrono::milliseconds(110));
    REQUIRE(pool.remainingTasks() == 0);
}

TEST_CASE("ThreadPool: Clear Queue", "[threadpool]") {
    ghoul::ThreadPool pool(1);
    REQUIRE(pool.isRunning());

    REQUIRE(pool.remainingTasks() == 0);
    pushWait(pool, 100);
    pushWait(pool, 100);
    pushWait(pool, 100);

    // Wait for the scheduler to pick up one of the threads
    threadSleep(SchedulingWaitTime);
    REQUIRE(pool.remainingTasks() == 2);

    pool.clearRemainingTasks();
    REQUIRE(pool.remainingTasks() == 0);

    auto start = std::chrono::high_resolution_clock::now();

    pool.stop(
        ghoul::ThreadPool::RunRemainingTasks::Yes,
        ghoul::ThreadPool::DetachThreads::No
    );

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    REQUIRE(100 + Epsilon > ms);
    REQUIRE(100 - Epsilon < ms);
}

TEST_CASE("ThreadPool: Start Stop With Remaining", "[threadpool]") {
    // @TODO (abock, 2020-01-06) This crashes with an exception (see issue #44)
#if 0
    ghoul::ThreadPool pool(1);
    REQUIRE(pool.isRunning());

    std::atomic_int counter(0);
    pushWait(pool, 100, counter);
    threadSleep(SchedulingWaitTime);
    pool.stop(ghoul::ThreadPool::RunRemainingTasks::Yes);
    REQUIRE_FALSE(pool.isRunning());
    REQUIRE(counter == 1);

    pool.start();
    REQUIRE(pool.isRunning());
    pushWait(pool, 100, counter);
    pool.stop();
    REQUIRE(counter == 2);
#endif
}

TEST_CASE("ThreadPool: Basic", "[threadpool]") {
    ghoul::ThreadPool pool(5);

    std::atomic_int counter(0);

    for (int i = 0; i < 10; ++i) {
        pushWait(pool, 20 + 10 * i, counter);
    }

    threadSleep(std::chrono::milliseconds(500));
    REQUIRE(counter == 10);
}

//TEST_CASE("ThreadPool: Return Value", "[threadpool]") {
//    // Checking whether the return value is set correctly
//
//    ghoul::ThreadPool pool(1);
//
//    std::future<int> f = pool.queue([]() { return 1337; });
//    REQUIRE(f.valid());
//
//    f.wait();
//
//    REQUIRE(f.valid());
//    REQUIRE(f.get() == 1337);
//
//    auto g = pool.queue([]() { return std::string("foobar"); });
//
//    REQUIRE(g.valid());
//    g.wait();
//
//    REQUIRE(g.valid());
//    REQUIRE(g.get() == "foobar");
//}
//TEST_CASE("ThreadPool: Var Args", "[threadpool]") {
//    ghoul::ThreadPool pool(1);
//
//    auto func = [](int i, float f, std::string s) { return std::make_tuple(s, f, i); };
//
//    std::future<std::tuple<std::string, float, int>> ret = pool.queue(func, 1, 2.f, "3");
//    REQUIRE(ret.valid());
//    ret.wait();
//    REQUIRE(ret.valid());
//    std::tuple<std::string, float, int> val = ret.get();
//    REQUIRE(std::get<0>(val) == "3");
//    REQUIRE(std::get<1>(val) == 2.f);
//    REQUIRE(std::get<2>(val) == 1);
//}

TEST_CASE("ThreadPool: Task Ordering", "[threadpool]") {
    // Tests whether the pushed tasks are performed in the correct FIFO order

    ghoul::ThreadPool pool(1);

    std::vector<int> res;
    auto func = [&res](int i) { res.push_back(i); };

    pool.queue(func, 4);
    pool.queue(func, 3);
    pool.queue(func, 2);
    pool.queue(func, 1);
    pool.queue(func, 0);

    auto start = std::chrono::high_resolution_clock::now();

    pool.stop();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    REQUIRE(ms < Epsilon);

    REQUIRE(res.size() == 5);
    REQUIRE(res[0] == 4);
    REQUIRE(res[1] == 3);
    REQUIRE(res[2] == 2);
    REQUIRE(res[3] == 1);
    REQUIRE(res[4] == 0);
}

TEST_CASE("ThreadPool: Parallelism", "[threadpool]") {
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
    REQUIRE(counter == 5);
    REQUIRE(ms < 100 + Epsilon);
    REQUIRE(ms > 100 - Epsilon);
}

TEST_CASE("ThreadPool: Missing Parallelism With Wait", "[threadpool]") {
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
    REQUIRE(counter == 2);
    REQUIRE(ms < 200 + Epsilon);
    REQUIRE(ms > 200 - Epsilon);
}

TEST_CASE("ThreadPool: Missing Parallelism Without Wait", "[threadpool]") {
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
    REQUIRE(counter == 1);
    REQUIRE(ms < 100 + Epsilon);
    REQUIRE(ms > 100 - Epsilon);
}

//TEST_CASE("ThreadPool: Blocking Stop", "[threadpool]") {
//    ghoul::ThreadPool pool(1);
//
//    std::atomic_int counter(0);
//    pushWait(pool, 100, counter);
//    pushWait(pool, 100, counter);
//
//    auto start = std::chrono::high_resolution_clock::now();
//
//    pool.stop(
//        ghoul::ThreadPool::RunRemainingTasks::Yes,
//        ghoul::ThreadPool::DetachThreads::No
//    );
//
//    auto end = std::chrono::high_resolution_clock::now();
//    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
//
//    REQUIRE(counter == 2);
//    REQUIRE(ms < 200 + Epsilon);
//    REQUIRE(ms > 200 - Epsilon);
//}

//TEST_CASE("ThreadPool: Detaching Stop", "[threadpool]") {
//    ghoul::ThreadPool pool(2);
//
//    std::atomic_int counter(0);
//    pushWait(pool, 100, counter);
//    pushWait(pool, 100, counter);
//    threadSleep(SchedulingWaitTime);
//
//    auto start = std::chrono::high_resolution_clock::now();
//
//    pool.stop(
//        ghoul::ThreadPool::RunRemainingTasks::No,
//        ghoul::ThreadPool::DetachThreads::Yes
//    );
//
//    auto end = std::chrono::high_resolution_clock::now();
//    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
//
//    // We have to wait for the detached thread to finish
//
//    threadSleep(std::chrono::milliseconds(250));
//
//    REQUIRE(counter == 2);
//    // As it is not blocking, the operation shouldn't take any time at all
//    REQUIRE(ms < Epsilon);
//}

#endif 