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

#include <ghoul/misc/thread.h>

#include <ghoul/misc/assert.h>
#include <ghoul/misc/exception.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

namespace ghoul::thread {

namespace {

int convertThreadPriorityLevel(ThreadPriorityClass c, ThreadPriorityLevel p) {
#ifdef WIN32
    (void)c;
    switch (p) {
        case ThreadPriorityLevel::Lowest:      return THREAD_PRIORITY_LOWEST;
        case ThreadPriorityLevel::BelowNormal: return THREAD_PRIORITY_BELOW_NORMAL;
        case ThreadPriorityLevel::Normal:      return THREAD_PRIORITY_NORMAL;
        case ThreadPriorityLevel::AboveNormal: return THREAD_PRIORITY_ABOVE_NORMAL;
        case ThreadPriorityLevel::Highest:     return THREAD_PRIORITY_HIGHEST;
        default:                               throw MissingCaseException();

    }
#else
    switch (c) {
        case ThreadPriorityClass::Idle:
        case ThreadPriorityClass::Normal:
            return 0;
        case ThreadPriorityClass::High:
            switch (p) {
                // min-max values come from the man page of getschedparam
                // normal value was measured on macOS to be the default value
                // below/above normal values are halfway between other values
                case ThreadPriorityLevel::Lowest:      return 1;
                case ThreadPriorityLevel::BelowNormal: return 16;
                case ThreadPriorityLevel::Normal:      return 32;
                case ThreadPriorityLevel::AboveNormal: return 66;
                case ThreadPriorityLevel::Highest:     return 99;
                default:                               throw MissingCaseException();

            }
    }
    return 0;
#endif 
}

[[maybe_unused]] int convertThreadPriorityClass(ThreadPriorityClass c) {
#if defined WIN32
    switch (c) {
        case ThreadPriorityClass::Idle:   return IDLE_PRIORITY_CLASS;
        case ThreadPriorityClass::Normal: return NORMAL_PRIORITY_CLASS;
        case ThreadPriorityClass::High:   return HIGH_PRIORITY_CLASS;
        default:                          throw MissingCaseException();

    }
#elif defined __APPLE__ || defined __FreeBSD__
    switch (c) {
        case ThreadPriorityClass::Idle:   return SCHED_OTHER;
        case ThreadPriorityClass::Normal: return SCHED_OTHER;
        case ThreadPriorityClass::High:   return SCHED_RR;
        default:                          throw MissingCaseException();

    }
#else
    switch (c) {
        case ThreadPriorityClass::Idle:   return SCHED_IDLE;
        case ThreadPriorityClass::Normal: return SCHED_OTHER;
        case ThreadPriorityClass::High:   return SCHED_RR;
        default:                          throw MissingCaseException();
}
#endif
}
    
} // namespace

void setPriority(std::thread& t, ThreadPriorityClass priorityClass, 
                 ThreadPriorityLevel priorityLevel)
{
#ifdef WIN32
    std::thread::native_handle_type h = t.native_handle();
    
    SetPriorityClass(h, convertThreadPriorityClass(priorityClass));
    SetThreadPriority(h, convertThreadPriorityLevel(priorityClass, priorityLevel));
#else
    int policy;
    struct sched_param param;
    int res = pthread_getschedparam(t.native_handle(), &policy, &param);
    if (res != 0) {
        throw ghoul::RuntimeError(
            "Error accessing scheduling parameters with error " + std::to_string(res),
            "Thread"
        );
    }

    param.sched_priority = convertThreadPriorityLevel(priorityClass, priorityLevel);
    res = pthread_setschedparam(
        t.native_handle(),
        policy,
        &param
    );
    if (res != 0) {
        throw ghoul::RuntimeError(
            "Error setting scheduling parameters with error " + std::to_string(res),
            "Thread"
        );
    }
#endif
}

#ifdef WIN32
void setThreadBackground(std::thread& t, Background background) {
    int m;
    if (background) {
        m = THREAD_MODE_BACKGROUND_BEGIN;
    }
    else {
        m = THREAD_MODE_BACKGROUND_END;
    }

    std::thread::native_handle_type h = t.native_handle();
    SetThreadPriority(h, m);
}
#else
void setThreadBackground(std::thread&, Background) {}
#endif

} // namespace ghoul::thread
