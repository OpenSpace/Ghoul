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

#ifndef __GHOUL___LUA_HELPER___H__
#define __GHOUL___LUA_HELPER___H__

#include <ghoul/misc/exception.h>

struct lua_State;

namespace ghoul { class Dictionary; }

namespace ghoul::lua {

struct LuaRuntimeException : public RuntimeError {
    explicit LuaRuntimeException(std::string message);
};

struct LuaFormatException : public LuaRuntimeException {
    explicit LuaFormatException(std::string message, std::string file = "");
    std::string filename;
};

struct LuaLoadingException : public LuaRuntimeException {
    explicit LuaLoadingException(std::string error, std::string file = "");
    std::string errorMessage;
    std::string filename;
};

struct LuaExecutionException : public LuaRuntimeException {
    explicit LuaExecutionException(std::string error, std::string file = "");
    std::string errorMessage;
    std::string filename;
};

/**
 * Returns the location of the calling function using <code>luaL_where</code> and returns
 * that location as a string. This method is just a wrapper around this function and its
 * use is for non-fatal error handling.
 * \param state The Lua state that is to be exermined
 * \return The location of the function whose stack is being tested
 * \pre \p state must not be nullptr
 */
std::string errorLocation(lua_State* state);


/**
 * Returns a string describing the \p state's current stack. The values of each entry in
 * the stack is printed, which includes tables (printed recursively), but excludes
 * <code>function</code>, <code>thread</code>, <code>userdata</code>, and
 * <code>light userdata</code> objects, for which only the type is returned. The returned
 * string is in the the format:
 * \verbatim
<code>
1: <entry>
2: <entry>
...
</code>
\endverbatim
 * If the stack does not contain any values, an empty string is returned.
 * \param state The Lua state that will have its stack printed
 * \return The description of the current \p state's stack
 * \pre \p state must not be nullptr
 */
std::string stackInformation(lua_State* state);


/**
 * Loads a Lua script into the given #ghoul::Dictionary%, extending the passed in
 * dictionary. This method will overwrite values with the same keys, but will not remove
 * any other keys from the dictionary. The script contained in the file must return a
 * single table, which is then parsed and included into the #ghoul::Dictionary. The single
 * restriction on the script is that it can only contain a pure array-style table (= only
 * indexed by numbers) or a pure dictionary-style table (= no numbering indices).
 * \param filename The filename pointing to the script that is executed.
 * \param dictionary The #ghoul::Dictionary into which the values from the script are
 * added
 * \param state If this is set to a valid lua_State, this state is used instead of
 * creating a new state. It is the callers responsibility to ensure that the passed state
 * is valid if this parameter is not <code>nullptr</code>. After calling this method, the
 * stack of the passed state will be empty after this function returns.
 * \throws FormattingException If the #ghoul::Dictionary contains mixed keys of both type
 * <code>string</code> and type <code>number</code>
 * \throws FormattingException If the script did not return anything else but a table
 * \throws LuaRuntimeException If there was an error initializing a new Lua state if it
 * was necessary
 * \pre \p filename must not be empty
 * \pre \p filename must be a path to an existing file
 * \post The \p state%'s stack is empty
 */
void loadDictionaryFromFile(const std::string& filename, ghoul::Dictionary& dictionary,
    lua_State* state = nullptr);

/**
 * Loads a Lua script and returns it as a #ghoul::Dictionary%. The script contained in the
 * file must return a single table, which is then parsed and included into the
 * #ghoul::Dictionary. The single restriction on the script is that it can only contain a
 * pure array-style table (= only indexed by numbers) or a pure dictionary-style table
 * (= no numbering indices).
 * \param filename The filename pointing to the script that is executed.
 * \param dictionary The #ghoul::Dictionary into which the values from the script are
 * added
 * \param state If this is set to a valid lua_State, this state is used instead of
 * creating a new state. It is the callers responsibility to ensure that the passed state
 * is valid if this parameter is not <code>nullptr</code>. After calling this method, the
 * stack of the passed state will be empty after this function returns.
 * \return The ghoul::Dictionary described by the Lua script
 * \throws FormattingException If the #ghoul::Dictionary contains mixed keys of both type
 * <code>string</code> and type <code>number</code>
 * \throws FormattingException If the script did not return anything else but a table
 * \throws LuaRuntimeException If there was an error initializing a new Lua state if it
 * was necessary
 * \pre \p filename must not be empty
 * \pre \p filename must be a path to an existing file
 * \post The \p state%'s stack is empty
 */
ghoul::Dictionary loadDictionaryFromFile(const std::string& filename,
    lua_State* state = nullptr);

/**
 * Loads a Lua configuration into the given #ghoul::Dictionary%, extending the passed in
 * dictionary. This method will overwrite values with the same keys, but will not remove
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
 * \throws ghoul::lua::FormattingException If the #ghoul::Dictionary contains mixed
 * keys of both type <code>string</code> and type <code>number</code>
 * \throws FormattingException If the script did not return anything else but a table
 * \pre \p script must not be empty
 * \post \p state%'s stack is empty
 */
void loadDictionaryFromString(const std::string& script, ghoul::Dictionary& dictionary,
    lua_State* state = nullptr);

/**
 * Loads a Lua script and returns it as a #ghoul::Dictionary%. The script contained in the
 * string must return a single table, which is then parsed. The single restriction on the
 * script is that it can only contain a pure array-style table (= onl indexed by numbers)
 * or a pure dictionary-style table (= no numbering indices).
 * \param script The source code of the script that is executed
 * \param state If this is set to a valid lua_State, this state is used instead of
 * creating a new state. It is the callers responsibility to ensure that the passed state
 * is valid. After calling this method, the stack of the passed state will be empty.
 * \return The ghoul::Dictionary described by the Lua script
 * \throws ghoul::lua::FormattingException If the #ghoul::Dictionary contains mixed
 * keys of both type <code>string</code> and type <code>number</code>
 * \throws FormattingException If the script did not return anything else but a table
 * \pre \p script must not be empty
 * \post \p state%'s stack is empty
 */
ghoul::Dictionary loadDictionaryFromString(const std::string& script,
    lua_State* state = nullptr);

/**
 * Uses the Lua \p state to populate the provided ghoul::Dictionary%, extending the passed
 * \p dictionary. This method will overwrite values with the same keys, but will not
 * remove any other keys from the dictionary. The \p state must have a single table object
 * at the top of the stack. The table can only contain a pure array-style table (= only
 * indexed by numbers) or a pure dictionary-style table (= no numbering indices).
 * \param state The Lua state that is used to populate the \p dictionary
 * \param dictionary The #ghoul::Dictionary into which the values from the stack are
 * added
 * \throws LuaFormatException If the \p dictionary contains mixed keys of both type
 * <code>string</code> and type <code>number</code>
 * \pre \p state must not be nullptr
 * \post \p state%'s stack is unchanged
 */
void luaDictionaryFromState(lua_State* state, ghoul::Dictionary& dictionary);

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
 * \return The converted string
 */
std::string luaTypeToString(int type);

/**
 * Creates a new Lua state and initializes it with the default Lua libraries.
 * \param loadStandardLibraries If \c true, the Lua standard libraries will be loaded into
 * the newly created state by means of a \c luaL_openlibs call
 * \return A valid new Lua state initialized with the default Lua libraries
 * \throw LuaRuntimeException If there was an error creating the new Lua state
 */
lua_State* createNewLuaState(bool loadStandardLibraries = true);

/**
 * Destroys the passed lua state and frees all memory that is associated with it.
 * \param state The Lua state that is to be deleted
 * \pre \p state must not be nullptr
 */
void destroyLuaState(lua_State* state);

/**
 * This function executes the Lua script pointed to by \p filename using the passed
 * <code>lua_State</code> \p state.
 * \param state The Lua state that is used to execute the script
 * \param filename The file path that contains the Lua script that is executed
 * \throw LuaLoadingException If there was an error loading the script
 * \throw LuaExecutionError If there was an error executing the script
 * \throw FileNotFoundError If the file does not exist
 * \pre \p state must not be nullptr
 * \pre \p filename must not be empty
 * \pre \p filename must be a file that exist
 */
void runScriptFile(lua_State* state, const std::string& filename);

/**
 * This function executes the Lua script provided as plain text in \p script using the
 * passed <code>lua_State</code> \p state.
 * \throw LuaLoadingException If there was an error loading the script
 * \throw LuaExecutionError If there was an error executing the script
 * \pre \p state must not be nullptr
 * \pre \p script must not be empty
 */
void runScript(lua_State* state, const std::string& script);

/**
 * This function calls into the Lua library function <code>luaL_checkstring</code> and,
 * if successful, pops the string from the stack. If the top argument is not a string,
 * the stack remains untouched.
 * \param L The Lua stack to be inspected
 * \return The string or <code>nullptr</code> if the top element is not a string
 */
const char* checkStringAndPop(lua_State* L);

/**
 * Checks the number of arguments on the Lua stack against the \p expected number of
 * parameters. If the numbers do not agree, an error is logged and a
 * ghoul::lua::LuaExecutionException is raised.
 * \param L The Lua stack from which the arguments are checked
 * \param expected The number of expected arguments
 * \param component The name of the component that is used for the error message
 * \return The number of arguments
 * \throw ghoul::lua::LuaExecutionException if the number of arguments is wrong
 */
int checkArgumentsAndThrow(lua_State* L, int expected, const char* component = nullptr);

/**
 * Checks if the number of arguments on the Lua stack is equal to \p expected1 or
 * \p expected2 parameters. If the numbers do not agree, an error is logged and a
 * ghoul::lua::LuaExecutionException is raised.
 * \param L The Lua stack from which the arguments are checked
 * \param expected1 The first allowed number of expected arguments
 * \param expected2 The second allowed number of expected arguments
 * \param component The name of the component that is used for the error message
 * \return The number of arguments
 * \throw ghoul::lua::LuaExecutionException if the number of arguments is wrong
 */
int checkArgumentsAndThrow(lua_State* L, int expected1, int expected2,
    const char* component = nullptr);

/**
 * Checks if the number of arguments on the Lua stack is in the \p range of allowed
 * values. If the numbers do not agree, an error is logged and a
 * ghoul::lua::LuaExecutionException is raised.
 * \param L The Lua stack from which the arguments are checked
 * \param range The inclusive range that the number of arguments are tested against
 * \param component The name of the component that is used for the error message
 * \return The number of arguments
 * \throw ghoul::lua::LuaExecutionException if the number of arguments is wrong
 */
int checkArgumentsAndThrow(lua_State* L, std::pair<int, int> range,
    const char* component = nullptr);


namespace internal {
    void deinitializeGlobalState();
} // namespace internal

} // namespace ghoul::lua

#endif // __GHOUL___LUA_HELPER___H__
