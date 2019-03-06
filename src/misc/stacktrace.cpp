/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2019                                                               *
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
 * The Linux/Mac code is taken from Sarang Baheti                                        *
 * www.nullptr.me/2013/04/14/generating-stack-trace-on-os-x/                             *
 *****************************************************************************************
 * The VectorStackWalker is taken from the Inviwo project found at http://www.inviwo.org *
 ****************************************************************************************/

#include <ghoul/misc/stacktrace.h>

#if defined __unix__ || defined __APPLE__
#include <execinfo.h>
#include <cxxabi.h>
#include <stdio.h>
#include <stdlib.h>
#elif defined _MSC_VER
#include <StackWalker.h>
#endif

#ifdef _MSC_VER
/**
 * The internal class that is used by the StackWalker library to customize the output.
 * In our case, we want to store each stackframe in a vector.
 */
class VectorStackWalker : public StackWalker {
public:
    VectorStackWalker(StackWalkOptions level, std::vector<std::string>& vector)
        : StackWalker(level)
        , _vector(vector)
    {}

    void setVector(std::vector<std::string>& vector) {
        _vector = vector;
    }

protected:
    void OnOutput(LPCSTR szText) override {
        std::string str(szText);
        // Remove trailing newline character
        str = str.substr(0, str.size() - 1);
        _vector.push_back(str);
    }

private:
    std::vector<std::string>& _vector;
};

#endif


namespace ghoul {

std::vector<std::string> stackTrace() {
    std::vector<std::string> stackFrames;

#if defined __unix__ || defined __APPLE__
    const int MaxCallStackDepth = 128;

    int callstack[MaxCallStackDepth] = {};

    // Get the full stacktrace
    int nFrames = backtrace(reinterpret_cast<void**>(callstack), MaxCallStackDepth);

    // Unmangle the stacktrace to get it in a human-readable format
    char** strs = backtrace_symbols(reinterpret_cast<void**>(callstack), nFrames);

    stackFrames.reserve(nFrames);

    for (int i = 0; i < nFrames; ++i) {
        const int MaxFunctionSymbolLength = 1024;
        const int MaxModuleNameLength = 1024;
        const int MaxAddressLength = 48;

        std::vector<char> functionSymbol(MaxFunctionSymbolLength);
        std::vector<char> moduleName(MaxModuleNameLength);
        std::vector<char> addr(MaxAddressLength);
        int  offset = 0;

        //
        // Typically this is how the backtrace looks like:
        //
        // 0   <app/lib-name>     0x0000000100000e98 _Z5tracev + 72
        // 1   <app/lib-name>     0x00000001000015c1 _ZNK7functorclEv + 17
        // 2   <app/lib-name>     0x0000000100000f71 _Z3fn0v + 17
        // 3   <app/lib-name>     0x0000000100000f89 _Z3fn1v + 9
        // 4   <app/lib-name>     0x0000000100000f99 _Z3fn2v + 9
        // 5   <app/lib-name>     0x0000000100000fa9 _Z3fn3v + 9
        // 6   <app/lib-name>     0x0000000100000fb9 _Z3fn4v + 9
        // 7   <app/lib-name>     0x0000000100000fc9 _Z3fn5v + 9
        // 8   <app/lib-name>     0x0000000100000fd9 _Z3fn6v + 9
        // 9   <app/lib-name>     0x0000000100001018 main + 56
        // 10  libdyld.dylib      0x00007fff91b647e1 start + 0
        //

        // split the string, take out chunks out of stack trace
        // we are primarily interested in module, function and address
        sscanf(
            strs[i],
            "%*s %s %s %s %*s %d",
            moduleName.data(),
            addr.data(),
            functionSymbol.data(),
            &offset
        );

        int isValidCppName = 0;
        //  if this is a C++ library, symbol will be demangled
        //  on success function returns 0
        //
        char* functionName = abi::__cxa_demangle(
            functionSymbol.data(),
            nullptr,
            nullptr,
            &isValidCppName
        );

        const int MaxStackFrameSize = 4096;
        char stackFrame[MaxStackFrameSize] = {};
        if (isValidCppName == 0) {
            // success
            sprintf(
                stackFrame,
                "(%s)\t0x%s — %s + %d",
                moduleName.data(),
                addr.data(),
                functionName,
                offset
            );
        }
        else {
            //  in the above traceback (in comments) last entry is not
            //  from C++ binary, last frame, libdyld.dylib, is printed
            //  from here
            sprintf(
                stackFrame,
                "(%s)\t0x%s — %s + %d",
                moduleName.data(),
                addr.data(),
                functionName,
                offset
            );
        }

        if (functionName) {
            free(functionName);
        }

        stackFrames.push_back(std::string(stackFrame));
    }
    free(strs);
#elif WIN32
    static VectorStackWalker sw(StackWalker::OptionsAll, stackFrames);
    static bool IsInitialized = false;
    if (!IsInitialized) {
        // We only want to load the modules once as it is a very expensive operation
        sw.LoadModules();
        IsInitialized = true;
    }

    // The vector has to be set as the StackWalker library is statically initialized the
    // first time this function is called (and thus with a different stackFrames
    // reference. If this call is removed, invalid memory will be accessed
    sw.setVector(stackFrames);

    sw.ShowCallstack();
#endif

    return stackFrames;
}

} // namespace ghoul
