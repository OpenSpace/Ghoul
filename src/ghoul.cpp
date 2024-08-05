/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2024                                                               *
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

#include <ghoul/ghoul.h>

#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/lua/lua_helper.h>
#include <ghoul/misc/profiling.h>
#include <ghoul/opengl/renderdoc.h>

namespace ghoul {

void initialize() {
    ZoneScoped;

    ghoul::filesystem::FileSystem::initialize();
    ghoul::opengl::loadRenderDoc();
}

void deinitialize() {
    ZoneScoped;

    ghoul::lua::internal::deinitializeGlobalState();
    ghoul::filesystem::FileSystem::deinitialize();
}

std::string licenseText() {
    return "GHOUL\n\
General Helpful Open Utility Library\n\
\n\
Copyright(c) 2012 - 2024\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy of this\n\
software and associated documentation files(the \"Software\"), to deal in the Software\n\
without restriction, including without limitation the rights to use, copy, modify,\n\
merge, publish, distribute, sublicense, and / or sell copies of the Software, and to\n\
permit persons to whom the Software is furnished to do so, subject to the following\n\
conditions:\n\
\n\
The above copyright notice and this permission notice shall be included in all copies\n\
or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,\n\
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A\n\
PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT\n\
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF\n\
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE\n\
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n";
}

} // namespace ghoul
