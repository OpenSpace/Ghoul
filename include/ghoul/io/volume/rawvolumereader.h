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

#ifndef __GHOUL___RAWVOLUMEREADER___H__
#define __GHOUL___RAWVOLUMEREADER___H__

#include <ghoul/io/volume/volumereader.h>

namespace ghoul {

class RawVolumeReader : public VolumeReader {
public:
    struct ReadHints {
        ReadHints(glm::ivec3 dimensions = glm::ivec3(0));
        glm::ivec3 _dimensions;
        opengl::Texture::Format _format;
        GLenum _internalFormat;
    };

    RawVolumeReader();
    RawVolumeReader(const ReadHints& hints);
    ~RawVolumeReader();

    void setReadHints(glm::ivec3 dimension);
    void setReadHints(const ReadHints& hints);

    opengl::Texture* read(const std::string& filename) override;

private:
    ReadHints _hints;
};

} // namespace ghoul

#endif // __GHOUL___RAWVOLUMEREADER___H__
