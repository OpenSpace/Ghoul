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

#ifndef __GHOUL___SHADERMANAGER___H__
#define __GHOUL___SHADERMANAGER___H__

#include <ghoul/misc/exception.h>
#include <map>
#include <memory>
#include <string>

namespace ghoul::opengl {

class ShaderObject;

/**
 * This singleton class provides a central and efficient storage for ShaderObject%s. The
 * shaders stored in this class belong to the ShaderManager and should be deleted using
 * #unregisterShaderObject which returns the stored ShaderObject. If the calling function
 * does not pick up the returned value, it is then destroyed. Shader%s can be registered
 * (#registerShaderObject), unregistered (#unregisterShaderObject), or retrieved
 * (#shaderObject) using either a string name or a generated hash value which is more
 * efficient to retrieve than a string.
 */
class ShaderManager {
public:
    /// Main exception that is thrown in the methods of the ShaderManager
    struct ShaderManagerError : public RuntimeError {
        explicit ShaderManagerError(std::string msg);
    };

    /**
     * This method returns a reference to the initialized ShaderManager.
     *
     * \return An initialized reference to the singleton manager
     * \pre The static ShaderManager must have been initialized before
     */
    static ShaderManager& ref();

    /**
     * This method will return the ShaderObject that was registered with a string whose
     * hash value is equal to \p hashedName. The hashed name that can be used will be
     * returned from the #registerShaderObject method.
     *
     * \param hashedName The hashed name of the ShaderObject that is to be fetched
     * \return The ShaderObject that has been registered with a string that evaluates
     *         to the \p hashedName
     *
     * \throw ShaderManagerError if the ShaderObject for \p hashedName did not exist
     */
    ShaderObject* shaderObject(unsigned int hashedName);

    /**
     * This method will return the ShaderObject that was registered with the passed name.
     * This method will create the hash value from the passed string and will call the
     * #shaderObject method.
     *
     * \param name The name of the ShaderObject that is to be fetched
     * \return The ShaderObject that has been registered with the passed name
     *
     * \throw ShaderManagerError If the ShaderObject for \p name did not exist
     */
    ShaderObject* shaderObject(const std::string& name);

    /**
     * Register the passed ShaderObject under the passed name so that it can be retrieved
     * either by the given \p name or by the returned hashed value. If the manager already
     * contains a hashed value equal to the hash value of the name, an exception is
     * thrown.
     *
     * \param name The name under which the ShaderObject should be registered
     * \param shader The ShaderObject that should be registered
     * \return The hashed value that is generated from the \p name
     *
     * \throw ShaderManagerError If there already was a ShaderObject for the provided
     *        \p name
     * \pre \p shader must not be nullptr
     */
    unsigned int registerShaderObject(const std::string& name,
        std::unique_ptr<ShaderObject> shader);

    /**
     * This method will unregister the ShaderObject that was registered under the passed
     * name. The ShaderObject will be returned to the caller. If the caller ignores the
     * return value, the ShaderObject will be deleted.
     *
     * \param name The name under which the ShaderObject was registered previously
     * \return The previously registered ShaderObject or <code>nullptr</code> if the
     *         \p name was not a valid ShaderObject
     */
    std::unique_ptr<ShaderObject> unregisterShaderObject(const std::string& name);

    /**
     * This method will unregister the ShaderObject that was registered under a name whose
     * hash value is equal to the passed \p hashName. The ShaderObject will be returned to
     * the caller. If the caller ignores the return value, the ShaderObject will be
     * deleted.
     *
     * \param hashedName The hash value which is used to refer to a previously stored
     *        ShaderObject
     * \return The registered ShaderObject or <code>nullptr</code> if the \p name was not
     *         a valid ShaderObject
     */
    std::unique_ptr<ShaderObject> unregisterShaderObject(unsigned int hashedName);

    /**
     * This method returns the hash value for a given \p name. The hash function is
     * implementation-dependent, but it is guaranteed to be static in an application run
     * and it will produce reliable, consistent results.
     *
     * \param name The name which should be converted into a hash value
     * \return The hash value for the passed name
     */
    unsigned int hashedNameForName(const std::string& name) const;

private:
    /// Map containing all the registered ShaderObject%s
    std::map<unsigned int, std::unique_ptr<ShaderObject>> _objects;
};

#define ShdrMgr (ghoul::opengl::ShaderManager::ref())

} // namespace ghoul::opengl

#endif // __GHOUL___SHADERMANAGER___H__
