/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#ifndef __SHADERMANAGER_H__
#define __SHADERMANAGER_H__

#include <map>
#include <string>

namespace ghoul {
namespace opengl {

class ShaderObject;

/**
 * This singleton class provides a central and efficient storage for \see ShaderObject s.
 * The shaders stored in this class belong to the ShaderManager and should be deleted
 * using \see unregisterShaderObject or their ownership be taken away from the 
 * ShaderManager by using \see forgetShaderObject.
 * Shaders can be registered (\see registerShaderObject), unregistered
 * (\see unregisterShaderObject), forgotten (\see forgetShaderObject), or retrieved 
 * (\see shaderObject) using either a string name, or a generated hash value which is more
 * efficient to retrieve than a string.
 */
class ShaderManager {
public:
    /**
     * Static initializer that initializes the static member. This needs to be done before
     * the ShaderManager can be used. If the manager has been already initialized, this
     * results in a no op.
     */
    static void initialize();
    
    /**
     * Static deinitializer that will remove all of the registered ShaderObjects, that 
     * haven't been either forgotten (\see forgetShaderObject) or unregistered (\see 
     * unregisterShaderObject, which will instead delete the shader). The manager will be 
     * deleted and will be unavailable until it is re-initialized (\see initialize).
     */
    static void deinitialize();
    
    /**
     * This method returns a reference to the initialized ShaderManager. If the manager
     * has not been initialized, it will be done so in the first call to this method.
     * \return An initialized reference to the singleton manager
     */
    static ShaderManager& ref();
    
    /**
     * This method will return the \see ShaderObject that was registered with a string
     * whose hash value is equal to <code>hashedName</code>. The hashed name that can be 
     * used will be returned from the \see registerShaderObject method.
     * \param hashedName The hashed name of the ShaderObject that is to be fetched
     * \return The \see ShaderObject that has been registered with a string that evaluates 
     * to the <code>hashedName</code>
     */
    ShaderObject* shaderObject(unsigned int hashedName);
    
    /**
     * This method will return the \see ShaderObject that was registered with the passed 
     * name. This method will create the hash value from the passed string and will call 
     * the \see shaderObject method.
     * \param name The name of the ShaderObject that is to be fetched
     * \return The \see ShaderObject that has been registered with the passed name
     */
    ShaderObject* shaderObject(const std::string& name);
    
    /**
     * Register the passed \see ShaderObject under the passed name so that it can be 
     * retrieved either by the given name or by the hashed value that is equal to the hash 
     * value of the passed name. The hashed value can later be retrieved by the function
     * \see hashedNameForName. This method will transfer the ownership of the ShaderObject 
     * to the ShaderManager. If the manager already contains a hashed value equal to the
     * hash value of the name, a warning is logged and <code>false</code> is returned.
     * \param name The name under which the ShaderObject should be registered
     * \param shader The ShaderObject that should be registered
     * \return <code>true</code> if the ShaderObject was registered successfully in the
     * ShaderManager, <code>false</code> if a ShaderObject with the specified name was
     * already present in the manager
     */
    bool registerShaderObject(const std::string& name, ShaderObject* shader);
    
    /**
     * Register the passed \see ShaderObject under the passed name so that it can be 
     * retrieved either by the given name or by the hashed value is returned in the third 
     * parameter of this function. The hashed value can later be retrieved by the function
     * \see hashedNameForName as well. This method will transfer the ownership of the
     * ShaderObject to the ShadereManager. If the manager already contains a hashed value 
     * equal to the hash value of the name, a warning is logged and <code>false</code> is
     * returned.
     * \param name The name under which the ShaderObject should be registered
     * \param shader The ShaderObject that should be registered
     * \param hashedName The hashed value that is generated from the <code>name</code>
     * \return <code>true</code> if the ShaderObject was registered successfully in the
     * ShaderManager, <code>false</code> if a ShaderObject with the specified name was 
     * already present in the manager
     */
    bool registerShaderObject(const std::string& name,
                              ShaderObject* shader, unsigned int& hashedName);
    
    /**
     * This method will unregister the ShaderObject that was registered under the passed 
     * name. The ShaderObject will be removed from the manager and deleted.
     * \param name The name under which the ShaderObject was registered previously
     */
    void unregisterShaderObject(const std::string& name);
    
    /**
     * This method will unregister the ShaderObject that was registered under a name whose
     * hash value is equal to the passed hashName. The ShaderObject will be removed from 
     * the manager and deleted.
     * \param hashedName The hash value which is used to refer to a previously stored
     * ShaderObject
     */
    void unregisterShaderObject(unsigned int hashedName);
    
    /**
     * This method will unregister the ShaderObject that was registered under the passed 
     * name. The ShaderObject will be removed from the manager, but it will not be 
     * automatically deleted. This means that the ownership of the ShaderObject is 
     * transferred to the caller of this function.
     * \param name The name under which the ShaderObject was registered previously
     */
    void forgetShaderObject(const std::string& name);

    /**
     * This method will unregister the ShaderObject that was registered under a name whose 
     * hash value is equal to the passed hashName. The ShaderObject will be removed from 
     * the manager, but it will not be automatically deleted. This means that the 
     * ownership of the ShaderObject is transferred to the caller of this function.
     * \param hashedName The hash value which is used to refer to a previously stored
     * ShaderObject
     */
    void forgetShaderObject(unsigned int hashedName);
    
    /**
     * This method returns the hash value for a given string. The hash function is
     * implementation detail, but it is guaranteed to be static in an application run and
     * it will always produce reliable, consistent results.
     * \param name The name which should be converted into a hash value
     * \return The hash value for the passed name
     */
    unsigned int hashedNameForName(const std::string& name) const;
    
    
private:
    ShaderManager();
    ShaderManager(const ShaderManager& c);
    ~ShaderManager();
    
    static ShaderManager* _manager; ///< singleton member
    std::map<unsigned int, ShaderObject*> _objects;
};
    
#define ShdrMgr (ghoul::opengl::ShaderManager::ref())

} // namespace opengl
} // namespace ghoul

#endif // __SHADERMANGER_H__