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

#ifndef __GHOUL___MODELREADERMULTIFORMAT___H__
#define __GHOUL___MODELREADERMULTIFORMAT___H__

#include <ghoul/io/model/modelreaderbase.h>

#include <vector>

struct aiMesh;
struct aiScene;

namespace ghoul::io {

/**
 * This model reader loads the provided file using the Assimp library. This simple
 * method loads multiple shapes, from different 3D file formats:
 * Autodesk ( .fbx )
 * Collada ( .dae )
 * glTF ( .gltf, .glb )
 * Blender 3D ( .blend )
 * 3ds Max 3DS ( .3ds )
 * 3ds Max ASE ( .ase )
 * Wavefront Object ( .obj )
 * Industry Foundation Classes (IFC/Step) ( .ifc )
 * XGL ( .xgl,.zgl )
 * Stanford Polygon Library ( .ply )
 * AutoCAD DXF ( .dxf )
 * LightWave ( .lwo )
 * LightWave Scene ( .lws )
 * Modo ( .lxo )
 * Stereolithography ( .stl )
 * DirectX X ( .x ) <-- Disabled
 * AC3D ( .ac )
 * Milkshape 3D ( .ms3d )
 * TrueSpace ( .cob,.scn )
 *
 * \sa https://github.com/assimp/assimp
 */
class ModelReaderMultiFormat : public ModelReaderBase {
public:
    /**
     * Loads the 3D model (anyone from the previous list) file pointed to by \p filename
     * and returns a constructed VertexBufferObject from it. Provided materials are
     * ignored and all shapes are collapsed into one VertexBufferObject.
     *
     * \param filename The geometric model file to be loaded
     * \return The initialized VertexBufferObject containing the model or models
     *
     * \throw ModelReaderException If there was an error reading the model from
     *        \p filename
     * \pre \p filename must not be empty
     */
    std::unique_ptr<opengl::VertexBufferObject> loadModel(
        const std::string& filename) const override;

    /**
     * Loads the 3D model (anyone from the previous list) file pointed to by \p filename
     * and returns populates the vertexArray and indexArray objects. Provided materials
     * are ignored and all shapes are collapsed into one VertexBufferObject.
     *
     * \param filename The geometric model file to be loaded
     * \param vertexArray STL vector of ModelReaderBase::Vertex objects
     * \param indexArray STL vector of integers containing the indexes for the vertexes
     * \return The initialized VertexBufferObject containing the model or models
     *
     * \throw ModelReaderException If there was an error reading the model from
     *        \p filename
     * \pre \p filename must not be empty
     */
    void loadModel(const std::string& filename, std::vector<Vertex>& vertexArray,
        std::vector<int>& indexArray) const;
};

} // namespace ghoul::io

#endif // __GHOUL___MODELREADERMULTIFORMAT___H__
