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

#ifndef __GHOUL___TEXTUREUNIT___H__
#define __GHOUL___TEXTUREUNIT___H__

#include <ghoul/misc/exception.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <string>
#include <vector>

namespace ghoul::opengl {

/**
 * This class manages Texture Units and is a wrapper around <code>GL_TEXTURE0</code>,
 * <code>GL_TEXTURE1</code>, ... It manages which texture units are currently active and
 * which units are free to use. To use a TextureUnit, it has to be #activate%d, the
 * Texture has to be bound, and then the #unitNumber can be retrieved so that it can be
 * used in a uniform. A unit number is assigned as soon as the first call to #activate,
 * #glEnum, or #unitNumber is made. If there are no free unit numbers left, an
 * <code>std::runtime_error</code> will be thrown.
 */
class TextureUnit {
public:
    /// Main exception that is thrown if a new TextureUnit could not be assigned
    struct TextureUnitError : public RuntimeError {
        explicit TextureUnitError(std::string msg);
    };

    /**
     * The constructor will initialize the static variables when the first TextureUnit is
     * created and the non-static variables are initiated.
     */
    TextureUnit();

    /**
     * The destructor will free the used texture unit and mark it as free again.
     */
    ~TextureUnit();

    /**
     * This method will activate the enum assigned to this TextureUnit. If this is the
     * first call to either activate, #glEnum, or #unitNumber, a free unit number will be
     * assigned to this object. This will call the OpenGL function
     * <code>glActiveTexture</code> with the enum as a parameter.
     *
     * \throw TextureUnitError If the new unit number could not be assigned
     */
    void activate();

    /**
    * This method will deactivate the enum assigned to this TextureUnit and mark it as
    * free again.
    */
    void deactivate();

    /**
     * This method returns the texture unit enum that was assigned to this TextureUnit. If
     * this is the first call to either #activate, glEnum, or #unitNumber, a free unit
     * enum will be assigned to this object. The returned enum is equivalent to
     * <code>GL_TEXTURE0 + unitNumber()</code>.
     *
     * \return The unit enum that was assigned to this TextureUnit
     */
    GLenum glEnum();

    /**
     * This method returns the texture unit number that was assigned to this TextureUnit.
     * If this is the first call to either #activate, #glEnum, or unitNumber, a free unit
     * number will be assigned to this object.
     *
     * \return The texture unit number that was assigned to this TextureUnit
     *
     * \throw TextureUnitError If the new unit number could not be assigned
     */
    GLint unitNumber();

    /**
     * This operator returns the texture unit number that was assigned to this
     * TextureUnit. If this is the first call to either #activate, #glEnum, or unitNumber,
     * a free unit number will be assigned to this object. Is guaranteed to give the same
     * result as #unitNumber.
     *
     * \return The texture unit number that was assigned to this TextureUnit
     *
     * \throw TextureUnitError If the new unit number could not be assigned
     */
    operator GLint();

    /**
     * Sets the zero unit as the active texture unit. This call is equivalent to calling
     * the OpenGL function <code>glActiveTexture(GL_TEXTURE0)</code>.
     */
    static void setZeroUnit();

    /**
     * This method returns the number texture units that have been marked as used by
     * calling one of the methods #activate, #glEnum, or #unitNumber. The maximum number
     * of supported texture units can be queried from the
     * #ghoul::systemcapabilities::SystemCapabilities.
     *
     * \return The number of texture units in use
     */
    static int numberActiveUnits();

private:
    /**
     * This method is called the first time either #activate, #glEnum, or #unitNumber is
     * called. It will assign a new OpenGL texture unit to this TextureUnit and mark this
     * new unit as used until this TextureUnit is destroyed.
     *
     * \throw TextureUnitError If the new unit number could not be assigned
     */
    void assignUnit();

    /**
     * Initializes the maximum number of texture units using
     * #ghoul::systemcapabilities::SystemCapabilities and marks all texture units as
     * unused. This method is called the first time a TextureUnit is created.
     */
    static void initialize();

    /// The number in <code>[0, maxTexUnits]</code> referring to this TextureUnit
    GLint _number;

    /**
     * The enum in <code>[GL_TEXTURE0, GL_TEXTURE0 + maxTexUnits]</code> referring to this
     * TextureUnit
     */
    GLenum _glEnum;

    /// <code>true</code> if this TextureUnit has been assigned
    bool _assigned;

    /// <code>true</code> if the list of busy units and the maximum number of units have
    /// been initialized
    static bool _isInitialized;

    /// The total number of active texture unit
    static unsigned int _totalActive;

    /// The maximum number of texture units
    static unsigned int _maxTexUnits;

    /**
     * This vector stores a bool at position <code>i</code> if the texture unit number
     * <code>i</code> is currently in use
     */
    static std::vector<bool> _busyUnits;
};

} // namespace ghoul::opengl

#endif // __GHOUL___TEXTUREUNIT___H__
