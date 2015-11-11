/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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
 *
 * This code is based on the Freetype GL engine as developed by Nicolas P. Rougier. The  *
 * library is available at http://code.google.com/p/freetype-gl/. His implementation of  *
 * the TextureAtlas is based on the article by Jukka Jylänki: "A Thousand Ways to Pack   *
 * the Bin - A Practical Approach to Two-Dimensional Rectangle Bin Packing", 2010.       *
 * More precisely, this is an implementation of the Skyline Bottom-Left algorithm based  *
 * on C++ sources provided by Jukka Jylänki at:                                          *
 * http://clb.demon.fi/files/RectangleBinPack/                                           *
 ****************************************************************************************/

#ifndef __TEXTUREATLAS_H__
#define __TEXTUREATLAS_H__

#include "ghoul/glm.h"
#include "ghoul/opengl/ghoul_gl.h"
#include "ghoul/opengl/texture.h"

#include <vector>

namespace ghoul {
namespace opengl {
    
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
    static const glm::ivec4 InvalidRegion;
    /**
     * The constructor completely initializes the Texture Atlas. No additional 
     * initialization step is necessary. Due to the fact that the underlying Texture is
     * initialized here, it requires a valid OpenGL context.
     * \param size The size (<code>width</code>, <code>height</code>, <code>depth</code>)
     * of the TextureAtlas. The <code>width</code> and <code>height</code> have to be
     * bigger than <code>4</code> and smaller than the GPU limit for 2D tetxures. The
     * <code>depth</code> has to be <code>1</code>, <code>2</code>, <code>3</code>, or
     * <code>4</code>.
     */
    TextureAtlas(glm::ivec3 size);
    
    /**
     * The constructor completely initializes the Texture Atlas. No additional
     * initialization step is necessary. Due to the fact that the underlying Texture is
     * initialized here, it requires a valid OpenGL context.
     * \param width The width of the TextureAtlas; has to be bigger than <code>4</code>
     * and smaller than the GPU limit for 2D tetxures.
     * \param height The height of the TextureAtlas; has to be bigger than <code>4</code>
     * and smaller than the GPU limit for 2D tetxures.
     * \param depth The depth of the TetureAtlas; has to be <code>1</code>,
     * <code>2</code>, <code>3</code>, or <code>4</code>.
     */
    TextureAtlas(int width, int height, int depth);
    
    /**
     * Copy constructor that performs a deep copy of all the element in the
     * TextureAtlas such that the resulting atlas can be used independently.
     * \param rhs The original TextureAtlas
     */
    TextureAtlas(const TextureAtlas& rhs);
    
    /**
     * Move constructor that leaves the incoming atlas invalid.
     * \rhs The origin TextureAtlas
     */
    TextureAtlas(TextureAtlas&& rhs);

    /// The destructor taking care of the allocated memory and texture identifiers.
    ~TextureAtlas();
    
    /**
     * Assignment operator that performs a deep copy of all the elements in the
     * TextureAtlas such that the resulting atlas can be used independently.
     * \param rhs The original TextureAtlas
     * \return A copy of the original TextureAtlas
     */
    TextureAtlas& operator=(const TextureAtlas& rhs);
    
    /**
     * Move operator that moves the data into the new atlas without performing any copy
     * operations. This leaves the original atlas invalid.
     * \param rhs The original TextureAtlas
     * \return The atlas into which the original values were moved into
     */
    TextureAtlas& operator=(TextureAtlas&& rhs);
    
    /**
     * Uploads the TextureAtlas to the graphics card. This function requires a valid
     * OpenGL context.
     */
    void upload();
    
    /**
     * Clears the TextureAtlas of all data, but leaves the underlying Texture unchanged.
     * A separate call to #upload is required to change the representation on the GPU as
     * well
     */
    void clear();

    /**
     * Allocate a new region in the TextureAtlas with the desired <code>width</code> and
     * <code>height</code>. If a region cannot be created, the returned vector is equal to
     * #InvalidRegion. The only reason for this function to fail is if there is not enough
     * free space in the atlas.
     * \param width The width of the requested region
     * \param height The height of the requested region
     * \return A vector describing the location of the region in the TextureAtlas. The
     * first two elements are the <code>x</code> and <code>y</code> coordinates of the
     * top left corner. The last two elements are the <code>width</code> and
     * <code>height</code> of the region.
     */
    glm::ivec4 newRegion(int width, int height);

    void setRegionData(const glm::ivec4& region, void* data);
    void setRegionData(int x, int y, int width, int height, void* data);
    
    /**
     * Returns the size of the TextureAtlas in <code>width</code>,
     * <code>height</code>, and <code>depth</code>.
     * \return The size of the TextureAtlas in <code>width</code>,
     * <code>height</code>, and <code>depth</code>.
     */
    glm::ivec3 size() const;
    
    /**
     * Returns the amount of pixels out of the maximum size
     * (<code>width</code> * <code>height</code>) that are currently in use.
     * Please note that this is <b>not</b> equal to the amount of pixels that
     * can possiblity be used due to fragmentation in the atlas.
     * \return The amount of pixels that are currently in use in the atlas.
     */
    int spaceUsed() const;
    
    /**
     * Returns the Texture that is the underlying storage for the
     * TextureAtlas. This Texture can bound to a ProgramObject and 
     * subsequently sampled to retrieve the stored textures.
     * \return The Texture that is the underlying storage for this 
     * TextureAtlas.
     */
    const Texture& texture() const;

private:
    int atlasFit(size_t index, int width, int height);
    void atlasMerge();
    
    std::vector<glm::ivec3> _nodes;
    Texture* _texture;
    
    glm::ivec3 _size;
    
    int _nUsed; ///< Allocated surface size
    
    unsigned char* _data;
};

} // namespace opengl
} // namespace ghoul

#endif // __TEXTUREATLAS_H__
