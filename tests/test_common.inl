/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#ifdef WIN32
#define START_TIMER(__name__, __stream__, __num__) \
    __stream__ << #__name__; \
    for (int __i__ = 0; __i__ < __num__; ++__i__) { \
        LARGE_INTEGER __name__##Start; \
        LARGE_INTEGER __name__##End; \
        QueryPerformanceCounter(&__name__##Start)

#define FINISH_TIMER(__name__, __stream__) \
        QueryPerformanceCounter(&__name__##End); \
        LARGE_INTEGER freq; \
        QueryPerformanceFrequency(&freq); \
        const double freqD = double(freq.QuadPart) / 1000000.0; \
        __stream__ << '\t' << \
        ((__name__##End.QuadPart - __name__##Start.QuadPart) / freqD) << "us"; \
    } \
    __stream__ << '\n';

#else
#include <chrono>

#define START_TIMER(__name__) \
        std::chrono::high_resolution_clock::time_point __name__##Start = \
        std::chrono::high_resolution_clock::now() \

#define FINISH_TIMER(__name__, __stream__) \
    do { \
        std::chrono::high_resolution_clock::time_point __name__##End = \
        std::chrono::high_resolution_clock::now(); \
        std::chrono::microseconds d = __name__##End - __name__##Start; \
        __stream__ << #__name__ << '\t' << d.count() << char(230) << "s" << '\n'; \
    } while (false)
#endif
