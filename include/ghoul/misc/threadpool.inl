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

namespace ghoul {

template <typename F, typename... Arg>
auto ThreadPool::queue(F&& f, Arg&&... arg)->std::future<decltype(f(arg...))> {
    using ReturnType = decltype(f(arg...));
    // We wrap the packaged_task into a shared pointer so that we can store it in the
    // lambda expression below. The capture of the lambda expression will keep this
    // packaged_task alive
    auto pck = std::make_shared<std::packaged_task<ReturnType ()>>(
        std::bind(std::forward<F>(f), std::forward<Arg>(arg)...)
    );
    
    // Push the packaged packaged_task onto the queue of work items
    _taskQueue.push(
        [pck]() { (*pck)(); }
    );
    
    // Get the future of the result (which might be std::future<void>, but that is not a
    // problem
    auto future = pck->get_future();

    // Notify a potentially waiting thread that a new task is available
    _cv.notify_one();

    // And return the future back to the caller
    return future;
}

} // namespace ghoul
