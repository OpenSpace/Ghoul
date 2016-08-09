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

namespace ghoul {

ThreadPool::ThreadPool(int nThreads)
    : _nWaiting(nThreads)
    , _isStop(false)
    , _isDone(false)
{
    resize(nThreads);
}
    
ThreadPool::~ThreadPool() {
    stop(Waiting::Yes);
}
    
int ThreadPool::size() const {
    return static_cast<int>(_workers.size());
}

int ThreadPool::nIdle() const {
    return _nWaiting;
}

void ThreadPool::resize(int nThreads) {
    if (!_isStop && !_isDone) {
        int oldNThreads = size();
        if (oldNThreads <= nThreads) {
            // if the number of threads is increased
            _workers.resize(nThreads);
            
            for (int i = oldNThreads; i < nThreads; ++i) {
                setThread(i);
            }
        }
        else {
            // the number of threads is decreased
            for (int i = oldNThreads - 1; i >= nThreads; --i) {
                // this thread will finish
                *(_workers[i].shouldStop) = true;
                _workers[i].thread->detach();
            }
            {
                // stop the detached threads that were waiting
                std::unique_lock<std::mutex> lock(_mutex);
                _cv.notify_all();
            }
            // safe to delete because the threads are detached
            _workers.resize(nThreads);
        }
    }
    
}
    
void ThreadPool::clearQueue() {
    std::packaged_task<void()> dummy;
    while (!_taskQueue.isEmpty()) {
        _taskQueue.pop();
    }
}

void ThreadPool::stop(Waiting shouldWait) {
    if (shouldWait == Waiting::No) {
        if (_isStop)
            return;
        _isStop = true;
        for (int i = 0, n = this->size(); i < n; ++i) {
            *(_workers[i].shouldStop) = true;
        }
        clearQueue();  // empty the queue
    }
    else {
        if (_isDone || _isStop)
            return;
        _isDone = true;  // give the waiting threads a command to finish
    }
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cv.notify_all();  // stop all waiting threads
    }
    for (int i = 0; i < size(); ++i) {  // wait for the computing threads to finish
        if (_workers[i].thread->joinable()) {
            _workers[i].thread->join();
        }
    }
    // if there were no threads in the pool but some functors in the queue, the functors are not deleted by the threads
    // therefore delete them here
    clearQueue();
    _workers.clear();
}


void ThreadPool::setThread(int i) {
    // a copy of the shared ptr to the flag
    auto shouldTerminate = std::make_shared<std::atomic_bool>(false);
    // capturing the flag by value to maintain a copy of the shared_ptr
    auto workerLoop = [this, shouldTerminate]() {
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
                         return hasItem || _isDone || *shouldTerminate;
                     }
                     );
            --_nWaiting;
            if (!hasItem) {
                // if the queue is empty and this->isDone == true or *flag then return
                return;
            }
        }
    };
    
    _workers[i] = {
        std::make_unique<std::thread>(workerLoop),
        std::move(shouldTerminate)
    };
}

std::tuple<ThreadPool::Task, bool> ThreadPool::TaskQueue::pop() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_queue.empty()) {
        return { Task(), false };
    }
    else {
        Task t = std::move(_queue.front());
        _queue.pop();
        return { std::move(t), true };
    }
}
    
void ThreadPool::TaskQueue::push(ThreadPool::Task task) {
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push(std::move(task));
}
    
bool ThreadPool::TaskQueue::isEmpty() const {
    std::unique_lock<std::mutex> lock(_mutex);
    return _queue.empty();
}
    
size_t ThreadPool::TaskQueue::size() const {
    std::unique_lock<std::mutex> lock(_mutex);
    return _queue.size();
}

} // namespace openspace
