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

#ifndef __CONFIGURATIONMANAGER_H__
#define __CONFIGURATIONMANAGER_H__

#include <ghoul/logging/logmanager.h>
#include <string>
#include <vector>

struct lua_State;

namespace ghoul {

/**
 * This class provides a generic way of accessing nested configuration variables of 
 * different types in an efficient way. The main use is for managing settings and
 * configurations, but it can be used as a general key,value map as well.  The main
 * work flow for usage is to call #initialize with the base configuration script. The
 * default argument requires the ${SCRIPTS} token to be defined
 * (#FileSystem::registerPathToken). After the initialization, an arbitrary amount of
 * configurations can be loaded(#loadConfiguration), which have to have to be valid Lua
 * scripts defining a table. For example:
 * \verbatim
{
    settingA = 2.0,
    value = 1,
    color = { r = 1, g = 0, b = 1 }
}\endverbatim
 * The configuration script will automatically prepend a <code>return</code> to make it
 * into a valid chuck. If multiple scripts define the same variable, the latest assignment
 * gets precedence. For example if three configuration files are loaded in the order
 * <code>a.cfg</code>, <code>b.cfg</code>, <code>c.cfg</code> and they all write to a
 * variable <code>value</code>, the value set by <code>c.cfg</code> will be stored, with
 * the values of <code>a.cfg</code> and <code>b.cfg</code> overwritten. The same holds
 * true for #setValue, which will silently overwrite already existing keys in the table.
 * All commands accept nested identifiers, like <code>general.color.r</code> and will
 * resolve the recursion efficiently. The #setValue method will generate all intermediate
 * tables automatically should they not exist. The #keys method returns for a specific
 * location all available settings, while the #hasKey method can check for existance of 
 * individual keys.
 *
 * The allowed types which can be stored (#setValue) and requested (#getValue) in the
 * configuration are:
 * - <code>bool</code>
 * - <code>char</code>
 * - <code>signed char</code>
 * - <code>unsigned char</code>
 * - <code>wchar_t</code>
 * - <code>short</code>
 * - <code>unsigned short</code>
 * - <code>int</code>
 * - <code>unsigned int</code>
 * - <code>long</code>
 * - <code>unsigned long</code>
 * - <code>long long</code>
 * - <code>unsigned long long</code>
 * - <code>float</code>
 * - <code>double</code>
 * - <code>long double</code> (will currently be truncated to <code>double</code>)
 * - <code>std::string</code>
 * - <code>glm::vec2</code>, <code>glm::vec3</code>, <code>glm::vec4</code>
 * - <code>glm::dvec2</code>, <code>glm::dvec3</code>, <code>glm::dvec4</code>
 * - <code>glm::ivec2</code>, <code>glm::ivec3</code>, <code>glm::ivec4</code>
 * - <code>glm::uvec2</code>, <code>glm::uvec3</code>, <code>glm::uvec4</code>
 * - <code>glm::bvec2</code>, <code>glm::bvec3</code>, <code>glm::bvec4</code>
 * - <code>glm::mat2x2</code>, <code>glm::mat2x3</code>, <code>glm::mat2x4</code>
 * - <code>glm::mat3x2</code>, <code>glm::mat3x3</code>, <code>glm::mat3x4</code>
 * - <code>glm::mat4x2</code>, <code>glm::mat4x3</code>, <code>glm::mat4x4</code>
 * - <code>glm::dmat2x2</code>, <code>glm::dmat2x3</code>, <code>glm::dmat2x4</code>
 * - <code>glm::dmat3x2</code>, <code>glm::dmat3x3</code>, <code>glm::dmat3x4</code>
 * - <code>glm::dmat4x2</code>, <code>glm::dmat4x3</code>, <code>glm::dmat4x4</code>
 * A call to #getValue using an invalid type will return <code>false</code> and log an
 * error message. A call to #setValue using an invalid type will just log an error.
 */

class ConfigurationManager {
public:
    /**
     * Empty constructor initializing the necessary Lua state to nullptr. #initialize has
     * to be called before the ConfigurationManager can be used.
     */
    ConfigurationManager();

    /**
     * The #deinitialize method has to be called before the destructor will be executed to
     * release the Lua state and prevent memory leaks. If the code is compiled with 
     * <code>GHL_DEBUG</code> defined, a warning will be logged if the
     * ConfigurationManager was not #deinitialize%d before.
     */
    ~ConfigurationManager();

    /**
     * Initializes the ConfigurationManager with the <code>configurationScript</code>.
     * This method can only be called once for each ConfigurationManager and will load,
     * compile, and test the passed script. It will perform checks if all necessary
     * functions and tables are available in the script. If any error occurs during the
     * loading, compilation, or testing, <code>false</code> will be returned and the error
     * is logged. If everything passes, <code>true</code> is returned. The main script 
     * has to adhere to the following interface:
     * - A function <code>loadFunction</code>, called with a filename as paramenter, has
     *   to add the contents of the pointed file to the current configuration state.
     * - A function <code>getValue</code>, called with a string key, has to return the
     *   current value of the configuration of that key or <code>nil</code> if the key was
     *   not present in the list.
     * - A function <code>setValue</code>, called with a string key and the value to be
     *   set, must store the passed value so that it will be available to future
     *   <code>getValue</code> calls.
     * - A function <code>getKeys</code>, called with a string location, must return a
     *   table containing the available keys for the settings pointed to by the location.
     * - A function <code>hasKey</code>, called with a string key that checks if the
     *   designated key exists in the configuration.
     * \param configurationScript The script containing the main script that handles all
     * the interaction to the Lua state. The filename to the script will automatically be
     * converted using the #FileSystem::absolutePath method. If no configuration script
     * is provided, a hard-coded default script will be used instead
     * \return <code>true</code> if it script was successfully compiled and tested and the
     * ConfigurationManager is ready to be used. <code>false</code> otherwise
     */
    bool initialize(const std::string& configurationScript = "");
    
    /**
     * Cleans up the Lua state and makes the ConfigurationManager ready to be destroyed
     * or re%initialize%d again later. If it is reinitialized, all previous values have
     * been lost.
     */
    void deinitialize();

    /**
     * Load the configuration at the <code>filename</code> into the local store. Each new
     * configuration will be merged with the current state, which will overwrite duplicate
     * settings. The configuration has to be provided as a semi-valid Lua script, i.e., a
     * chunk that is made valid by adding a <code>return</code> in front of it. All
     * default Lua libraries are available in the configuration script and will 
     * automatically be loaded in the #initialize step.
     * \param filename The path to the configuration script that is to be loaded.The
     * filename will automatically be converted using the #FileSystem::absolutePath
     * method
     * \return <code>true</code> if the configuration was successfully loaded;
     * <code>false</code> if an error occurred
     */
    bool loadConfiguration(const std::string& filename);

    /**
     * This method returns all the keys that are available at a certain
     * <code>location</code>. If the parameter is <code>""</code>, the keys of the root of
     * the configuration will be returned. The location can consist of many hierarchies of
     * tables and the configuration hierarchy will automatically be traversed. If one of
     * the identifiers during the traversal does not exist or does not denote a
     * <code>table</code>, an empty vector is returned. Otherwise, each element in the
     * vector contains a valid key <code>k</code> so that <code>location.k</code> can be
     * used in a call to #getValue or #setValue. The method will only return keys of the
     * direct children and will not traverse the table further down.
     * \param location The location in the table for which the keys should be fetched
     * \return The available keys at the provided <code>location</code>
     */
    std::vector<std::string> keys(const std::string& location = "");

    /**
     * This method checks if a non-nil value for the <code>key</code> is available. The
     * method, like #setValue and #getvalue accepts nested identifiers and will return
     * <code>false</code> if one of the subtables does not exist. Returns
     * <code>true</code> if the value at the <code>key</code> exists.
     * \param key The key that is to be tested. Can be a nested identifier
     * \return <code>true</code> if the value pointed to by <code>key</code> is non-nil.
     * <code>false</code> otherwise
     */
    bool hasKey(const std::string& key);

    /**
     * Sets the <code>value</code> for the <code>key</code>, generating all necessary
     * intermediate steps in the process. If a value already exists at the provided key,
     * the old value is silently overwritten.
     * \tparam T The type of the value that is to be stored. Has to be one of the types
     * as described in the class overview
     * \param key The key under which the <code>value</code> is to be stored
     * \param value The value which should be stored at the provided location
     */
    template <typename T>
    void setValue(const std::string& key, const T& value);

    /**
     * Sets the <code>value</code> for the <code>key</code>, generating all necessary
     * intermediate steps in the process. If a value already exists at the provided key,
     * the old value is silently overwritten. This is a specialized variant from 
     * #setValue to deal with constant strings
     * \param key The key under which the <code>value</code> is to be stored
     * \param value The value which should be stored at the provided location
     */
    void setValue(const std::string& key, const char* value);

    /**
     * Gets the <code>value</code> for the provided <code>key</code>. The value at the
     * location has to agree with the type of the template parameter <code>T</code>. If
     * it is not possible to convert the stored type to the requested type, the value is
     * unchanged and <code>false</code> is returned. The method also returns
     * <code>false</code>, if the <code>key</code> could not be found in the
     * configuration. If the value could be retrieved successfully, <code>true</code> is
     * returned and the value is modified.
     * \tparam T The type of the value that is requested. Has to be one of the types as
     * described in the class overview
     * \param key The key of the <code>value</code> that is to be retrieved
     * \param value The reference which will contain the requested value if the retrieval
     * was successful
     * \return <code>true</code> if the value was retrieved successfully and could be
     * casted into the requested type. <code>false</code> otherwise.
     */
    template <typename T>
    bool getValue(const std::string& key, T& value);

private:
    /// Stores the Lua state including the main script and the table containing the config
    lua_State* _state;
};

} // namespace ghoul

#include "configurationmanager.inl"

#endif // __CONFIGURATIONMANAGER_H__
