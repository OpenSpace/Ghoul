/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#include <ghoul/opengl/programobjectmanager.h>

#include <ghoul/fmt.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/opengl/programobject.h>

namespace ghoul::opengl {

ProgramObjectManager::~ProgramObjectManager() {
    ghoul_assert(_programs.empty(), "ProgramObjects were left at the end of the program");

    // This loop is just to check *which* programs were left. If everything goes fine, the
    // next loop should iterate 0 times
    for (const std::pair<const std::string, Info>& p : _programs) {
        (void)p; // Silence an unused parameter warning in Release builds
        ghoul_assert(
            p.second.refCount == 0,
            "Ref count for ProgramObject '" + p.first + "' was not 0"
        );
    }
}

void ProgramObjectManager::releaseAll(Warnings emitWarnings) {
    // If we are not interested in the warnings, we can just clear the map and let the
    // destructors do their job right away.
    if (!emitWarnings) {
        _programs.clear();
        return;
    }

    if (!_programs.empty()) {
        LWARNINGC(
            "ProgramObjectManager",
            "Remaining ProgramObjects detected. There was probably some error during "
            "deinitialization that caused this."
        );
    }
    for (std::pair<const std::string, Info>& p : _programs) {
        LWARNINGC(
            p.first,
            fmt::format("Remaining reference counter: {}", p.second.refCount)
        );
        // We have to destroy the ProgramObject now; otherwise it will destroyed when this
        // ProgramObjectManager leaves scope, at which point there might not be a valid
        // OpenGL state left
        p.second.program = nullptr;
    }

}

ProgramObject* ProgramObjectManager::requestProgramObject(const std::string& name,
                                                 const CreationCallback& creationFunction)
{
    auto it = _programs.find(name);
    if (it == _programs.end()) {
        // If we couldn't find the name, we'll have to create the ProgramObject and
        // initialize the reference counter to 1
        LDEBUGC(name, "Creating shader program");
        Info info {
            creationFunction(),
            1
        };
        ProgramObject* p = info.program.get();
        _programs[name] = std::move(info);
        return p;
    }
    else {
        // If we found the name, we increase the reference counter and return the pointer
        ++(it->second.refCount);
        LDEBUGC(
            name,
            fmt::format("Reusing shader program (new ref count {})", it->second.refCount)
        );
        return it->second.program.get();
    }
}

void ProgramObjectManager::releaseProgramObject(const std::string& name,
                                           const DestructionCallback& destructionFunction)
{
    auto it = _programs.find(name);
    ghoul_assert(it != _programs.end(), "Could not find ProgramObject '" + name + "'");
    ghoul_assert(it->second.refCount >= 0, "Ref count cannot be negative");

    --(it->second.refCount);
    LDEBUGC(name, fmt::format("Ref count decreased to {}", it->second.refCount));
    if (it->second.refCount == 0) {
        // This was the final call, so we will delete the ProgramObject
        destructionFunction(it->second.program.get());
        it->second.program = nullptr;
        _programs.erase(it);
    }
}

} // namespace ghoul::opengl
