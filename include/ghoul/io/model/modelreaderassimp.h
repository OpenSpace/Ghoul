/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2021                                                               *
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

#ifndef __GHOUL___MODELREADERASSIMP___H__
#define __GHOUL___MODELREADERASSIMP___H__

#include <ghoul/io/model/modelreaderbase.h>

#include <vector>

struct aiMesh;
struct aiScene;

namespace ghoul::io {

/**
 * This model reader loads the provided file using the Assimp library. This simple
 * method loads multiple shapes, from different 3D file formats (An asterisk 
 * indicates limited support):
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
 * AutoCAD DXF ( .dxf ) *
 * LightWave ( .lwo )
 * LightWave Scene ( .lws )
 * Modo ( .lxo )
 * Stereolithography ( .stl )
 * DirectX X ( .x )
 * AC3D ( .ac )
 * Milkshape 3D ( .ms3d )
 * TrueSpace ( .cob,.scn ) *
 *
 * \sa https://github.com/assimp/assimp
 */
class ModelReaderAssimp : public ModelReaderBase {
public:
    /**
     * Loads the 3D model (anyone from the previous list) file pointed to by \p filename
     * and returns a constructed ModelGeometry from it. 
     *
     * \param filename The geometric model file to be loaded
     * \param forceRenderInvisible Force invisible meshes to render or not
     * \param notifyInvisibleDropped Notify in log if invisible meshses were dropped
     * \return The ModelGeometry containing the model
     *
     * \throw ModelReaderException If there was an error reading the model from
     *        \p filename
     * \pre \p filename must not be empty
     */
    std::unique_ptr<modelgeometry::ModelGeometry> loadModel(std::string& filename,
        bool forceRenderInvisible = false,
        bool notifyInvisibleDropped = true) const override;

    /**
     * Returns a list of all extensions that this ModelReaderAssimp supports.
     *
     * \return A list of all extensions that this ModelReaderAssimp supports
     */
    std::vector<std::string> supportedExtensions() const override;
};

} // namespace ghoul::io

#endif // __GHOUL___MODELREADERASSIMP___H__