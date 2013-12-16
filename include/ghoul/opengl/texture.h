/**************************************************************************************************
 * GHOUL                                                                                          *
 * General Helpful Open Utility Library                                                           *
 *                                                                                                *
 * Copyright (c) 2012 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************************************
 * This class is inspired by the file 'texture.{h|cpp}' from the tiny graphics toolbox created by *
 * the Visualization and Computer Graphics Group, University of Muenster, Germany                 *
 *************************************************************************************************/

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/glm.h>
#include <glm/gtx/std_based_type.hpp>
#include <string>

namespace ghoul {
namespace opengl {

/**
 * This class is a wrapper for an OpenGL texture. It wraps the OpenGL method for generating (
 * <code>glGenTextures</code>), binding (<code>glBindTexture</code>), enabling 
 * <code>glEnable</code>) and others. A new texture can be created either from an image file using
 * the TextureManager or TextureReader class or using a pointer to the data. The data layout must
 * be as specified bt the <code>glTexImageXD</code> functions (see OpenGL reference). After
 * creation a Texture must be uploaded (<code>uploadTexture</code>) before it can be used in the
 * graphics pipeline.
 * See <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url> for detailed
 * information on the different arguments
 */
class Texture {
public:
    /**
     * This enum specifies the allowed formats for the Textures. These are directly mapped to the
     * appropriate OpenGL constants. See
     * <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url> for more information.
     */
    enum Format {
        FormatRed = GL_RED, ///< GL_RED
        FormatRG = GL_RG, ///< GL_RG
        FormatRGB = GL_RGB, ///< GL_RGB
        FormatBGR = GL_BGR, ///< GL_BGR
        FormatRGBA = GL_RGBA, ///< GL_RGBA
        FormatBGRA = GL_BGRA, ///< GL_BGRA
        FormatDepthComponent = GL_DEPTH_COMPONENT ///< GL_DEPTH_COMPONENT
    };

    /**
     * This enum specifies the filtering method this texture will use to interpolate between two
     * texel. The values for this enum correspond directly to OpenGL settings. See the OpenGL
     * specification for details.
     */
    enum FilterMode {
        FilterModeNearest, ///< GL_NEAREST
        FilterModeLinear, ///< GL_LINEAR
        FilterModeAnisotropicMipMap
    };

    /**
     * This enum specifies the wrapping mode this texture will use at the edges of the texture. The
     * values for this enum correspond directly to OpenGL settings. See the OpenGL specification
     * for details.
     */
    enum WrappingMode {
        WrappingModeRepeat = GL_REPEAT, ///< GL_REPEAT
        WrappingModeClamp = GL_CLAMP, ///< GL_CLAMP
        WrappingModeClampToEdge = GL_CLAMP_TO_EDGE, ///< GL_CLAMP_TO_EDGE
        WrappingModeClampToBorder = GL_CLAMP_TO_BORDER, ///< GL_CLAMP_TO_BORDER
        WrappingModeMirroredRepeat = GL_MIRRORED_REPEAT ///< GL_MIRRORED_REPEAT
    };

    /**
     * This constructor will create storage internally to fit the amount of data that is necessary
     * for the dimensions * bytesPerPixel (which is in turn dependent on the dataType). The texture
     * will be a 3D texture unless one or two of the components in dimensions is 1.
     * \param [in] dimensions The dimensions of the texture. A 3D texture will be created if all 
     * components are bigger than 1, a 2D texture will be created if the z component is equal to 1,
     * while a 1D texture is created if the y and z component is equal to 1.
     * \param [in] format Specifies the format of the data
     * \param [in] internalFormat The internal format for the texture. See
     * <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url> Tables 1,2, and 3 for
     * concrete values. In addition, the S3TC_DXT formats can be used to support hardware
     * compression. See <url>http://www.opengl.org/wiki/Image_Format#S3TC.2FDXT</url> for more
     * information.
     * \param [in] dataType The data type of the pixel data. See
     * <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url> for a list of possible
     * values.
     * \param [in] filter The FilterMode that will be used to interpolate between texels.
     * \param [in] wrapping The WrappingMode that will be used to generate values on the border of
     * the texture
     */
    Texture(const glm::size3_t& dimensions, Format format = FormatRGBA, GLint internalFormat = GL_RGBA,
        GLenum dataType = GL_UNSIGNED_BYTE, FilterMode filter = FilterModeLinear, WrappingMode
        wrapping = WrappingModeRepeat);

    /**
     * This constructor will generate a Texture out of the passed data. The data should contain
     * enough bytes to fill dimensions * bytesPerPixel (which is in turn dependent on the dataType)
     * pixel. The texture will be a 3D texture unless one or two of the components in dimensions is
     * 1.
     * \param [in] data The data from which to generate the Texture. The data must be in a linear
     * format and (in 2D and 3D cases) aligned so that it can be accessed using the following
     * equations:
     * <code>(y * dimensions.x) + x</code> in the 2D case and 
     * <code>(z * dimensions.x * dimensions.y) + (y * dimensions.x) + x</code> in the 3D case.
     * This Texture will take ownership of the data array and will delete it once this object is
     * destroyed.
     * \param [in] dimensions The dimensions of the texture. A 3D texture will be created if all 
     * components are bigger than 1, a 2D texture will be created if the z component is equal to 1,
     * while a 1D texture is created if the y and z component is equal to 1.
     * \param [in] format Specifies the format of the data
     * \param [in] internalFormat The internal format for the texture. See
     * <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url> Tables 1,2, and 3 for
     * concrete values. In addition, the S3TC_DXT formats can be used to support hardware
     * compression. See <url>http://www.opengl.org/wiki/Image_Format#S3TC.2FDXT</url> for more
     * information.
     * \param [in] dataType The data type of the pixel data. See
     * <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url> for a list of possible
     * values.
     * \param [in] filter The FilterMode that will be used to interpolate between texels.
     * \param [in] wrapping The WrappingMode that will be used to generate values on the border of
     * the texture
     */
    Texture(void* data, const glm::size3_t& dimensions, Format format = FormatRGBA,
        GLint internalFormat = GL_RGBA, GLenum dataType = GL_UNSIGNED_BYTE,
        FilterMode filter = FilterModeLinear, WrappingMode wrapping = WrappingModeRepeat);

    /**
     * Unloads the Texture from GPU memory and destroys the id. The destructor will also remove the
     * data associated with this texture.
     */
    ~Texture();

    /// Enables this texture type by calling <code>glEnable</code>
    void enable();
    /// Disables this texture type by calling <code>glDisable</code>
    void disable();

    /// Binds this texture to the texture type by calling <code>glBindTexture</code>
    void bind();

    /// Returns the OpenGL name of this texture.
    operator GLuint() const;

    /**
     * Returns an (optional) name for this texture. The name is not used internally and is solely
     * for external purposes. One possible use is the filename from which the texture was loaded.
     * \return The name for this texture
     */
    const std::string& getName() const;

    /**
     * Sets an (optional) name for this texture. The name is not used internally and is solely
     * for external purposes. One possible use is the filename from which the texture was loaded.
     * \param [in] name The name for this texture
     */
    void setName(const std::string& name);

    /**
     * Returns the type for this texture.
     * \returns The type for this texture. This value can either be GL_TEXTURE_1D, GL_TEXTURE_2D or
     * GL_TEXTURE_3D depending on the dimensionality of the stored texture.
     */
    GLenum type() const;

    /**
     * Overrides the type of the texture that was automatically determined.
     * \param [in] The new type for the texture. This value can either be GL_TEXTURE_1D,
     * GL_TEXTURE_2D or GL_TEXTURE_3D.
     */
    void setType(GLenum type);

    /**
     * Returns the dimensions of this texture. If the texture is a 2D texture, the z component
     * will be equal to 1 and if the texture is a 1D texture, the y and z components will be equal
     * to 1.
     * \return The dimensions of this texture.
     */
    const glm::size3_t& dimensions() const;

    /**
     * Sets new dimensions for this texture. The texture is not automatically updated or uploaded
     * to the graphics card. Solely the stored dimensions are changed. 
     * \param [in] dimensions The new dimensions for this texture
     */
    void setDimensions(const glm::size3_t& dimensions);

    /// Returns the width of the stored texture
    size_t width() const;

    /// Returns the height of the stored texture or 1 if it is a 1D texture
    size_t height() const;

    /// Returns the depth of the stored texture or 1 if it is a 1D or 2D texture
    size_t depth() const;

    /**
     * Returns the format for this texture. 
     * \returns The format for this texture.
     */
    Format format() const;

    /**
     * Sets the format for this texture. This only updates the internal state and doesn't affect
     * the texture on the graphics card. Call <code>uploadTexture</code> to apply changes.
     * \param [in] format The new format for this texture.
     */
    void setFormat(Format format);

    /** 
     * Returns the internal format for this texture. See
     * <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url> for more information
     * and the possible return values.
     */
    GLint internalFormat() const;

    /**
     * Sets the new internal format for this texture. This only updates the internal state and
     * doesn't affect the texture on the graphics card. Call <code>uploadTexture</code> to apply
     * changes. See <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url> for more
     * information and possible parameter values.
     * \param [in] The new internal format. See
     * <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url> Tables 1, 2, and 3 for
     * possible values.
     */
    void setInternalFormat(GLint internalFormat);

    /// Returns the FilterMode used by this texture
    FilterMode filter() const;

    /**
     * Sets a new FilterMode for this Texture. The new FilterMode is applied immediately. If the
     * filter is FilterMode::AnisotropicMipMap, the texture has to be uploaded before calling this
     * method. Otherwise, the result is undefined.
     * \param [in] filter The new FilterMode for this Texture
     */
    void setFilter(FilterMode filter);

    /**
     * Returns the storage data type for this Texture. For a complete list of available return
     * values see <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url> for more
     * information.
     * \return The storage data type
     */
    GLenum dataType() const;

    /**
     * Sets a new storage data type for this Texture. For a complete list of available parameters
     * see <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url>. The new data type
     * is only stored internally and not updated immediately, although it will update the number of
     * bytes per pixel (see bytesPerPixel method).
     * \param [in] dataType The new storage data type
     */
    void setDataType(GLenum dataType);

    /**
     * Returns the number of channels that are stored in this texture. If the format of the Texture
     * is not in the list of formats found at 
     * <url>http://www.opengl.org/sdk/docs/man/xhtml/glTexImage1D.xml</url>, an assertion will be
     * triggered.
     */
    size_t numberOfChannels() const;

    /// Returns the number of bytes each pixel stores
    GLubyte bytesPerPixel() const;

    /**
     * Returns the stored data of the texture. If the memory is deleted, a new (sufficiently)
     * large memory block have been created or a seg fault might occur!
     */
    const void* pixelData() const;

    /**
     * Returns the size the pixel data should have according to the dimensionality and the bytes
     * per pixel. <code>dimensions.x * dimensions.y * dimensions.z * bpp</code>. The real size of
     * the data can be different if it was set manually.
     */
    size_t expectedPixelDataSize() const;

    /**
     * Sets new data for the texture to use. If the dimensions are not updated and the new data has
     * a different size, undefined behavior will occur. This Texture will take ownership of the
     * data array.
     * \param [in] pixels The pointer to the new data array that should be used.
     * \param [in] takeOwnership Should this Texture take ownership of the data and delete it?
     */
    void setPixelData(void* pixels, bool takeOwnership = true);

    /// Returns <code>true</code> if the OpenGL texture is resident in the GPU memory.
    bool isResident() const;

    /// Sets a new WrappingMode for this Texture. The new mode is applied immediately.
    void setWrapping(WrappingMode wrapping);

    /// Returns the currently used Wrapping Mode for this texture
    WrappingMode wrapping() const;

    /**
     * Sets the maximum number of MipMap levels to use. This is only valid when the
     * FilterMode::AnisotropicMipMap is selected. Defaults to 8 levels.
     * \param [in] mipMapLevel The MipMap level that should be used in this texture.
     */
    void setMipMapLevel(int mipMapLevel);

    /**
     * Sets the maximum anisotropy level that should be used. This is only valid when the
     * FilterMode::AnisotropicMipMap is selected. On default, the maximum anisotropy supported by
     * the graphics card is used.
     * \param [in] anisotropyLevel The anisotropy level that should be used
     */
    void setAnisotropyLevel(float anisotropyLevel);

    /**
     * Binds and uploads the texture to graphics memory. The Texture has to be of type
     * GL_TEXTURE_1D, GL_TEXTURE_2D, or GL_TEXTURE_3D. The type will be determined automatically
     * based on the provided dimensions
     */
    void uploadTexture();

    /**
     * Downloads the texture from graphics memory and makes it available using the pixelData
     * method.
     */
    void downloadTexture();

    /**
     * Changes (=grants/revokes) ownership of the stored data. Changing this will not change the
     * underlying data.
     * \param [in] hasOwnership <code>true</code> if this Texture should own the data in
     * <code>_pixels</code>
     */
    void setDataOwnership(bool hasOwnership);

    /**
     * Returns if this Texture owns the data contained in <code>_pixels</code>
     */
    bool dataOwnership() const;

    /**
     * Accesses the <code>i</code>-th texel in the data array. Returns a reference to it so that it
     * can be changed by the caller. The Texture must be a one dimensional Texture or an assertion
     * will be triggered. Only Textures with a data type of the following list can be used to fetch
     * textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,
     * GL_FLOAT. Trying to use this function on another type will lead to undefined behavior in the
     * return value.
     * \tparam T The type of the data that is returned. The size of T must be equal to the bytes per
     * pixel stored in the Texture or an assertion will be triggered.
     * \param i The position of the texel that should be fetched. This value must be with in the
     * width of the Texture or an assertion will be triggered.
     * \return The texel at the specified position casted to the requested type T.
     */
    template <class T>
    T& texel(size_t x);

    /**
     * Accesses the <code>i</code>-th texel in the data array. The Texture must be a one
     * dimensional Texture or an assertion will be triggered. Only Textures with a data type of the
     * following list can be used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT,
     * GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type
     * will lead to undefined behavior in the return value.
     * \tparam T The type of the data that is returned. The size of T must be equal to the bytes per
     * pixel stored in the Texture or an assertion will be triggered.
     * \param i The position of the texel that should be fetched. This value must be with in the
     * width of the Texture or an assertion will be triggered.
     * \return The texel at the specified position casted to the requested type T.
     */
    template <class T>
    const T& texel(size_t x) const;

    /**
     * Accesses the texel at <code>(x,y)</code> from the data array. Returns a reference to it so
     * that it can be changed by the caller. The Texture must be a 2D dimensional Texture or an
     * assertion will be triggered. Only Textures with a data type of the following list can be
     * used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT,
     * GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type will lead to
     * undefined behavior in the return value.
     * \tparam T The type of the data that is returned. The size of T must be equal to the bytes per
     * pixel stored in the Texture or an assertion will be triggered.
     * \param x The coordinate on the <code>width</code> axis. The value must be smaller than the
     * width of the Texture or an assertion will be triggered.
     * \param y The coordinate on the <code>height</code> axis. The value must be smaller than the
     * height of the Texture or an assertion will be triggered.
     * \return The texel at the specified position casted to the requested type T.
     */
    template <class T>
    T& texel(size_t x, size_t y);

    /**
     * Accesses the texel at <code>(x,y)</code> from the data array. The Texture must be a 2D
     * dimensional Texture or an assertion will be triggered. Only Textures with a data type of the
     * following list can be used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT,
     * GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type
     * will lead to undefined behavior in the return value.
     * \tparam T The type of the data that is returned. The size of T must be equal to the bytes per
     * pixel stored in the Texture or an assertion will be triggered.
     * \param x The coordinate on the <code>width</code> axis. The value must be smaller than the
     * width of the Texture or an assertion will be triggered.
     * \param y The coordinate on the <code>height</code> axis. The value must be smaller than the
     * height of the Texture or an assertion will be triggered.
     * \return The texel at the specified position casted to the requested type T.
     */
    template<class T>
    const T& texel(size_t x, size_t y) const;

    /**
     * Accesses the texel at <code>position</code> from the data array. Returns a reference to it
     * so that it can be changed by the caller. The Texture must be a 2D dimensional Texture or an
     * assertion will be triggered. Only Textures with a data type of the following list can be
     * used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT,
     * GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type will lead to
     * undefined behavior in the return value.
     * \tparam T The type of the data that is returned. The size of T must be equal to the bytes per
     * pixel stored in the Texture or an assertion will be triggered.
     * \param position The coordinate of the texel. <code>position.x</code> must be smaller than
     * the width of the Texture, while <code>position.y</code> must be smaller than the height.
     * \return The texel at the specified position casted to the requested type T.
     */
    template <class T>
    T& texel(const glm::size2_t& position);

    /**
     * Accesses the texel at <code>position</code> from the data array. The Texture must be a 2D
     * dimensional Texture or an assertion will be triggered. Only Textures with a data type of the
     * following list can be used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT,
     * GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type
     * will lead to undefined behavior in the return value.
     * \tparam T The type of the data that is returned. The size of T must be equal to the bytes per
     * pixel stored in the Texture or an assertion will be triggered.
     * \param position The coordinate of the texel. <code>position.x</code> must be smaller than
     * the width of the Texture, while <code>position.y</code> must be smaller than the height.
     * \return The texel at the specified position casted to the requested type T.
     */
    template<class T>
    const T& texel(const glm::size2_t& position) const;

    /**
     * Accesses the texel at <code>(x,y,z)</code> from the data array. Returns a reference to it so
     * that it can be changed by the caller. The Texture must be a 3D dimensional Texture or an
     * assertion will be triggered. Only Textures with a data type of the following list can be
     * used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT,
     * GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type will lead to
     * undefined behavior in the return value.
     * \tparam T The type of the data that is returned. The size of T must be equal to the bytes per
     * pixel stored in the Texture or an assertion will be triggered.
     * \param x The coordinate on the <code>width</code> axis. The value must be smaller than the
     * width of the Texture or an assertion will be triggered.
     * \param y The coordinate on the <code>height</code> axis. The value must be smaller than the
     * height of the Texture or an assertion will be triggered.
     * \param z The coordinate on the <code>depth</code> axis. The value must be smaller than the
     * depth of the Texture or an assertion will be triggered.
     * \return The texel at the specified position casted to the requested type T.
     */
    template <class T>
    T& texel(size_t x, size_t y, size_t z);

    /**
     * Accesses the texel at <code>(x,y,z)</code> from the data array. The Texture must be a 3D
     * dimensional Texture or an assertion will be triggered. Only Textures with a data type of the
     * following list can be used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT,
     * GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type
     * will lead to undefined behavior in the return value.
     * \tparam T The type of the data that is returned. The size of T must be equal to the bytes per
     * pixel stored in the Texture or an assertion will be triggered.
     * \param x The coordinate on the <code>width</code> axis. The value must be smaller than the
     * width of the Texture or an assertion will be triggered.
     * \param y The coordinate on the <code>height</code> axis. The value must be smaller than the
     * height of the Texture or an assertion will be triggered.
     * \param z The coordinate on the <code>depth</code> axis. The value must be smaller than the
     * depth of the Texture or an assertion will be triggered.
     * \return The texel at the specified position casted to the requested type T.
     */
    template<class T>
    const T& texel(size_t x, size_t y, size_t z) const;

    /**
     * Accesses the texel at <code>position</code> from the data array. Returns a reference to it
     * so that it can be changed by the caller. The Texture must be a 3D dimensional Texture or an
     * assertion will be triggered. Only Textures with a data type of the following list can be
     * used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT,
     * GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type will lead to
     * undefined behavior in the return value.
     * \tparam T The type of the data that is returned. The size of T must be equal to the bytes per
     * pixel stored in the Texture or an assertion will be triggered.
     * \param position The coordinate of the texel. <code>position.x</code> must be smaller than
     * the width of the Texture, while <code>position.y</code> must be smaller than the height and
     * <code>position.z</code> must be smaller than the depth.
     * \return The texel at the specified position casted to the requested type T.
     */
    template <class T>
    T& texel(const glm::size3_t& position);

    /**
     * Accesses the texel at <code>position</code> from the data array. The Texture must be a 3D
     * dimensional Texture or an assertion will be triggered. Only Textures with a data type of the
     * following list can be used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT,
     * GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type
     * will lead to undefined behavior in the return value.
     * \tparam T The type of the data that is returned. The size of T must be equal to the bytes per
     * pixel stored in the Texture or an assertion will be triggered.
     * \param position The coordinate of the texel. <code>position.x</code> must be smaller than
     * the width of the Texture, while <code>position.y</code> must be smaller than the height and
     * <code>position.z</code> must be smaller than the depth.
     * \return The texel at the specified position casted to the requested type T.
     */
    template<class T>
    const T& texel(const glm::size3_t& position) const;

    /**
     * Returns the texel at the specified position as a float vector. Independent of the
     * underlying type, each component of the vector is in the range of [0,1]. The Texture must be
     * 1 dimensional or an assertion will be triggered. Only Textures with a data type of the
     * following list can be used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT,
     * GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type
     * will lead to undefined behavior in the return value.
     * \param x The coordinate on the <code>width</code> axis. The value must be smaller than the
     * width of the Texture or an assertion will be triggered.
     * \return The texel at the specified position as a vector with each component \in [0,1].
     */
    glm::vec4 texelAsFloat(size_t x) const;

    /**
     * Returns the texel at the specified position as a float vector. Independent of the
     * underlying type, each component of the vector is in the range of [0,1]. The Texture must be
     * 2 dimensional or an assertion will be triggered. Only Textures with a data type of the
     * following list can be used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT,
     * GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type
     * will lead to undefined behavior in the return value.
     * \param x The coordinate on the <code>width</code> axis. The value must be smaller than the
     * width of the Texture or an assertion will be triggered.
     * \param y The coordinate on the <code>height</code> axis. The value must be smaller than the
     * height of the Texture or an assertion will be triggered.
     * \return The texel at the specified position as a vector with each component \in [0,1].
     */
    glm::vec4 texelAsFloat(size_t x, size_t y) const;

    /**
     * Returns the texel at the specified position as a float vector. Independent of the
     * underlying type, each component of the vector is in the range of [0,1]. The Texture must be
     * 2 dimensional or an assertion will be triggered. Only Textures with a data type of the
     * following list can be used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT,
     * GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type
     * will lead to undefined behavior in the return value.
     * \param position The coordinate of the texel. <code>position.x</code> must be smaller than
     * the width of the Texture, while <code>position.y</code> must be smaller than the height.
     * \return The texel at the specified position as a vector with each component \in [0,1].
     */
    glm::vec4 texelAsFloat(const glm::size2_t& pos) const;

    /**
     * Returns the texel at the specified position as a float vector. Independent of the
     * underlying type, each component of the vector is in the range of [0,1]. The Texture must be
     * 3 dimensional or an assertion will be triggered. Only Textures with a data type of the
     * following list can be used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT,
     * GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type
     * will lead to undefined behavior in the return value.
     * \param x The coordinate on the <code>width</code> axis. The value must be smaller than the
     * width of the Texture or an assertion will be triggered.
     * \param y The coordinate on the <code>height</code> axis. The value must be smaller than the
     * height of the Texture or an assertion will be triggered.
     * \param z The coordinate on the <code>depth</code> axis. The value must be smaller than the
     * depth of the Texture or an assertion will be triggered.
     * \return The texel at the specified position as a vector with each component \in [0,1].
     */
    glm::vec4 texelAsFloat(size_t x, size_t y, size_t z) const;

    /**
     * Returns the texel at the specified position as a float vector. Independent of the
     * underlying type, each component of the vector is in the range of [0,1]. The Texture must be
     * 3 dimensional or an assertion will be triggered. Only Textures with a data type of the
     * following list can be used to fetch textures: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT,
     * GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT. Trying to use this function on another type
     * will lead to undefined behavior in the return value.
     * \param position The coordinate of the texel. <code>position.x</code> must be smaller than
     * the width of the Texture, while <code>position.y</code> must be smaller than the height and
     * <code>position.z</code> must be smaller than the depth.
     * \return The texel at the specified position as a vector with each component \in [0,1].
     */
    glm::vec4 texelAsFloat(const glm::size3_t& pos) const;

protected:
    /**
     * Initializes the Texture by determining the Texture type, the bytes per pixel, generating an
     * OpenGL name, allocating memory (depending on the passed parameter) and applying the filter
     * and wrapping modes
     */
    void initialize(bool allocateData);

    /// Allocates new memory according to the dimensions and the bytes per pixel
    void allocateMemory();

    /// Frees the stored memory
    void destroyMemory();

    /// Generates a new OpenGL name and stores it in the _id variable
    void generateId();

    /**
     * Bind the Texture and apply the changes to the OpenGL state according to the current filter.
     * If the FilterMode is FilterMode::AnisotropicMipMap, the Texture has to be uploaded before
     * this method is called.
     */
    void applyFilter();

    /**
     * Bind the Texture and apply the changes to the OpenGL state according to the current
     * wrapping mode.
     */
    void applyWrapping();

    /**
     * Calculates the bytes each pixel needs to store its content. This is dependent on the number
     * of channels as well as the data type this texture has. If an unknown data type is stored,
     * an assertion will be triggered.
     */
    void calculateBytesPerPixel();

    /// Determines from the dimensions whether this is a 1D, 2D, or 3D texture
    void determineTextureType();

private:
    glm::size3_t _dimensions;
    Format _format;
    GLint _internalFormat;
    GLenum _dataType;
    FilterMode _filter;
    WrappingMode _wrapping;
    GLuint _id;
    GLenum _type;
    GLubyte _bpp;
    int _mipMapLevel;
    float _anisotropyLevel;
    std::string _name;

    bool _hasOwnershipOfData;
    void* _pixels;
};

} // namespace opengl
} // namespace logging

#include "texture.inl"

#endif
