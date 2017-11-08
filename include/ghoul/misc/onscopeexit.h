/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#ifndef __GHOUL___ONSCOPEEXIT___H__
#define __GHOUL___ONSCOPEEXIT___H__

#include <functional>

namespace ghoul {

#define __MERGE(a,b)  a##b
#define __LABEL(a) __MERGE(unique_name_, a)
/**
 * This macro constructs a new OnScopeExit object with a unique name and the passed
 * \p func. Due to the usage of the <code>__LINE__</code> macro to create a unique
 * variable name, this macro cannot be used multiple times in the same row.
 */
#define OnExit(func) ghoul::OnScopeExit __LABEL(__LINE__)(func)

/**
 * This structure makes it easy to execute a function, for example a lambda, at the end of
 * the scope. The execution order of multiple OnScopeExit objects is determined by the
 * usual stack unwinding rules.
 *
 * As the function will be called in the destructor, it is imperative that the function
 * does not throw an exception.
 */
struct OnScopeExit {
    /**
     * Constructs an OnScopeExit exit that will execute the \p function when this object
     * is destructed.
     * \p function The function that will be executed when this object is destroyed. As
     * this function is executed in the destructor, it may never throw an exception.
     */
    explicit OnScopeExit(std::function<void()> function);

    /**
     * Executes the function passed in the constructor if it is a valid function.
     */
    ~OnScopeExit();

    /**
     * Removes the function that would otherwise be called at the end of this objects
     * lifetime.
     */
    void clear();

private:
    OnScopeExit() = delete;
    OnScopeExit(const OnScopeExit&) = delete;
    OnScopeExit(OnScopeExit&& rhs) = delete;
    OnScopeExit& operator=(const OnScopeExit& rhs) = delete;
    OnScopeExit& operator=(OnScopeExit&& rhs) = delete;

    /// The function that is passed in the constructor and called in the destructor
    std::function<void()> _function;
};

} // namespace ghoul

#endif // __GHOUL___ONSCOPEEXIT___H__
