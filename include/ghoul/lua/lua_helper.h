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

struct lua_State;

namespace ghoul {
namespace lua {

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
std::string lua_logTable(lua_State* state,logging::LogManager::LogLevel level = 
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
std::string lua_logStack(lua_State* state, logging::LogManager::LogLevel level = 
                         logging::LogManager::LogLevel::Info);


} // namespace lua
} // namespace ghoul

#endif // __LUAHELPER_H__
