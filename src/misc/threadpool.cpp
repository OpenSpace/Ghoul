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
 ****************************************************************************************/

#include <ghoul/misc/threadpool.h>

#include <ghoul/misc/assert.h>

namespace ghoul {

using Func = std::function<void()>;
    
ThreadPool::ThreadPool(int nThreads, Func workerInit, Func workerDeinit)
    : _nWaiting(0)
    , _isRunning(true)
    , _workerInitialization(std::move(workerInit))
    , _workerDeinitialization(std::move(workerDeinit))
{
    ghoul_assert(nThreads > 0, "nThreads must be bigger than 0");
    ghoul_assert(workerInit, "workerInit must be a valid function");
    ghoul_assert(workerDeinit, "workerDeinit must be a valid function");
    
    resize(nThreads);
}
    
ThreadPool::~ThreadPool() {
    if (isRunning()) {
        stop();
    }
}

void ThreadPool::start() {
    ghoul_assert(!isRunning(), "ThreadPool must not be running");
    
    _isRunning = true;
    for (Worker& w : _workers) {
        activateWorker(w);
    }
}
    
void ThreadPool::stop(RunRemainingTasks runTasks, DetachThreads detachThreads) {
    ghoul_assert(isRunning(), "ThreadPool must be running");
    ghoul_assert(
        !(runTasks == RunRemainingTasks::Yes && detachThreads == DetachThreads::Yes),
        "Cannot run remaining tasks and detach threads"
    );

    _isRunning = false;

    if (runTasks == RunRemainingTasks::No) {
        clearQueue();
    }
    
    _cv.notify_all();
    for (Worker& w : _workers) {
        if (detachThreads == DetachThreads::Yes) {
            w.thread->detach();
        }
        else {
            w.thread->join();
        }
    }
    
    _workers.clear();
}

bool ThreadPool::isRunning() const {
    return _isRunning;
}

int ThreadPool::size() const {
    return static_cast<int>(_workers.size());
}

int ThreadPool::nIdleThreads() const {
    return _nWaiting;
}

void ThreadPool::resize(int nThreads) {
    ghoul_assert(nThreads > 0, "nThreads must be bigger than 0");
    
//    if (_isRunning) {
        int oldNThreads = size();
        if (oldNThreads <= nThreads) {
            // if the number of threads is increased
            _workers.resize(nThreads);
            
            for (int i = oldNThreads; i < nThreads; ++i) {
                activateWorker(_workers[i]);
            }
        }
        else {
            // the number of threads is decreased
            for (int i = oldNThreads - 1; i >= nThreads; --i) {
                // this thread will finish
                *(_workers[i].shouldStop) = true;
                _workers[i].thread->detach();
            }
//            {
                // stop the detached threads that were waiting
//                std::unique_lock<std::mutex> lock(_mutex);
                _cv.notify_all();
//            }
            // safe to delete because the threads are detached
            _workers.resize(nThreads);
        }
//    }
    
}
    
void ThreadPool::clearQueue() {
    while (!_taskQueue.isEmpty()) {
        _taskQueue.pop();
    }
}


void ThreadPool::activateWorker(Worker& worker) {
    // a copy of the shared ptr to the flag
    auto shouldTerminate = std::make_shared<std::atomic_bool>(false);
    // capturing the flag by value to maintain a copy of the shared_ptr
    auto workerLoop = [this, shouldTerminate]() {
        _workerInitialization();
        
        std::function<void()> f;
        bool hasItem;
        std::tie(f, hasItem) = _taskQueue.pop();
        
        while (true) {
            while (hasItem) {
                // if there is anything in the queue
                f();
                
                if (*shouldTerminate) {
                    return;
                }
                else {
                    std::tie(f, hasItem) = _taskQueue.pop();
                }
            }
            
            // the queue is empty here, wait for the next command
            std::unique_lock<std::mutex> lock(_mutex);
            ++_nWaiting;
            _cv.wait(
                 lock,
                 [this, &f, &hasItem, shouldTerminate]() {
                     std::tie(f, hasItem) = _taskQueue.pop();
                     return hasItem || *shouldTerminate || !_isRunning;
                 }
             );
            --_nWaiting;
            if (!hasItem) {
                // if the queue is empty and this->isDone == true or *flag then return
                return;
            }
        }
        
        _workerDeinitialization();
    };
    
    worker = {
        std::make_unique<std::thread>(workerLoop),
        std::move(shouldTerminate)
    };
}

std::tuple<ThreadPool::Task, bool> ThreadPool::TaskQueue::pop() {
    std::lock_guard<std::mutex> lock(_queueMutex);
    if (_queue.empty()) {
        return { Task(), false };
    }
    else {
        Task t = std::move(_queue.front());
        _queue.pop();
        return { std::move(t), true };
    }
}
    
void ThreadPool::TaskQueue::push(ThreadPool::Task&& task) {
    std::lock_guard<std::mutex> lock(_queueMutex);
    _queue.push(task);
}
    
bool ThreadPool::TaskQueue::isEmpty() const {
    std::lock_guard<std::mutex> lock(_queueMutex);
    return _queue.empty();
}
    
size_t ThreadPool::TaskQueue::size() const {
    std::lock_guard<std::mutex> lock(_queueMutex);
    return _queue.size();
}

} // namespace openspace
