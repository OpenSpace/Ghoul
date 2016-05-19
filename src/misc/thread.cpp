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

#include <ghoul/misc/thread.h>

#ifdef WIN32
#include <Windows.h>
#endif

namespace ghoul {
namespace thread {

int convertThreadPriority(ThreadPriority p) {
#ifdef WIN32
    switch (p) {
        case ThreadPriority::Lowest:
            return THREAD_PRIORITY_LOWEST;
        case ThreadPriority::BelowNormal:
            return THREAD_PRIORITY_BELOW_NORMAL;
        case ThreadPriority::Normal:
            return THREAD_PRIORITY_NORMAL;
        case ThreadPriority::AboveNormal:
            return THREAD_PRIORITY_ABOVE_NORMAL;
        case ThreadPriority::Highest:
            return THREAD_PRIORITY_HIGHEST;
    }
#else
    return 0;
#endif 
}


void setPriority(std::thread& t, ThreadPriority priority) {
#ifdef WIN32
    std::thread::native_handle_type h = t.native_handle();
    //SetPriorityClass(h, IDLE_PRIORITY_CLASS);
    SetThreadPriority(h, convertThreadPriority(priority));
#else
    // TODO: Implement thread priority ---abock
#endif
}

void setThreadBackground(std::thread& t, Background background) {
#ifdef WIN32
    int m;
    if (background == Background::Yes)
        m = THREAD_MODE_BACKGROUND_BEGIN;
    else
        m = THREAD_MODE_BACKGROUND_END;

    std::thread::native_handle_type h = t.native_handle();
    //SetPriorityClass(h, IDLE_PRIORITY_CLASS);
    SetThreadPriority(h, m);
#else

#endif
}

} // namespace thread
} // namespace ghoul
