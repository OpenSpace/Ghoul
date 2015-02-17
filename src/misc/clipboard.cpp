/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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
 * This file is inspired by Peter Kankowski (kankowski@narod.ru) and his work available  *
 * at http://www.strchr.com/hash_functions and http://www.strchr.com/crc32_popcnt        *
 *****************************************************************************************
 * Slicing-by-8 algorithms by Michael E. Kounavis and Frank L. Berry from Intel Corp.    *
 * http://www.intel.com/technology/comms/perfnet/download/CRC_generators.pdf             *
 ****************************************************************************************/

#include <ghoul/misc/clipboard.h>

#include <algorithm>
#include <sstream>

#ifdef WIN32
#include <Windows.h>
#endif

namespace {
#ifndef WIN32
    // Dangerous as fuck (if malicious input)
    bool exec(const std::string& cmd, std::string& value)
    {
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe)
            return false;

        const int buffer_size = 1024;
        char buffer[buffer_size];
        value = "";
        while (!feof(pipe))
        {
            if (fgets(buffer, buffer_size, pipe) != NULL)
            {
                value += buffer;
            }
        }
        pclose(pipe);
        return true;
    }
#endif
}

namespace ghoul {
    
std::string clipboardText() {
#ifdef WIN32
    // Try opening the clipboard
    if (!OpenClipboard(nullptr))
        return "";

    // Get handle of clipboard object for ANSI text
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr)
        return "";

    // Lock the handle to get the actual text pointer
    char * pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr)
        return "";

    // Save text in a string class instance
    std::string text(pszText);

    // Release the lock
    GlobalUnlock(hData);

    // Release the clipboard
    CloseClipboard();

    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    return text;
#elif defined(__APPLE__)
    std::string text;
    if (exec("pbpaste", text))
        return text.substr(0, text.length() - 1);
    return ""; // remove a line ending
#else
    std::string text;
    if (exec("xclip -o -sel c -f", text))
        return text.substr(0, text.length() - 1);
    return ""; // remove a line ending
#endif
}

bool setClipboardText(std::string text) {
#ifdef WIN32
    char *ptrData = nullptr;
    HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, text.length() + 1);

    ptrData = (char*)GlobalLock(hData);
    memcpy(ptrData, text.c_str(), text.length() + 1);

    GlobalUnlock(hData);

    if (!OpenClipboard(nullptr))
        return false;

    if (!EmptyClipboard())
        return false;

    SetClipboardData(CF_TEXT, hData);

    CloseClipboard();

    return true;
#elif defined(__APPLE__)
    std::stringstream cmd;
    cmd << "echo \"" << text << "\" | pbcopy";
    std::string buf;
    return exec(cmd.str(), buf);
#else
    std::stringstream cmd;
    cmd << "echo \"" << text << "\" | xclip -i -sel c -f";
    std::string buf;
    return exec(cmd.str(), buf);
#endif
}

} // namespace ghoul
