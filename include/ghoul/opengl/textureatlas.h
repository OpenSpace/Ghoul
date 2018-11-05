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
 *****************************************************************************************
 * This code is based on the Freetype GL engine as developed by Nicolas P. Rougier. The  *
 * library is available at http://code.google.com/p/freetype-gl/. His implementation of  *
 * the TextureAtlas is based on the article by Jukka Jylaanki: "A Thousand Ways to Pack  *
 * the Bin - A Practical Approach to Two-Dimensional Rectangle Bin Packing", 2010.       *
 * More precisely, this is an implementation of the Skyline Bottom-Left algorithm based  *
 * on C++ sources provided by Jukka Jylaanki at:                                         *
 * http://clb.demon.fi/files/RectangleBinPack/                                           *
 ****************************************************************************************/

#ifndef __GHOUL___TEXTUREATLAS___H__
#define __GHOUL___TEXTUREATLAS___H__

#include <ghoul/glm.h>
#include <ghoul/misc/exception.h>
#include <memory>
#include <vector>

namespace ghoul::opengl {

class Texture;

/**
 * This class represents a texture atlas which automatically organizes smaller textures
 * in a compact representation. The TextureAtlas is useful if many small textures are
 * needed, but the overhead of creating a separate Texture for each is not desireable.
 * The TextureAtlas is created with a <code>size</code>, and in order to fill the atlas,
 * new regions have to first be requested (#newRegion) and then filled with data
 * (#setRegionData). Due to the fact that the TextureAtlas is represented by a single
 * Texture on the GPU, the <code>depth</code> can only be <code>1</code>, <code>2</code>,
 * <code>3</code>, or <code>4</code>. Before the atlas can be used, it has to be uploaded
 * to the GPU first (#upload).
 */
class TextureAtlas {
public:
    /// Exception that gets thrown if an invalid region would be returned or is used
    struct InvalidRegionException : RuntimeError {
        explicit InvalidRegionException(std::string msg);
    };

    typedef int RegionHandle;

    /**
     * The constructor completely initializes the Texture Atlas. No additional
     * initialization step is necessary. Due to the fact that the underlying Texture is
     * initialized here, it requires a valid OpenGL context. The size is defined as
     * \c width, \c height, and \c depth.
     *
     * \param size The size (<code>width</code>, <code>height</code>, <code>depth</code>)
     *        of the TextureAtlas.
     *
     * \pre \p size's width and height has to be bigger than <code>4</code> and smaller
     *      than the GPU limit for 2D textures. The \p size's depth has to be
     *      <code>1</code>, <code>2</code>, <code>3</code>, or <code>4</code>
     * \pre \c width component must be bigger than <code>4</code> and smaller than the GPU
     *      limit for 2D tetxures
     * \pre \c height component must be bigger than <code>4</code> and smaller than the
     *      GPU limit for 2D tetxures
     * \pre \c depth component must be <code>1</code>, <code>2</code>, <code>3</code>, or
     *      <code>4</code>
     */
    explicit TextureAtlas(glm::ivec3 size);

    /**
     * Copy constructor that performs a deep copy of all the element in the TextureAtlas
     * such that the resulting atlas can be used independently.
     *
     * \param rhs The original TextureAtlas
     */
    TextureAtlas(const TextureAtlas& rhs);

    /**
     * Move constructor that leaves the incoming atlas invalid.
     *
     * \param rhs The origin TextureAtlas
     */
    TextureAtlas(TextureAtlas&& rhs);

    /**
     * Assignment operator that performs a deep copy of all the elements in the
     * TextureAtlas such that the resulting atlas can be used independently.
     *
     * \param rhs The original TextureAtlas
     * \return A copy of the original TextureAtlas
     */
    TextureAtlas& operator=(const TextureAtlas& rhs);

    /**
     * Move operator that moves the data into the new atlas without performing any copy
     * operations. This leaves the original atlas invalid.
     *
     * \param rhs The original TextureAtlas
     * \return The atlas into which the original values were moved into
     */
    TextureAtlas& operator=(TextureAtlas&& rhs);

    /**
     * Initializes the TextureAtlas and creates its backend storage. This method
     * requires a valid OpenGL context.
     */
    void initialize();

    /**
    * Deinitializes the TextureAtlas and cleans its backend storage. This method
    * requires a valid OpenGL context.
    */
    void deinitialize();

    /**
     * Uploads the TextureAtlas to the graphics card. This function requires a valid
     * OpenGL context.
     */
    void upload();

    /**
     * Clears the TextureAtlas of all data, but leaves the underlying Texture unchanged.
     * A separate call to #upload is required to change the representation on the GPU as
     * well.
     */
    void clear();

    /**
     * Allocate a new region in the TextureAtlas with the desired \p width and \p height.
     * Please note that the internal width and height of the region will be increased by 1
     * pixel to account for a margin and prevent interpolation issues. The only
     * implication for the usage is the increase storage requirement.
     *
     * \param width The width of the requested region
     * \param height The height of the requested region
     * \return A handle to the new region that can be passed to the #setRegionData and
     *         #textureCoordinates functions
     *
     * \throw InvalidRegionException If the new requested region does not fit in the
     *        TextureAtlas
     */
    RegionHandle newRegion(int width, int height);

    /**
     * Sets the data in the region designated by the \p handle to the passed \p data.
     * <b>Warning</b>: Please note that this function does not check for buffer overflows
     * or underflows. In all cases,
     * <code>width(region) * height(region) * depth(atlas)</code> number of bytes are read
     * from the <code>data</code> block.
     *
     * \param handle The handle of the region for which the data is provided
     * \param data The data that should be set for the specified region
     *
     * \pre \p data must not be a <code>nullptr</code>
     */
    void setRegionData(RegionHandle handle, void* data);

    /// Structure that is returned from textureCoordinates function
    struct TextureCoordinatesResult {
        glm::vec2 topLeft;
        glm::vec2 bottomRight;
    };

    /**
     * Returns the texture coordinates that define the provided region. If the returned
     * <code>topLeft</code> and <code>bottomRight</code> coordinates are used as texture
     * coordinates, the result will be the same as if the data would have been bound to a
     * separate texture. The \p windowing parameter provides possiblity to offset the
     * starting points (with the first two arguments and restrict the width (with the
     * third and fourth arguments).
     *\verbatim
 -------------------
|         b         |
|    -----------    |
| a |           | c |
|   |           |   |
|    -----------    |
|         d         |
 -------------------
     *\endverbatim
     * <code>windowing.x = a</code><br>
     * <code>windowing.y = b</code><br>
     * <code>windowing.z = c</code><br>
     * <code>windowing.w = d</code><br>
     *
     *
     * \param handle The handle of the region for which the texture coordinates shall be
     * retrieved
     * \param windowing Determines whether a subset of the region should be retrieved. If
     *        this parameter is equal to <code>glm::ivec4(0)</code>, the full region is
     *        returned. The first two parameters <code>x</code> and <code>y</code>
     *        determine an offset for the top left corner, while the third and fourth
     *        parameters are subtracted from the bottom right corner. That means that if
     *        <code>windowing</code> is equal to
     *        <code>glm::ivec4(width / 4, height / 4, width / 4, height / 4</code>, a
     *        subset in the center of the region is returned.
     * \return A TextureCoordinatesResult structure containing the <code>topLeft</code>
     *         corner of the region in texture coordinates and that
     *        <code>bottomRight</code> corner of the region in texture coordinates
     */
    TextureCoordinatesResult textureCoordinates(
        RegionHandle handle, const glm::ivec4& windowing = glm::ivec4(0)) const;

    /**
     * Returns the size of the TextureAtlas in <code>width</code>, <code>height</code>,
     * and <code>depth</code>.
     *
     * \return The size of the TextureAtlas in <code>width</code>, <code>height</code>,
     *         and <code>depth</code>.
     */
    glm::ivec3 size() const;

    /**
     * Returns the amount of pixels out of the maximum size (<code>width</code> *
     * <code>height</code>) that are currently in use. Please note that this is <b>not</b>
     * equal to the amount of pixels that can possiblity be used due to fragmentation in
     * the atlas.
     *
     * \return The amount of pixels that are currently in use in the atlas.
     */
    int spaceUsed() const;

    /**
     * Returns the Texture that is the underlying storage for the TextureAtlas. This
     * Texture can bound to a ProgramObject and subsequently sampled to retrieve the
     * stored textures.
     *
     * \return The Texture that is the underlying storage for this TextureAtlas.
     */
    const Texture& texture() const;

private:
    int atlasFit(size_t index, int width, int height);
    void atlasMerge();

    std::vector<glm::ivec3> _nodes;

    /// All of the individual elements that are stored in the atlas
    std::vector<glm::ivec4> _handleInformation;

    /// Size of the texture
    glm::ivec3 _size;

    /// Allocated surface size
    int _nUsed = 0;

    /// Pointer to the texture that is used as the atlas
    std::unique_ptr<Texture> _texture;

    /// Backend data storage for the texture
    std::vector<unsigned char> _data;
};

} // namespace ghoul::opengl

#endif // __GHOUL___TEXTUREATLAS___H__
