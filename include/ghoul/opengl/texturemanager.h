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

#ifndef __TEXTUREMANAGER_H__
#define __TEXTUREMANAGER_H__

#include <map>
#include <string>

namespace ghoul {
namespace opengl {

class Texture;

/**
 * This singleton class provides a central and efficient storage for Texture objects. The
 * Texture%s stored in this class belong to the TextureManager and should be deleted using
 * #unregisterTexture or their ownership taken be away from the TextureManager by using
 * #forgetTexture. Texture%s can be registered (#registerTexture), unregistered
 * (#unregisterTexture), forgotten (#forgetTexture), or retrieved (#texture) using either
 * an <code>std::string</code> name, or a generated hash value which is more efficient to
 * retrieve than a string.
 */
class TextureManager {
public:
    /**
     * Static initializer that initializes the static member. This needs to be done before
     * the TextureManager can be used. If the manager has been already initialized, this
     * results in a no op.
     */
    static void initialize();

    /**
     * Static deinitializer that will remove all of the registered textures, that haven't 
     * been either forgotten (#forgetTexture) or unregistered (#unregisterTexture, which
     * will instead delete the texture). The manager will be deleted and will be 
     * unavailable until it is re-initialized (#initialize).
     */
    static void deinitialize();

    /**
     * This method returns a reference to the initialized TextureManager. The manager has
     * to be initialized before it can be used, or an assertion will be triggered.
     * \return An initialized reference to the singleton manager
     */
    static TextureManager& ref();

    /**
     * This method will return the Texture that was registered with the passed
     * <code>hashedName</code>.
     * \param hashedName The hash of the Texture that is to be fetched
     * \return The Texture that has been registered with the passed name
     */

    Texture* texture(unsigned int hashedName);

    /**
     * This method will return the Texture that was registered with the passed
     * <code>name</code>. This method will create the hash value from the passed string
     * and will call the #texture method.
     * \param name The name of the Texture that is to be fetched
     * \return The Texture that has been registered with the passed name
     */
    Texture* texture(const std::string& name);

    /**
     * Register the passed Texture under the passed name so that it can be retrieved
     * either by the given <code>name</code> or by the hashed value that is equal to the
     * hash value of the passed name. The hashed value can later be retrieved by the
     * function #hashedNameForName. This method will transfer the ownership of the Texture
     * to the TextureManager. If the manager already contains a hashed value equal to the 
     * hash value of the name, a warning is logged and <code>false</code> is returned.
     * \param name The name under which the Texture should be registered
     * \param texture The Texture that should be registered
     * \return <code>true</code> if the Texture was registered successfully in the 
     * TextureManager, <code>false</code> if a texture with the specified name was already 
     * present in the manager
     */
    bool registerTexture(const std::string& name, Texture* texture);

    /**
     * Register the passed Texture under the passed name so that it can be retrieved
     * either by the given <code>name</code> or by the hashed value is returned in the
     * third parameter of this function. The hashed value can later be retrieved by the
     * function #hashedNameForName as well. This method will transfer the ownership of the
     * Texture to the TextureManager. If the manager already contains a hashed value equal
     * to the hash value of the name, a warning is logged and <code>false</code> is
     * returned.
     * \param name The name under which the Texture should be registered
     * \param texture The Texture that should be registered
     * \param hashedName The hashed value that is generated from the <code>name</code>
     * \return <code>true</code> if the Texture was registered successfully in the 
     * TextureManager, <code>false</code> if a texture with the specified name was already
     * present in the manager
     */
    bool registerTexture(const std::string& name,
                         Texture* texture, unsigned int& hashedName);

    /**
     * This method will unregister the Texture that was registered under the passed
     * <code>name</code>. The Texture will be removed from the manager and deleted.
     * \param name The name under which the Texture was registered previously
     */
    void unregisterTexture(const std::string& name);

    /**
     * This method will unregister the Texture that was registered under a name whose hash 
     * value is equal to the passed <code>hashName</code>. The Texture will be removed
     * from the manager and deleted.
     * \param hashedName The hash value which is used to refer to a previously stored 
     * Texture
     */
    void unregisterTexture(unsigned int hashedName);

    /**
     * This method will unregister the Texture that was registered under the passed
     * <code>name</code>.
     * The Texture will be removed from the manager, but it will not be automatically 
     * deleted. This means that the ownership of the Texture is transferred to the caller 
     * of this function.
     * \param name The name under which the Texture was registered previously
     */
    void forgetTexture(const std::string& name);

    /**
     * This method will unregister the Texture that was registered under a name whose hash 
     * value is equal to the passed <code>hashName</code>. The Texture will be removed
     * from the manager, but it will not be automatically deleted. This means that the
     * ownership of the Texture is transferred to the caller of this function.
     * \param hashedName The hash value which is used to refer to a previously stored 
     * Texture
     */
    void forgetTexture(unsigned int hashedName);

    /**
     * This method returns the hash value for a given string. The hash function is 
     * implementation detail, but it is guaranteed to be static in an application run and 
     * it will always produce reliable, consistent results.
     * \param name The name which should be converted into a hash value
     * \return The hash value for the passed name
     */
    unsigned int hashedNameForName(const std::string& name) const;

private:
    TextureManager();

    /// Not implemented on purpose, using this function should generate an error
    TextureManager(const TextureManager& c);
    
    ~TextureManager();

    static TextureManager* _manager; ///< singleton member
    std::map<unsigned int, Texture*> _textures;
};

#define TexMgr (ghoul::opengl::TextureManager::ref())

} // namespace opengl
} // namespace ghoul

#endif
