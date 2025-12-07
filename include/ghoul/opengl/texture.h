/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2025                                                               *
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

#ifndef __GHOUL___TEXTURE___H__
#define __GHOUL___TEXTURE___H__

#include <ghoul/glm.h>
#include <ghoul/misc/boolean.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <array>
#include <string>

//#define Debugging_Ghoul_Textures_Indices

namespace ghoul::opengl {

/**
 * This class is a wrapper for an OpenGL texture. It wraps the OpenGL method for
 * generating (`glGenTextures`), binding (`glBindTexture`, #bind), enabling (`glEnable`,
 * #enable) and others. A new texture can be created either from an image file using the
 * TextureManager or loadTexture function or using a pointer to the data. The data layout
 * must be as specified by the `glTexImageXD` functions (see OpenGL reference). After
 * creation a Texture must be uploaded (uploadTexture) before it can be used in
 * the graphics pipeline. See http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml
 * for detailed information on the different arguments
 */
class Texture {
public:
    BooleanType(AllocateData);
    BooleanType(TakeOwnership);

    /**
     * This enum specifies the allowed formats for the Texture%s. These are directly
     * mapped to the appropriate OpenGL constants.
     *
     * \see http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml
     */
    enum class Format : std::underlying_type_t<GLenum> {
        Red = static_cast<std::underlying_type_t<GLenum>>(GL_RED),
        RG = static_cast<std::underlying_type_t<GLenum>>(GL_RG),
        RGB = static_cast<std::underlying_type_t<GLenum>>(GL_RGB),
        BGR = static_cast<std::underlying_type_t<GLenum>>(GL_BGR),
        RGBA = static_cast<std::underlying_type_t<GLenum>>(GL_RGBA),
        BGRA = static_cast<std::underlying_type_t<GLenum>>(GL_BGRA),
        DepthComponent = static_cast<std::underlying_type_t<GLenum>>(GL_DEPTH_COMPONENT)
    };

    /**
     * This enum specifies the filtering method this texture will use to interpolate
     * between two texel. The values for this enum correspond directly to OpenGL settings.
     * See the OpenGL specification for details.
     */
    enum class FilterMode {
        Nearest, ///< GL_NEAREST
        Linear, ///< GL_LINEAR
        LinearMipMap, ///< GL_LINEAR_MIPMAP_LINEAR
        AnisotropicMipMap
    };

    /**
     * This enum specifies the wrapping mode this texture will use at the edges of the
     * texture. The values for this enum correspond directly to OpenGL settings. See the
     * OpenGL specification for details.
     */
    enum class WrappingMode : std::underlying_type_t<GLenum> {
        Repeat = static_cast<std::underlying_type_t<GLenum>>(GL_REPEAT),
        Clamp = static_cast<std::underlying_type_t<GLenum>>(GL_CLAMP),
        ClampToEdge = static_cast<std::underlying_type_t<GLenum>>(GL_CLAMP_TO_EDGE),
        ClampToBorder = static_cast<std::underlying_type_t<GLenum>>(GL_CLAMP_TO_BORDER),
        MirroredRepeat = static_cast<std::underlying_type_t<GLenum>>(GL_MIRRORED_REPEAT)
    };

    /**
     * Encapsulating the wrapping mode state for 1D, 2D, and 3D textures.  1D textures
     * only use `s`, 2D textures use `s` and `t`, where as 3D textures use all three
     * specified wrapping modes.
     */
    struct WrappingModes {
        WrappingMode s;
        WrappingMode t = s;
        WrappingMode r = t;
    };

    static int numberOfChannels(Format format);

    /**
     * This constructor will create storage internally to fit the amount of data that is
     * necessary for the \p dimensions `* bytesPerPixel` (which is in turn
     * dependent on the \p dataType). The Texture can be 1D, 2D, or 3D depending on how
     * many components are equal to `1`.
     *
     * \param dimensions The dimensions of the texture. A 3D texture will be created
     *        if all components are bigger than `1`, a 2D texture will be created if the
     *        `z` component is equal to `1`, while a 1D texture is created if the `y` and
     *        `z` component is equal to `1`
     * \param type The type of the texture. Must be one of GL_TEXTURE_1D, GL_TEXTURE_2D,
     *        or GL_TEXTURE_3D
     * \param format Specifies the format of the data
     * \param internalFormat The internal format for the texture. See
     *        http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml Tables 1, 2, and 3
     *        for concrete values. In addition, the S3TC_DXT formats can be used to
     *        support hardware compression. See
     *        http://www.opengl.org/wiki/Image_Format#S3TC.2FDXT for more information
     * \param dataType The data type of the pixel data. See
     *        http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml for a list of
     *        possible values
     * \param filter The Texture::FilterMode that will be used to interpolate between
     *        texels
     * \param wrapping The Texture::WrappingMode that will be used to generate values on
     *        the border of the texture
     * \param allocate Sets whether or not the texture object should allocate texture
     *        data itself
     * \param takeOwnership Sets whether or not the Texture object should take ownership
     *        of the data allocated. In case there is no data allocated, the Texture will
     *        automatically not take ownership
     *
     * \pre Element of \p dimensions must be bigger or equal `1`
     */
    Texture(glm::uvec3 dimensions, GLenum type, Format format = Format::RGBA,
        GLenum internalFormat = GL_RGBA, GLenum dataType = GL_UNSIGNED_BYTE,
        FilterMode filter = FilterMode::Linear,
        WrappingMode wrapping = WrappingMode::Repeat,
        AllocateData allocate = AllocateData::Yes,
        TakeOwnership takeOwnership = TakeOwnership::Yes);

    /**
     * This constructor will generate a Texture out of the passed data. The data should
     * contain enough bytes to fill `dimensions * bytesPerPixel` (which is in
     * turn dependent on the dataType) pixel. The Texture can be 1D, 2D, or 3D depending
     * on how many components are equal to `1`.
     * \param data The data from which to generate the Texture. The data must be in a
     * linear format and (in 2D and 3D cases) aligned so that it can be accessed using the
     * following equations: `(y * dimensions.x) + x` in the 2D case and
     * `(z * dimensions.x * dimensions.y) + (y * dimensions.x) + x` in the 3D case. This
     * Texture will take ownership of the data array and will delete it once this object
     * is destroyed.
     *
     * \param dimensions The dimensions of the texture. A 3D texture will be created if
     *        all components are bigger than `1`, a 2D texture will be created if the `z`
     *        component is equal to `1`, while a 1D texture is created if the `y` and `z`
     *        component is equal to `1`
     * \param type The type of the texture. Must be one of GL_TEXTURE_1D, GL_TEXTURE_2D,
     *        or GL_TEXTURE_3D
     * \param format Specifies the format of the data
     * \param internalFormat The internal format for the texture. See
     *        http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml Tables 1,2, and 3
     *        for concrete values. In addition, the S3TC_DXT formats can be used to
     *        support hardware compression.
     *        See http://www.opengl.org/wiki/Image_Format#S3TC.2FDXT for more information
     * \param dataType The data type of the pixel data. See
     *        http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml for a list of
     *        possible values
     * \param filter The Texture::FilterMode that will be used to interpolate between
     *        texels
     * \param wrapping The Texture::WrappingMode that will be used to generate values on
     *        the border of the texture
     * \param pixelAlignment The byte-alignment for each of the pixels in the provided
     *        \p data array
     */
    Texture(void* data, glm::uvec3 dimensions, GLenum type, Format format = Format::RGBA,
        GLenum internalFormat = GL_RGBA, GLenum dataType = GL_UNSIGNED_BYTE,
        FilterMode filter = FilterMode::Linear,
        WrappingMode wrapping = WrappingMode::Repeat,
        int pixelAlignment = 1);

    /**
     * Unloads the Texture from GPU memory and destroys the id. The destructor will also
     * remove the data associated with this texture.
     */
    ~Texture();

    /**
     * Enables this texture type by calling `glEnable`.
     */
    void enable() const;

    /**
     * Disables this texture type by calling `glDisable`.
     */
    void disable() const;

    /**
     * Binds this texture to the texture type by calling `glBindTexture`.
     */
    void bind() const;

    /**
     * Returns the OpenGL name of this texture.
     */
    operator GLuint() const;

    /**
     * Returns an (optional) name for this Texture. The name is not used internally and is
     * solely for external purposes. One possible use is the filename from which the
     * texture was loaded.
     *
     * \return The name for this texture
     */
    const std::string& name() const;

    /**
     * Sets an (optional) name for this texture. The name is not used internally and is
     * solely for external purposes. One possible use is the filename from which the
     * texture was loaded.
     *
     * \param name The name for this texture
     */
    void setName(std::string name);

    /**
     * Returns the type for this texture.
     *
     * \return The type for this texture. This value can either be `GL_TEXTURE_1D`,
     *         `GL_TEXTURE_2D` or `GL_TEXTURE_3D` depending on the dimension of the stored
     *         texture.
     */
    GLenum type() const;

    /**
     * Overrides the type of the texture that was automatically determined.
     *
     * \param type The new type for the texture
     *
     * \pre \p type must be `GL_TEXTURE_1D`, `GL_TEXTURE_2D` or `GL_TEXTURE_3D`
     */
    void setType(GLenum type);

    /**
     * Returns the dimensions of this texture. If the texture is a 2D texture, the `z`
     * component will be equal to `1` and if the texture is a 1D texture, the `y` and `z`
     * components will be equal to `1`.
     *
     * \return The dimensions of this texture
     */
    const glm::uvec3& dimensions() const;

    /**
     * Sets new dimensions for this texture. The texture is not automatically updated or
     * uploaded to the graphics card. Solely the stored dimensions are changed.
     *
     * \param dimensions The new dimensions for this texture
     */
    void setDimensions(glm::uvec3 dimensions);

    /**
     * Returns the width of the stored texture.
     */
    unsigned int width() const;

    /**
     * Returns the height of the stored texture or 1 if it is a 1D texture.
     */
    unsigned int height() const;

    /**
     * Returns the depth of the stored texture or 1 if it is a 1D or 2D texture.
     */
    unsigned int depth() const;

    /**
     * Returns the format for this texture.
     *
     * \return The format for this texture
     */
    Format format() const;

    /**
     * Sets the format for this texture. This only updates the internal state and doesn't
     * affect the texture on the graphics card. Call `uploadTexture` to apply changes.
     *
     * \param format The new format for this texture
     */
    void setFormat(Format format);

    /**
     * Returns the internal format for this texture. See
     * http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml for more information and
     * the possible return values.
     */
    GLenum internalFormat() const;

    /**
     * Sets the new internal format for this texture. This only updates the internal state
     * and doesn't affect the texture on the graphics card. Call #uploadTexture to apply
     * changes. See http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml for more
     * information and possible parameter values.
     *
     * \param internalFormat The new internal format. See
     *        http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml Tables 1, 2, and 3
     *        for possible values
     */
    void setInternalFormat(GLenum internalFormat);

    /**
     * Returns the Texture::FilterMode used by this texture.
     *
     * \return The Texture::FilterMode used by this texture
     */
    FilterMode filter() const;

    /**
     * Sets a new Texture::FilterMode for this Texture. The new Texture::FilterMode is
     * applied immediately. If the filter is FilterMode::AnisotropicMipMap, the texture
     * has to be uploaded before calling this method. Otherwise, the result is undefined.
     *
     * \param filter The new Texture::FilterMode for this Texture
     */
    void setFilter(FilterMode filter);

    /**
     * Sets a swizzle mask that is applied to this Texture object. Each element of the
     * `std::array` corresponds to one of the components, i.e., the first
     * index of \p swizzleMask is the red channel and so on. The symbolic constants that
     * are allowed in \p swizzleMask are: `GL_RED`, `GL_GREEN`, `GL_BLUE`, `GL_ALPHA`,
     * `GL_ONE`, and `GL_ZERO`. See
     * https://www.opengl.org/sdk/docs/man/html/glTexParameter.xhtml for more information.
     *
     * \param swizzleMask The swizzle mask that is applied to this Texture
     */
    void setSwizzleMask(std::array<GLenum, 4> swizzleMask);

    /**
     * Reinstates the default swizzle mask of `{ GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA }`
     */
    void setDefaultSwizzleMask();

    /**
     * Returns the currently used swizzle mask for this Texture.
     *
     * \return The currently used swizzle mask for this Texture
     */
    std::array<GLenum, 4> swizzleMask() const;

    /**
     * Returns the storage data type for this Texture. For a complete list of available
     * return values see http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml for
     * more information.
     *
     * \return The storage data type
     */
    GLenum dataType() const;

    /**
     * Sets a new storage data type for this Texture. For a complete list of available
     * parameters see http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml. The new
     * data type is only stored internally and not updated immediately, although it will
     * update the number of* bytes per pixel (see bytesPerPixel method).
     *
     * \param dataType The new storage data type
     */
    void setDataType(GLenum dataType);

    /**
     * Returns the number of channels that are stored in this texture. If the format of
     * the Texture is not in the list of formats found at
     * http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml, an assertion will be
     * triggered.
     *
     * \return The number of channels that are stored in this texture
     */
    int numberOfChannels() const;

    /*
     * Returns the number of bytes each pixel stores.
     *
     * \return The number of bytes each pixel stores
     */
    GLubyte bytesPerPixel() const;

    /**
     * Returns the stored data of the texture. If the memory is deleted, a new
     * (sufficiently) large memory block have been created or a segmentation fault might
     * occur.
     *
     * \return The stored data of the texture
     */
    const void* pixelData() const;

    /**
     * Returns the size the pixel data should have according to the dimensionality and the
     * bytes per pixel. `dimensions.x * dimensions.y * dimensions.z * bpp`. The
     * real size of the data can be different if it was set manually.
     *
     * \return The size of the pixel data according to the dimensionality and the bytes
     *         per pixel
     */
    int expectedPixelDataSize() const;

    /**
     * Sets new data for the texture to use. If the dimensions are not updated and the new
     * data has a different size, undefined behavior will occur. This Texture will take
     * ownership of the data array.
     *
     * \param pixels The pointer to the new data array that should be used.
     * \param takeOwnership Should this Texture take ownership of the data and delete it?
     * \param pixelAlignment The byte-alignment for each of the pixels in the provided
     *        \p pixels array
     */
    void setPixelData(void* pixels, TakeOwnership takeOwnership = TakeOwnership::Yes,
        int pixelAlignment = 1);

    /**
     * Returns `true` if the OpenGL texture is resident in the GPU memory.
     *
     * \return `true` if the OpenGL texture is resident
     */
    bool isResident() const;

    /**
     * Sets a new Texture::WrappingMode for this Texture. The new mode is applied
     * immediately.
     *
     * \param wrapping The new Texture::WrappingMode for this Texture
     */
    void setWrapping(WrappingMode wrapping);

    /** Sets a new Texture::WrappingModes for this Texture. The new mode is applied
     * immediately. This function supports setting different wrapping modes for different
     * dimensions.
     *
     * \param wrapping The new wrappingmodes for this Texture
     */
    void setWrapping(WrappingModes wrapping);

    /**
     * Returns the currently used Texture::WrappingMode for this texture.
     *
     * \return The currently used Texture::WrappingMode for this texture
     */
    WrappingModes wrapping() const;

    /**
     * Sets the maximum number of MipMap levels to use. This is only valid when the
     * FilterMode::AnisotropicMipMap is selected. Defaults to 8 levels.
     *
     * \param mipMapLevel The MipMap level that should be used in this texture.
     */
    void setMipMapLevel(int mipMapLevel);

    /**
     * Sets the maximum anisotropy level that should be used. This is only valid when the
     * FilterMode::AnisotropicMipMap is selected. On default, the maximum anisotropy
     * supported by the graphics card is used.
     *
     * \param anisotropyLevel The anisotropy level that should be used
     */
    void setAnisotropyLevel(float anisotropyLevel);

    /**
     * Binds and uploads the texture to graphics memory. The Texture has to be of type
     * `GL_TEXTURE_1D`, `GL_TEXTURE_2D`, or `GL_TEXTURE_3D`. The type will be determined
     * automatically based on the provided dimensions.
     */
    void uploadTexture();

    /**
     * Binds and uploads the texture to graphics memory. The Texture has to be of type
     * `GL_TEXTURE_1D`, `GL_TEXTURE_2D`, or `GL_TEXTURE_3D`. The type will be determined
     * automatically based on the provided dimensions. The function calls glTexSubImage
     * which means that the texture will already have to be existing in graphics memory.
     */
    void reUploadTexture();

    /**
     * Binds and uploads the texture to graphics memory using a pixel buffer object.
     * The Texture has to be of type `GL_TEXTURE_1D`, `GL_TEXTURE_2D`, or `GL_TEXTURE_3D`.
     * The type will be determined automatically based on the provided dimensions. The
     * data pointer will not be used. Intead it is assumed that the provided pixel buffer
     * object contains the data of the right size and format.
     */
    void uploadTextureFromPBO(GLuint pbo);

    /**
     * Binds and uploads the texture to graphics memory using a pixel buffer object.
     * The Texture has to be of type `GL_TEXTURE_1D`, `GL_TEXTURE_2D`, or `GL_TEXTURE_3D`.
     * The type will be determined automatically based on the provided dimensions. The
     * data pointer will not be used. Intead it is assumed that the provided pixel buffer
     * object contains the data of the right size and format. The function calls
     * glTexSubImage which means that the texture will already have to be existing in
     * graphics memory.
     */
    void reUploadTextureFromPBO(GLuint pbo);

    /**
     * If the texture is stored locally in RAM, the data representation will be removed.
     * If thre is no local representation, this function is a no-op.
     */
    void purgeFromRAM();

    /**
     * Downloads the texture from graphics memory and makes it available using the
     * #pixelData method.
     */
    void downloadTexture();

    /**
     * Changes (=grants/revokes) ownership of the stored data. Changing this will not
     * change the underlying data.
     *
     * \param hasOwnership `true` if this Texture should own the data in `_pixels`
     */
    void setDataOwnership(TakeOwnership hasOwnership);

    /**
     * Returns if this Texture owns its contained data.
     *
     * \return `true` if this Texture owns its contained data
     */
    bool dataOwnership() const;

    /**
     * Accesses the `i`-th texel in the data array. Returns a reference to it so that it
     * can be changed by the caller. Only Textures with a data type of the following list
     * can be used to fetch textures: `GL_UNSIGNED_BYTE`, `GL_BYTE`, `GL_UNSIGNED_SHORT`,
     * `GL_SHORT`, `GL_UNSIGNED_INT`, `GL_INT`, `GL_FLOAT`. Trying to use this function on
     * another type will lead to undefined behavior in the return value.
     *
     * \tparam T The type of the data that is returned
     * \param x The position of the texel that should be fetched
     * \return The texel at the specified position casted to the requested type T
     *
     * \pre The Texture must be a one dimensional Texture
     * \pre The size of T must be equal to the bytes per pixel stored in the Texture
     * \pre \p x must be within the width of the Texture
     */
    template <class T>
    T& texel(unsigned int x);

    /**
     * Accesses the `i`-th texel in the data array. Only Textures with a data type of the
     * following list can be used to fetch textures: `GL_UNSIGNED_BYTE`, `GL_BYTE`,
     * `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`, `GL_INT`, `GL_FLOAT`. Trying to
     * use this function on another type will lead to undefined behavior in the return
     * value.
     *
     * \tparam T The type of the data that is returned
     * \param x The position of the texel that should be fetched
     * \return The texel at the specified position casted to the requested type T
     *
     * \pre The Texture must be a one dimensional Texture
     * \pre The size of T must be equal to the bytes per pixel stored in the Texture
     * \pre \p x must be smaller than the width of the Texture
     */
    template <class T>
    const T& texel(unsigned int x) const;

    /**
     * Accesses the texel at `(x,y)` from the data array. Returns a reference to it so
     * that it can be changed by the caller. Only Textures with a data type of the
     * following list can be used to fetch textures: `GL_UNSIGNED_BYTE`, `GL_BYTE`,
     * `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`, `GL_INT`, `GL_FLOAT`. Trying to
     * use this function on another type will lead to undefined behavior in the return
     * value.
     *
     * \tparam T The type of the data that is returned
     * \param x The coordinate on the `width` axis
     * \param y The coordinate on the `height` axis
     * \return The texel at the specified position casted to the requested type T
     *
     * \pre The Texture must be a two dimensional Texture
     * \pre The size of T must be equal to the bytes per pixel stored in the Texture
     * \pre \p x must be smaller than the width of the Texture
     * \pre \p y must be smaller than the height of the Texture
     */
    template <class T>
    T& texel(unsigned int x, unsigned int y);

    /**
     * Accesses the texel at `(x,y)` from the data array. Only Textures with a data type
     * of the following list can be used to fetch textures: `GL_UNSIGNED_BYTE`, `GL_BYTE`,
     * `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`, `GL_INT`, `GL_FLOAT`. Trying to
     * use this function on another type will lead to undefined behavior in the return
     * value.
     *
     * \tparam T The type of the data that is returned
     * \param x The coordinate on the `width` axis
     * \param y The coordinate on the `height` axis
     * \return The texel at the specified position casted to the requested type T
     *
     * \pre The Texture must be a two dimensional Texture
     * \pre The size of T must be equal to the bytes per pixel stored in the Texture
     * \pre \p x must be smaller than the width of the Texture
     * \pre \p y must be smaller than the height of the Texture
     */
    template <class T>
    const T& texel(unsigned int x, unsigned int y) const;

    /**
     * Accesses the texel at `position` from the data array. Returns a reference to it so
     * that it can be changed by the caller. Only Textures with a data type of the
     * following list can be used to fetch textures: `GL_UNSIGNED_BYTE`, `GL_BYTE`,
     * `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`, `GL_INT`, `GL_FLOAT`. Trying to
     * use this function on another type will lead to undefined behavior in the return
     * value.
     *
     * \tparam T The type of the data that is returned
     * \param position The coordinate of the texel
     * \return The texel at the specified position casted to the requested type T
     *
     * \pre The Texture must be a two dimensional Texture
     * \pre The size of T must be equal to the bytes per pixel stored in the Texture
     * \pre `position.x` must be smaller than the width of the Texture
     * \pre `position.y` must be smaller than the height of the Texture
     */
    template <class T>
    T& texel(const glm::uvec2& position);

    /**
     * Accesses the texel at `position` from the data array. Only Textures with a data
     * type of the following list can be used to fetch textures: `GL_UNSIGNED_BYTE`,
     * `GL_BYTE`, `GL_UNSIGNED_SHORT`,`GL_SHORT`, `GL_UNSIGNED_INT`, `GL_INT`, `GL_FLOAT`.
     * Trying to use this function on another type will lead to undefined behavior in the
     * return value.
     *
     * \tparam T The type of the data that is returned
     * \param position The coordinate of the texel
     * \return The texel at the specified position casted to the requested type T
     *
     * \pre The Texture must be a two dimensional Texture
     * \pre The size of T must be equal to the bytes per pixel stored in the Texture
     * \pre `position.x` must be smaller than the width of the Texture
     * \pre `position.y` must be smaller than the height of the Texture
     */
    template <class T>
    const T& texel(const glm::uvec2& position) const;

    /**
     * Accesses the texel at `(x,y,z)` from the data array. Returns a reference to it so
     * that it can be changed by the caller. Only Textures with a data type of the
     * following list can be used to fetch textures: `GL_UNSIGNED_BYTE`, `GL_BYTE`,
     * `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`, `GL_INT`, `GL_FLOAT`. Trying to
     * use this function on another type will lead to undefined behavior in the return
     * value.
     *
     * \tparam T The type of the data that is returned
     * \param x The coordinate on the `width` axis
     * \param y The coordinate on the `height` axis
     * \param z The coordinate on the `depth` axis
     * \return The texel at the specified position casted to the requested type T
     *
     * \pre The Texture must be a three dimensional Texture
     * \pre The size of T must be equal to the bytes per pixel stored in the Texture
     * \pre \p x must be smaller than the width of the Texture
     * \pre \p y must be smaller than the height of the Texture
     * \pre \p z must be smaller than the depth of the Texture
     */
    template <class T>
    T& texel(unsigned int x, unsigned int y, unsigned int z);

    /**
     * Accesses the texel at `>(x,y,z)` from the data array. Only Textures with a data
     * type of the following list can be used to fetch textures: `GL_UNSIGNED_BYTE`,
     * `GL_BYTE`, `GL_UNSIGNED_SHORT` `GL_SHORT`, `GL_UNSIGNED_INT`, `GL_INT`, `GL_FLOAT`.
     * Trying to use this function on another type will lead to undefined behavior in the
     * return value.
     *
     * \tparam T The type of the data that is returned
     * \param x The coordinates for the `width` axis
     * \param y The coordinates for the `height` axis
     * \param z The coordinates for the `depth` axis
     * \return The texel at the specified position casted to the requested type T
     *
     * \pre The Texture must be a three dimensional Texture
     * \pre The size of T must be equal to the bytes per pixel stored in the Texture
     * \pre \p x must be smaller than the width of the Texture
     * \pre \p y must be smaller than the height of the Texture
     * \pre \p z must be smaller than the depth of the Texture
     */
    template <class T>
    const T& texel(unsigned int x, unsigned int y, unsigned int z) const;

    /**
     * Accesses the texel at `position` from the data array. Returns a
     * reference to it so that it can be changed by the caller. Only Textures with a data
     * type of the following list can be used to fetch textures: `GL_UNSIGNED_BYTE`,
     * `GL_BYTE`, `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`, `GL_INT`,
     * `GL_FLOAT`. Trying to use this function on another type will lead to undefined
     * behavior in the return value.
     *
     * \tparam T The type of the data that is returned
     * \param position The coordinate of the texel
     * \return The texel at the specified position casted to the requested type T
     *
     * \pre The Texture must be a three dimensional Texture
     * \pre The size of T must be equal to the bytes per pixel stored in the Texture
     * \pre `position.x` must be smaller than the width of the Texture
     * \pre `position.y` must be smaller than the height of the Texture
     * \pre `position.z` must be smaller than the height of the Texture
     */
    template <class T>
    T& texel(const glm::uvec3& position);

    /**
     * Accesses the texel at `position` from the data array. Only Textures with
     * a data type of the following list can be used to fetch textures:
     * `GL_UNSIGNED_BYTE`, `GL_BYTE`, `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`,
     * `GL_INT`, `GL_FLOAT`. Trying to use this function on another type will lead to
     * undefined behavior in the return value.
     * \tparam T The type of the data that is returned
     * \param position The coordinate of the texel
     * \return The texel at the specified position casted to the requested type T
     *
     * \pre The Texture must be a three dimensional Texture
     * \pre The size of T must be equal to the bytes per pixel stored in the Texture
     * \pre `position.x` must be smaller than the width of the Texture
     * \pre `position.y` must be smaller than the height of the Texture
     * \pre `position.z` must be smaller than the height of the Texture
     */
    template <class T>
    const T& texel(const glm::uvec3& position) const;

    /**
     * Returns the texel at the specified position as a `float` vector. Independent of the
     * underlying type, each component of the vector is in the range of `[0,1]`. Only
     * Texture%s with a data type of the following list can be used to fetch textures:
     * `GL_UNSIGNED_BYTE`, `GL_BYTE`, `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`,
     * `GL_INT`, `GL_FLOAT` . Trying to use this function on another type will lead to
     * undefined behavior in the return value.
     *
     * \param x The coordinate on the `width` axis
     * \return The texel at the specified position as a vector with each component in
     *         `[0,1]`
     *
     * \pre The Texture must be a one dimensional Texture
     * \pre \p x must be smaller than the width of the Texture
     */
    glm::vec4 texelAsFloat(unsigned int x) const;

    /**
     * Returns the texel at the specified position as a `float` vector. Independent of the
     * underlying type, each component of the vector is in the range of `[0,1]`. Only
     * Texture%s with a data type of the following list can be used to fetch textures:
     * `GL_UNSIGNED_BYTE`, `GL_BYTE`, `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`,
     * `GL_INT`, `GL_FLOAT`. Trying to use this function on another type will lead to
     * undefined behavior in the return value.
     *
     * \param x The coordinate on the `width` axis
     * \param y The coordinate on the `height` axis
     * \return The texel at the specified position as a vector with each component in
     *         `[0,1]`
     *
     * \pre The Texture must be a two dimensional Texture
     * \pre \p x must be smaller than the width of the Texture
     * \pre \p y must be smaller than the height of the Texture
     */
    glm::vec4 texelAsFloat(unsigned int x, unsigned int y) const;

    /**
     * Returns the texel at the specified position as a `float` vector. Independent of the
     * underlying type, each component of the vector is in the range of `[0,1]`. Only
     * Textures with a data type of the following list can be used to fetch textures:
     * `GL_UNSIGNED_BYTE`, `GL_BYTE`, `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`,
     * `GL_INT`, `GL_FLOAT`. Trying to use this function on another type will lead to
     * undefined behavior in the return value.
     *
     * \param pos The coordinate of the texel
     * \return The texel at the specified position as a vector with each component in
     *         `[0,1]`
     *
     * \pre The Texture must be a two dimensional Texture
     * \pre `pos.x` must be smaller than the width of the Texture
     * \pre `pos.y` must be smaller than the height of the Texture
     */
    glm::vec4 texelAsFloat(const glm::uvec2& pos) const;

    /**
     * Returns the texel at the specified position as a `float` vector. Independent of the
     * underlying type, each component of the vector is in the range of `[0,1]`. Only
     * Textures with a data type of the following list can be used to fetch textures:
     * `GL_UNSIGNED_BYTE`, `GL_BYTE`, `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`,
     * `GL_INT`, `GL_FLOAT`. Trying to use this function on another type will lead to
     * undefined behavior in the return value.
     *
     * \param x The coordinate on the `width` axis
     * \param y The coordinate on the `height` axis
     * \param z The coordinate on the `depth` axis
     * \return The texel at the specified position as a vector with each component in
     *         `[0,1]`
     *
     * \pre The Texture must be a three dimensional Texture
     * \pre \p x must be smaller than the width of the Texture
     * \pre \p y must be smaller than the height of the Texture
     * \pre \p z must be smaller than the depth of the Texture
     */
    glm::vec4 texelAsFloat(unsigned int x, unsigned int y, unsigned int z) const;

    /**
     * Returns the texel at the specified position as a `float` vector. Independent of the
     * underlying type, each component of the vector is in the range of `[0,1]`. Only
     * Texture%s with a data type of the following list can be used to fetch textures:
     * `GL_UNSIGNED_BYTE`, `GL_BYTE`, `GL_UNSIGNED_SHORT`, `GL_SHORT`, `GL_UNSIGNED_INT`,
     * `GL_INT`, `GL_FLOAT`. Trying to use this function on another type will lead to
     * undefined behavior in the return value.
     *
     * \param pos The coordinate of the texel
     * \return The texel at the specified position as a vector with each component in
     *         `[0,1]`
     *
     * \pre The Texture must be a three dimensional Texture
     * \pre `pos.x` must be smaller than the width of the Texture
     * \pre `pos.y` must be smaller than the height of the Texture
     * \pre `pos.z` must be smaller than the depth of the Texture
     */
    glm::vec4 texelAsFloat(const glm::uvec3& pos) const;

protected:
    /**
     * Initializes the Texture by determining the Texture type, the bytes per pixel,
     * generating an OpenGL name, allocating memory (depending on the passed parameter)
     * and applying the filter and wrapping modes
     */
    void initialize(bool allocateData);

    /**
     * Allocates new memory according to the dimensions and the bytes per pixel.
     */
    void allocateMemory();

    /**
     * Generates a new OpenGL name and stores it in the _id variable.
     */
    void generateId();

    /**
     * Bind the Texture and apply the changes to the OpenGL state according to the current
     * filter. If the FilterMode is FilterMode::AnisotropicMipMap, the Texture has to be
     * uploaded before this method is called.
     */
    void applyFilter();

    /**
     * Bind the Texture and apply the changes to the OpenGL state according to the current
     * wrapping mode.
     */
    void applyWrapping() const;

    void applySwizzleMask();

    /**
     * Calculates the bytes each pixel needs to store its content. This is dependent on
     * the number of channels as well as the data type this texture has. If an unknown
     * data type is stored, an assertion will be triggered.
     */
    void calculateBytesPerPixel();

    /**
     * Upload the passed data pointer to graphics memory by calling glTexImage.
     */
    void uploadDataToTexture(void* pixelData) const;

    /**
     * Re-upload the passed data pointer to graphics memory by calling glTexSubImage.
     */
    void reUploadDataToTexture(void* pixelData) const;

private:
    const std::array<GLenum, 4> DefaultSwizzleMask = {
        GL_RED,
        GL_GREEN,
        GL_BLUE,
        GL_ALPHA
    };

    /**
     * Stores the dimensions of the texture, `y` and `z` may be `1` if the Texture is 1D
     * or 2D.
     */
    glm::uvec3 _dimensions = glm::uvec3(0);
    Format _format;
    GLenum _internalFormat;
    bool _swizzleMaskChanged = false;
    std::array<GLenum, 4> _swizzleMask = DefaultSwizzleMask;
    GLenum _dataType;
    FilterMode _filter;
    WrappingModes _wrapping;
    GLuint _id = 0;
    GLenum _type;
    GLubyte _bpp = 0;
    int _mipMapLevel = 8;
    float _anisotropyLevel = -1.f;
    std::string _name;

    bool _hasOwnershipOfData = false;
    void* _pixels = nullptr;
    int _pixelAlignment = 1;

#ifdef Debugging_Ghoul_Textures_Indices
    int index = 0;
    static int nextIndex;
#endif // Debugging_Ghoul_Textures_Indices
};

} // namespace ghoul::opengl

#include "texture.inl"

#endif // __GHOUL___TEXTURE___H__
