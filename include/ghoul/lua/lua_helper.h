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
 ****************************************************************************************/

#ifndef __LUAHELPER_H__
#define __LUAHELPER_H__

#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/dictionary.h>

#include <exception>
#include <stdexcept>

struct lua_State;

namespace ghoul {
namespace lua {

class FormattingException : public std::runtime_error {
public:
    FormattingException(const std::string&);
};


/**
 * Returns the location of the calling function using <code>luaL_where</code> and returns
 * that location as a string. This method is just a wrapper around this function and its
 * use is for non-fatal error handling.
 * \param L The Lua state that is to be exermined
 * \return The location of the function whose stack is being tested
 */
std::string errorLocation(lua_State* L);


/**
 * Logs the stack of the Lua state that is passed into the function at the provided <code>
 * level</code> and returns the logged string. The values of each entry in the stack is
 * printed, which includes tables (printed recursively), but excludes
 * <code>function</code> objects. For functions, merely the word <code>function</code> is
 * logged. The messages is writing in the the format:
 * \verbatim
<code>
1: <entry>
2: <entry>
...
</code>
\endverbatim
 * If the stack does not contain any values, an empty string is returned.
 * \param state The Lua state that will have its stack printed
 * \param level The logging::LogManager::LogLevel at which the stack will be logged
 * \return The same string that was logged, or <code>""</code> if the <code>state</code>
 * was not valid
 */
std::string logStack(lua_State* state, logging::LogManager::LogLevel level = 
                         logging::LogManager::LogLevel::Info);


/**
 * Loads a Lua configuration into the given #ghoul::Dictionary%, extending the passed in
 * dictionary. This method will overwrite value with the same keys, but will not remove
 * any other keys from the dictionary. The script contained in the file must return a
 * single table, which is then parsed and included into the #ghoul::Dictionary. The single
 * restriction on the script is that it can only contain a pure array-style table (= only
 * indexed by numbers) or a pure dictionary-style table (= no numbering indices).
 * \param filename The filename pointing to the script that is executed
 * \param dictionary The #ghoul::Dictionary into which the values from the script are
 * added
 * \param state If this is set to a valid lua_State, this state is used instead of
 * creating a new state. It is the callers responsibility to ensure that the passed state
 * is valid. After calling this method, the stack of the passed state will be empty.
 * \return Returns <code>true</code> if the loading succeeded; <code>false</code>
 * otherwise.
 * \throws #ghoul::lua::FormattingException If the #ghoul::Dictionary contains mixed
 * keys of both type <code>string</code> and type <code>number</code>
 */
bool loadDictionaryFromFile(
    const std::string& filename,
    ghoul::Dictionary& dictionary,
    lua_State* state = nullptr
    );

/**
 * Loads a Lua configuration into the given #ghoul::Dictionary%, extending the passed in
 * dictionary. This method will overwrite value with the same keys, but will not remove
 * any other keys from the dictionary. The script contained in the string must return a
 * single table, which is then parsed and included into the #ghoul::Dictionary. The single
 * restriction on the script is that it can only contain a pure array-style table (= only
 * indexed by numbers) or a pure dictionary-style table (= no numbering indices).
 * \param script The source code of the script that is executed
 * \param dictionary The #ghoul::Dictionary into which the values from the script are
 * added
 * \param state If this is set to a valid lua_State, this state is used instead of
 * creating a new state. It is the callers responsibility to ensure that the passed state
 * is valid. After calling this method, the stack of the passed state will be empty.
 * \return Returns <code>true</code> if the loading succeeded; <code>false</code>
 * otherwise.
 * \throws #ghoul::lua::FormattingException If the #ghoul::Dictionary contains mixed
 * keys of both type <code>string</code> and type <code>number</code>
 */
bool loadDictionaryFromString(
    const std::string& script,
    ghoul::Dictionary& dictionary,
    lua_State* state = nullptr
    );

/**
 * Converts the Lua type to a human-readable string. The supported types are:
 * \verbatim
 LUA_TNONE: None
 LUA_TNIL: Nil
 LUA_TBOOLEAN: Boolean
 LUA_TLIGHTUSERDATA: Light UserData
 LUA_TNUMBER: Number
 LUA_TSTRING: String
 LUA_TTABLE: Table
 LUA_TFUNCTION: Function
 LUA_TUSERDATA: UserData
 LUA_TTHREAD: Thread
 \endverbatim
 * \param type A Lua type that should be converted to a string
 * \return The converted string or <code>""</code> if <code>type</code> was an illegal
 * type
 */
std::string luaTypeToString(int type);

/**
 * Creates a new Lua state and initializes it with the default Lua libraries. If the
 * allocation of memory fails, an error is logged and <code>nullptr</code> is returned.
 * \return A valid Lua state or <code>nullptr</code> if the state creation failed
 */
lua_State* createNewLuaState();


void luaDictionaryFromState(lua_State* L, ghoul::Dictionary& d);

} // namespace lua
} // namespace ghoul

#endif // __LUAHELPER_H__
