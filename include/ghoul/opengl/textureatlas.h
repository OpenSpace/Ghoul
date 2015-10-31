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

#include "ghoul/opengl/ghoul_gl.h"
#include "ghoul/glm.h"

#include <vector>

namespace ghoul {
namespace opengl {
    
/**
 * This class represents a texture atlas which can hold and automatically ...
 */
class TextureAtlas {
public:
    /// width, height, depth of the underlying texture
    TextureAtlas(int width, int height, int depth);
    ~TextureAtlas();
    
    /// Upload to graphics card
    void upload();
    
    /// clear all data
    void clear();
    
    /// allocate a new region of size 'width' * 'height'
    glm::ivec4 allocateRegion(int width, int height);
    
    /// fill specific region with data
    void setRegion(int x, int y, int width, int height, void* data, int stride);
    
    int width() const;
    int height() const;
    int depth() const;

private:
    int atlasFit(size_t index, int width, int height);
    void atlasMerge();
    
    std::vector<glm::ivec3> _nodes;
    
    int _width;
    int _height;
    int _depth;
    
    int _nUsed; ///< Allocated surface size
    
    unsigned int _id;
    
    unsigned char* _data;
};

} // namespace opengl
} // namespace ghoul

#endif // __TEXTUREATLAS_H__
