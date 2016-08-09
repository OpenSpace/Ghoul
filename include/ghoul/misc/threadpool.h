/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2016                                                               *
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
 *****************************************************************************************
 * Based on the CPTL implementation by Vitaliy Vitsentiy found here:                     *
 *
 *****************************************************************************************
 *                                                                                       *
 *  Copyright (C) 2014 by Vitaliy Vitsentiy                                              *
 *                                                                                       *
 *  Licensed under the Apache License, Version 2.0 (the "License");                      *
 *  you may not use this file except in compliance with the License.                     *
 *  You may obtain a copy of the License at                                              *
 *                                                                                       *
 *     http://www.apache.org/licenses/LICENSE-2.0                                        *
 *                                                                                       *
 *  Unless required by applicable law or agreed to in writing, software                  *
 *  distributed under the License is distributed on an "AS IS" BASIS,                    *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.             *
 *  See the License for the specific language governing permissions and                  *
 *  limitations under the License.                                                       *
 *                                                                                       *
 ****************************************************************************************/

#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace ghoul {
    
class ThreadPool {
public:
    enum class Waiting { Yes, No };
    
    ThreadPool(int nThreads = 1);
    
    ~ThreadPool();
    
    int size() const;
    
    int nIdle() const;
    
    void resize(int nThreads);
    
    void clearQueue();
    
    void stop(Waiting shouldWait = Waiting::Yes);

    template<typename F, typename... Rest>
    auto push(F&& f, Rest&&... rest) -> std::future<decltype(f(rest...))>; 
    
private:
    using Task = std::function<void()>;

    struct Worker {
        std::unique_ptr<std::thread> thread;
        std::shared_ptr<std::atomic<bool>> shouldStop;
    };
    
    class TaskQueue {
    public:
        std::tuple<Task, bool> pop();
        void push(Task task);
        bool isEmpty() const;
        size_t size() const;
    
    private:
        std::queue<ThreadPool::Task> _queue;

        mutable std::mutex _mutex;
    };
    
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    
    void setThread(int i);
    
    
    std::vector<Worker> _workers;

    TaskQueue _taskQueue;

    std::atomic_bool _isDone;
    std::atomic_bool _isStop;
    std::atomic_int _nWaiting;  // how many threads are waiting
    
    std::mutex _mutex;
    std::condition_variable _cv;
    
};

} // namespace ghoul

#include "threadpool.inl"

#endif // __THREADPOOL_H__
