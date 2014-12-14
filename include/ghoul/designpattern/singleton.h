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

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <iostream>
#include <cassert>

namespace ghoul {

template <class T>
class Singleton {
public:
    template <typename... Args>
    static void initialize(Args... args) {
        assert( ! isInitialized());
        _instance = new T(std::forward<Args>(args)...);
    }
    
    static void deinitialize() {
        assert(isInitialized());
        delete _instance;
        _instance = nullptr;
    }
    
    static bool isInitialized() {
        return _instance != nullptr;
    }
    
    static T& ref() {
        assert(isInitialized());
        return *_instance;
    }
protected:
    Singleton() {};
    ~Singleton() {};
    
private:

    // protecting against evil
    Singleton(const Singleton&) = delete;
    Singleton(const Singleton&&) = delete;
    Singleton& operator= (const Singleton& rhs) = delete;
    
    // instance member
    static T* _instance;
    
}; // Singleton

template <class T> T* Singleton<T>::_instance = nullptr;

} // namespace ghoul
#endif