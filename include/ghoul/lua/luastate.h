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

#ifndef __GHOUL___LUASTATE___H__
#define __GHOUL___LUASTATE___H__

struct lua_State;

namespace ghoul::lua {

/**
 * This class is a thin RAII wrapper around a lua_State pointer as returned by the Lua
 * library. The constructor will create a new state with optionally registering the
 * default Lua standard libraries. The destructor will destroy the created state.
 */
class LuaState {
public:
    /**
     * The constructor will create a new Lua state and optionally fill it with the Lua
     * standard libraries, if \p includeStandardLibraries is \c true.
     * \param includeStandardLibraries If \c true, the created Lua state will contain the
     * set of Lua standard libraries.
     * \throw LuaRuntimeException If an error occurs during the state creation
     */
    explicit LuaState(bool includeStandardLibraries = true);

    /**
     * Destroys the created Lua state and frees all the related memory.
     */
    ~LuaState();

    /**
     * Converts this LuaState into a \c lua_State pointer for compatibility with other
     * parts that might still rely on bare \c lua_State pointers
     * \return The contained \c lua_State pointer
     */
    operator lua_State*() const;

private:
    /// The stored \c lua_State
    lua_State* _state;
};

}  // namespace ghoul::lua

#endif // __GHOUL___LUASTATE___H__
