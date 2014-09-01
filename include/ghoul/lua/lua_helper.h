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
 * Logs the Lua table that is located on the top of the stack in the passed
 * <code>state</code> with the passed ghoul::logging::LogManager::LogLevel and returns the
 * logged string. The key-value pair for each entry in the table is printed, which might
 * cause the function to be called recursively. The output will be written the format:
 * <code>{ key = value key = value ... }</code>. If the top of the stack does not
 * represent a Lua table, an empty string is returned
 * \param state The Lua state that contains the table to be logged
 * \param level The logging::LogManager::LogLevel at which the table will be logged
 * \return The same string that was logged, or <code>""</code> if the <code>state</code>
 * was not valid
 */
std::string logTable(lua_State* state,logging::LogManager::LogLevel level = 
                         logging::LogManager::LogLevel::Info);

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
 * \return Returns <code>true</code> if the loading succeeded; <code>false</code>
 * otherwise.
 * \throws #ghoul::lua::FormattingException If the #ghoul::Dictionary contains mixed
 * keys of both type <code>string</code> and type <code>number</code>
 */
bool loadDictionaryFromFile(const std::string& filename, ghoul::Dictionary& dictionary);

/**
* Loads a Lua configuration into the given #ghoul::Dictionary%, extending the passed in
* dictionary. This method will overwrite value with the same keys, but will not remove
* any other keys from the dictionary. The script contained in the file must return a
* single table, which is then parsed and included into the #ghoul::Dictionary. The single
* restriction on the script is that it can only contain a pure array-style table (= only
* indexed by numbers) or a pure dictionary-style table (= no numbering indices).
* \param script The source code of the script that is executed
* \param dictionary The #ghoul::Dictionary into which the values from the script are
* added
* \return Returns <code>true</code> if the loading succeeded; <code>false</code>
* otherwise.
* \throws #ghoul::lua::FormattingException If the #ghoul::Dictionary contains mixed
* keys of both type <code>string</code> and type <code>number</code>
*/
bool loadDictionaryFromString(const std::string& script, ghoul::Dictionary& dictionary);

/**
 * Converts the Lua type to a human-readable string. The supported types are:
 * \verbatim
 * LUA_TNONE: None
 * LUA_TNIL: Nil
 * LUA_TBOOLEAN: Boolean
 * LUA_TLIGHTUSERDATA: Light UserData
 * LUA_TNUMBER: Number
 * LUA_TSTRING: String
 * LUA_TTABLE: Table
 * LUA_TFUNCTION: Function
 * LUA_TUSERDATA: UserData
 * LUA_TTHREAD: Thread
 * \param type A Lua type that should be converted to a string
 * \return The converted string or <code>""</code> if <code>type</code> was an illegal
 * type
 */
std::string luaTypeToString(int type);


void luaDictionaryFromState(lua_State* L, ghoul::Dictionary& d);

} // namespace lua
} // namespace ghoul

#endif // __LUAHELPER_H__
