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

#ifndef __GHOUL___TEXTUREMANAGER___H__
#define __GHOUL___TEXTUREMANAGER___H__

#include <ghoul/misc/exception.h>
#include <map>
#include <memory>
#include <string>

namespace ghoul::opengl {

class Texture;

/**
 * This singleton class provides a central and efficient storage for Texture objects. The
 * Texture%s stored in this class belong to the TextureManager and should be deleted using
 * #unregisterTexture which returns the stored Texture. If the calling function does not
 * pick up the returned value, it is then destroyed. Texture%s can be registered
 * (#registerTexture), unregistered (#unregisterTexture), or retrieved (#texture) using
 * either an string name or a generated hash value which is more efficient to
 * retrieve than a string.
 */
class TextureManager {
public:
    /// Main exception that is thrown in the methods of the TextureManager
    struct TextureManagerError : public RuntimeError {
        explicit TextureManagerError(std::string msg);
    };

    /**
     * This method returns a reference to the initialized TextureManager.
     *
     * \return An initialized reference to the singleton manager
     *
     * \pre The static TextureManager must have been initialized before
     */
    static TextureManager& ref();

    /**
     * This method will return the Texture that was registered with a string whose hash
     * value is equal to \p hashedName. The hashed name that can be used will be returned
     * from the #registerTexture method.
     *
     * \param hashedName The hash of the Texture that is to be fetched
     * \return The Texture that has been registered with the passed name
     *
     * \throw TextureManagerError If the ShaderObject for \p name did not exist
     */
    Texture* texture(unsigned int hashedName);

    /**
     * This method will return the Texture that was registered with the passed \p name.
     * This method will create the hash value from the passed string and will call the
     * #texture method.
     *
     * \param name The name of the Texture that is to be fetched
     * \return The Texture that has been registered with the passed name
     *
     * \throw TextureManagerError If the ShaderObject for \p name did not exist
     */
    Texture* texture(const std::string& name);

    /**
     * Register the passed Texture under the passed name so that it can be retrieved
     * either by the given \p name or by the returned hashed value. If the manager already
     * contains a hashed value equal to the hash value of hte name, an exception is
     * thrown.
     *
     * \param name The name under which the Texture should be registered
     * \param texture The Texture that should be registered
     * \return The hashed value that is generated from the \p name
     *
     * \pre \p texture must not be nullptr
     */
    unsigned int registerTexture(const std::string& name,
        std::unique_ptr<Texture> texture);

    /**
     * This method will unregister the Texture that was registered under the passed
     * \p name. The Texture will be returned to the caller. If the caller ignores the
     * return value, the Texture will be deleted.
     *
     * \param name The name under which the Texture was registered previously
     * \return The previously registered Texture or <code>nullptr</code> if the \p name
     *         was not a valid Texture
     */
    std::unique_ptr<Texture> unregisterTexture(const std::string& name);

    /**
     * This method will unregister the Texture that was registered under a name whose hash
     * value is equal to the passed \p hashName. The Texture will be returned to the
     * caller. If the caller ignores the return value, the Texture will be deleted.
     *
     * \param hashedName The hash value which is used to refer to a previously stored
     *        Texture
     * \return The previously registered Texture or <code>nullptr</code> if the \p name
     *         was not a valid Texture
     */
    std::unique_ptr<Texture> unregisterTexture(unsigned int hashedName);

    /**
     * This method returns the hash value for a given string. The hash function is
     * implementation detail, but it is guaranteed to be static in an application run and
     * it will always produce reliable, consistent results.
     *
     * \param name The name which should be converted into a hash value
     * \return The hash value for the passed name
     */
    unsigned int hashedNameForName(const std::string& name) const;

private:
    /// The singleton member
    static TextureManager* _manager;

    /// Map containing all the registered Texture%s
    std::map<unsigned int, std::unique_ptr<Texture>> _textures;
};

#define TexMgr (ghoul::opengl::TextureManager::ref())

} // namespace ghoul::opengl

#endif // __GHOUL___TEXTUREMANAGER___H__
