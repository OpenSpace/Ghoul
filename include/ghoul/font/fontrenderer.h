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

#ifndef __GHOUL___FONTRENDERER___H__
#define __GHOUL___FONTRENDERER___H__

#include <ghoul/glm.h>
#include <ghoul/font/font.h>
#include <ghoul/opengl/uniformcache.h>

#include <memory>

namespace ghoul::opengl { class ProgramObject; }

namespace ghoul::fontrendering {

/**
 * The FontRenderer is a class that can take a Font object and renderer it at a given
 * position. It has two separate usage modes; 1. it works as a singleton that has a
 * predefined vertex and fragment shader that perform rendering in a predetermined way.
 * 2. Instances of the FontRenderer can be instatiatd that need a custom ProgramObject
 * that handles the rendering. As the rendering is performed in screen space, the
 * size of the rendering window in pixels has to be provided to the FontRenderer
 * (#setFramebufferSize) for a correct rendering.
 * The main methods for rendering are the #render methods and their varieties.
 */
class FontRenderer {
public:
    /**
     * Information about the bounding box for a rendered text.
     */
    struct BoundingBoxInformation {
        glm::vec2 boundingBox;
        int numberOfLines;
    };

    /**
     * This constructor requires a custom ProgramObject that handles the rendering of any
     * passed text. In addition the initial size of the rendering window has to be passed.
     * The inputs for the ProgramObject are as follows: <br>
     * The <b>vertex shader</b> recieves three <code>vec2</code> for each vertex: The
     * <i>Position</i> (location 0) in pixel screen space coordinates, the
     * <i>Base Texture Coordinates</i> (location 1) which provides the texture coordinates
     * for the base font layer, and the <i>Outline Texture Coordinates</i> (location 2)
     * which provides the texture coordinates for the outline font layer. Furhermore, the
     * following uniforms are provided: The <code>projection</code> (<code>mat4</code>)
     * contains the projection matrix that is derived using the provided window size and
     * maps the pixel coordinates to normalized device coordinates, the <code>tex</code>
     * (<code>sampler2D</code>) is the TextureAtlas that contains all glyphs and into
     * which the <i>Base Texture Coordinates</i> and <i>Outline Texture Coordinates</i>
     * index into. The <code>baseColor</code> (<code>vec4</code>) contains the
     * user-specified color for the base layer, whereas the <code>outlineColor</code>
     * (<code>vec4</code>) is the color for the outline layer. Finally, the
     * <code>hasOutline</code> (<code>bool</code>) is <code>true</code> whether the passed
     * font has an outline or not.
     * \param program The custom ProgramObject that is used to render any passed text.
     * \param framebufferSize The size of the framebuffer into which this FontRenderer
     * renders
     */
    FontRenderer(std::unique_ptr<opengl::ProgramObject> program,
        glm::vec2 framebufferSize);

    /// Default destructor that cleans used OpenGL names and the ProgramObject
    ~FontRenderer();

    /**
    * Creates a new instance of the FontRenderer with a default ProgramObject.
    * This method requires the FileSystem to be initialized, as temporary
    * files containing the source code of the ShaderObjects will be created.
    * This method requires a valid OpenGL state.
    * \return a raw pointer to the new default instance
    */
    static std::unique_ptr<FontRenderer> createDefault();

    /**
    * Creates a new instance of the FontRenderer with a perspective subject
    * ProgramObject.
    * This method requires the FileSystem to be initialized, as temporary
    * files containing the source code of the ShaderObjects will be created.
    * This method requires a valid OpenGL state.
    * \return a raw pointer to the new default instance
    */
    static std::unique_ptr<FontRenderer> createProjectionSubjectText();

    /**
     * Initializes the singleton variant of the FontRenderer with the default
     * ProgramObject. This method requires the FileSystem to be initialized, as temporary
     * files containing the source code of the ShaderObjects will be created. The method
     * returns the success of the compilation and linking of the default ProgramObject.
     * This method requires a valid OpenGL state. An assertion is triggered if the
     * singleton FontRenderer is initialized twice.
     * \return <code>true</code> if the initialization succeeded, <code>false</code>
     * otherwise
     */
    static bool initialize();

    /**
     * Deinitializes the singleton variant of the FontRenderer and cleans all used OpenGL
     * objects. Therefore, it requires a valid OpenGL state.
     * \return <code>true</code> if the deinitialization succeeded, <code>false</code>
     * otherwise
     */
    static bool deinitialize();

    /**
     * Checks whether the singleton variant of the FontRenderer has already been
     * initialized.
     * \return <code>true</code> if the singleton FontRenderer has already been
     * initialized; <code>false</code> otherwise
     */
    static bool isInitialized();

    /**
     * Returns the singleton FontRenderer. This method triggers an assertion if the
     * FontRenderer was not initialized before usage.
     * \return The singleton FontRenderer
     */
    static FontRenderer& defaultRenderer();

    /**
     * Returns the singleton FontRenderer that renders projected text. This method
     * triggers an assertion if the FontRenderer was not initialized before usage.
     * \return The singleton Fontrenderer rendering projected text.
     */
    static FontRenderer& defaultProjectionRenderer();

    /**
     * Sets the size of the framebuffer that is used as the target for this FontRenderer.
     * It is used to convert the pixel coordinates (used in the #render) method to
     * normalized device coordinates.
     * \param framebufferSize The size of the target framebuffer
     */
    void setFramebufferSize(glm::vec2 framebufferSize);

    /**
     * Returns the size in pixels that the text would require if it were rendered to
     * screen.
     * \param font The Font that is used to render the provided text
     * \param format The format text that is rendered to the screen. This text can contain
     * symbolic constants (the same as in printf) to refer to later variable arguments,
     * which are substituted. The <code>text</code> can also contain '\\n' to have a
     * linebreak, which is of the correct length with regard to the selected font. This
     * parameter cannot be a <code>nullptr</code>.
     * \return A tuple containing the bounding box of the text that was printed and the
     * number of lines that were printed
     */
    BoundingBoxInformation boundingBox(Font& font, const char* format, ...) const;

    /**
     * Renders the provided texts (<code>format</code> + variable arguments) to the pixel
     * coordinates <code>pos</code> using the Font <code>font</code> in the base color
     * <code>color</code> and the outline color <code>outlineColor</code>. If the Font
     * does not have an outline, the <code>outlineColor</code> is ignored.
     * \param font The Font that is used to render the provided text
     * \param pos The screen-space position (in pixel coordinates) for 2D text rendering,
     * or world-space coordinate for projection subject rendering, that is used to render
     * the text
     * \param color The base color that is used to render the text
     * \param outlineColor The outline color that is used to render the text if the Font
     * has an outline
     * \param format The format text that is rendered to the screen. This text can contain
     * symbolic constants (the same as in printf) to refer to later variable arguments,
     * which are substituted. The <code>text</code> can also contain '\\n' to have a
     * linebreak, which is of the correct length with regard to the selected font. This
     * parameter cannot be a <code>nullptr</code>.
     * \return A tuple containing the bounding box of the text that was printed and the
     * number of lines that were printed
     */
    BoundingBoxInformation render(Font& font, glm::vec2 pos, glm::vec4 color,
        glm::vec4 outlineColor, const char* format, ...) const;

    /**
    * Renders the provided texts (<code>format</code> + variable arguments) to the pixel
    * coordinates <code>pos</code> using the Font <code>font</code> in the base color
    * <code>color</code> and the outline color <code>outlineColor</code>. If the Font
    * does not have an outline, the <code>outlineColor</code> is ignored.
    * \param font The Font that is used to render the provided text
    * \param pos The world-space coordinate for projection subject rendering that
    * is used to render the text
    * \param color The base color that is used to render the text
    * \param outlineColor The outline color that is used to render the text if the Font
    * has an outline
    * \param textScale Scale applied on the rendered text.
    * \param textMinSize Minimal size (in pixels) for a text to be rendered.
    * \param textMaxSize Maximal size (in pixels) for a text to be rendered.
    * \param mvpMatrix ModelViewProjection matrix transformation.
    * \param orthonormalRight Right vector from the orthonormal basis defining the
    * text's plane.
    * \param orthonormalUp Up vector from the orthonormal basis defining the
    * text's plane.
    * \param format The format text that is rendered to the screen. This text can contain
    * symbolic constants (the same as in printf) to refer to later variable arguments,
    * which are substituted. The <code>text</code> can also contain '\\n' to have a
    * linebreak, which is of the correct length with regard to the selected font. This
    * parameter cannot be a <code>nullptr</code>.
    * \return A tuple containing the bounding box of the text that was printed and the
    * number of lines that were printed
    */
    BoundingBoxInformation render(Font& font, glm::vec3 pos, glm::vec4 color,
        glm::vec4 outlineColor, const float textScale, const int textMinSize,
        const int textMaxSize, const glm::dmat4& mvpMatrix,
        const glm::vec3& orthonormalRight, const glm::vec3& orthonormalUp,
        const glm::dvec3& cameraPos, const glm::dvec3& cameraLookUp,
        const int renderType, char* format, ...) const;

    /**
     * Renders the provided texts (<code>format</code> + variable arguments) to the pixel
     * coordinates <code>pos</code> using the Font <code>font</code> in the base color
     * <code>color</code>. In case the Font has an outline, the outline is rendered in
     * black with the same transparency as the provided <code>color</code>.
     * \param font The Font that is used to render the provided text
     * \param pos The screen-space position (in pixel coordinates) that is used to render
     * the text
     * \param color The base color that is used to render the text
     * \param format The format text that is rendered to the screen. This text can contain
     * symbolic constants (the same as in printf) to refer to later variable arguments,
     * which are substituted. The <code>text</code> can also contain '\\n' to have a
     * linebreak, which is of the correct length with regard to the selected font. This
     * parameter cannot be a <code>nullptr</code>.
     * \return A tuple containing the bounding box of the text that was printed and the
     * number of lines that were printed
     */
    BoundingBoxInformation render(Font& font, glm::vec2 pos, glm::vec4 color,
        const char* format, ...) const;

    /**
    * Renders the provided texts (<code>format</code> + variable arguments) to the pixel
    * coordinates <code>pos</code> using the Font <code>font</code> in the base color
    * <code>color</code>. In case the Font has an outline, the outline is rendered in
    * black with the same transparency as the provided <code>color</code>.
    * \param font The Font that is used to render the provided text
    * \param pos The world-space coordinate for projection subject rendering,
    * that is used to render the text
    * \param color The base color that is used to render the text
    * \param textScale Scale applied on the rendered text.
    * \param textMinSize Minimal size (in pixels) for a text to be rendered.
    * \param textMaxSize Maximal size (in pixels) for a text to be rendered.
    * \param mvpMatrix ModelViewProjection matrix transformation.
    * \param orthonormalRight Right vector from the orthonormal basis defining the
    * text's plane.
    * \param orthonormalUp Up vector from the orthonormal basis defining the
    * text's plane.
    * \param format The format text that is rendered to the screen. This text can contain
    * symbolic constants (the same as in printf) to refer to later variable arguments,
    * which are substituted. The <code>text</code> can also contain '\\n' to have a
    * linebreak, which is of the correct length with regard to the selected font. This
    * parameter cannot be a <code>nullptr</code>.
    * \return A tuple containing the bounding box of the text that was printed and the
    * number of lines that were printed
    */
    BoundingBoxInformation render(Font& font, glm::vec3 pos, glm::vec4 color,
        const float textScale, const int textMinSize, const int textMaxSize,
        const glm::dmat4& mvpMatrix, const glm::vec3& orthonormalRight,
        const glm::vec3& orthonormalUp, const glm::dvec3& cameraPos,
        const glm::dvec3& cameraLookUp, const int renderType,
        const char* format, ...) const;

    /**
     * Renders the provided texts (<code>format</code> + variable arguments) to the pixel
     * coordinates <code>pos</code> using the Font <code>font</code> in white color. In
     * case the Font has an outline, the outline is rendered in black.
     * \param font The Font that is used to render the provided text
     * \param pos The screen-space position (in pixel coordinates) that is used to render
     * the text
     * \param format The format text that is rendered to the screen. This text can contain
     * symbolic constants (the same as in printf) to refer to later variable arguments,
     * which are substituted. The <code>text</code> can also contain '\\n' to have a
     * linebreak, which is of the correct length with regard to the selected font. This
     * parameter cannot be a <code>nullptr</code>.
     * \return A tuple containing the bounding box of the text that was printed and the
     * number of lines that were printed
     */
    BoundingBoxInformation render(Font& font, glm::vec2 pos,
        const char* format, ...) const;

    /**
    * Renders the provided texts (<code>format</code> + variable arguments) to the pixel
    * coordinates <code>pos</code> using the Font <code>font</code> in white color. In
    * case the Font has an outline, the outline is rendered in black.
    * \param font The Font that is used to render the provided text
    * \param pos The world-space coordinate for projection subject rendering that is
    * used to render the text
    * \param textScale Scale applied on the rendered text.
    * \param textMinSize Minimal size (in pixels) for a text to be rendered.
    * \param textMaxSize Maximal size (in pixels) for a text to be rendered.
    * \param mvpMatrix ModelViewProjection matrix transformation.
    * \param orthonormalRight Right vector from the orthonormal basis defining the
    * text's plane.
    * \param orthonormalUp Up vector from the orthonormal basis defining the
    * text's plane.
    * \param format The format text that is rendered to the screen. This text can contain
    * symbolic constants (the same as in printf) to refer to later variable arguments,
    * which are substituted. The <code>text</code> can also contain '\\n' to have a
    * linebreak, which is of the correct length with regard to the selected font. This
    * parameter cannot be a <code>nullptr</code>.
    * \return A tuple containing the bounding box of the text that was printed and the
    * number of lines that were printed
    */
    BoundingBoxInformation render(Font& font, glm::vec3 pos, const float textScale,
        const int textMinSize, const int textMaxSize, const glm::dmat4& mvpMatrix,
        const glm::vec3& orthonormalRight, const glm::vec3& orthonormalUp,
        const glm::dvec3& cameraPos, const glm::dvec3& cameraLookUp,
        const int renderType, const char* format, ...) const;

private:
    /// Private constructor that is used in the #initialize static method
    FontRenderer();

    BoundingBoxInformation internalRender(Font& font, glm::vec2 pos, glm::vec4 color,
        glm::vec4 outlineColor, const char* buffer) const;

    BoundingBoxInformation internalProjectionRender(Font& font, glm::vec3 pos,
        glm::vec4 color, glm::vec4 outlineColor, const char* buffer,
        const float textScale, const int textMinSize, const int textMaxSize,
        const glm::dmat4& mvpMatrix, const glm::vec3& orthonormalRight,
        const glm::vec3& orthonormalUp, const glm::dvec3& cameraPos,
        const glm::dvec3& cameraLookUp, const int renderType) const;


    /// The singleton instance of the default FontRenderer
    static std::unique_ptr<FontRenderer> _defaultRenderer;

    /// The singleton instance of the default projection FontRenderer
    static std::unique_ptr<FontRenderer> _defaultProjectionRenderer;

    /// The framebuffer size that is used to compute the transformation from pixel
    /// coordinates to normalized device coordinates
    glm::vec2 _framebufferSize;

    /// The ProgramObject that is used to render the text
    std::unique_ptr<opengl::ProgramObject> _program;

    /// The vertex array object holding the other OpenGL objects
    unsigned int _vao;

    /// The vertex buffer object that contains the vertices for the text to be rendered
    unsigned int _vbo;

    /// The index buffer object that allows reusing vertices to form one quad per glyph
    unsigned int _ibo;

    UniformCache(baseColor, outlineColor, texture, hasOutline, projection, mvpMatrix
        /*textMinSize*/) _uniformCache;
};

/**
 * This helper method prints the passed arguments using the Font::render function of the
 * default font. It is equivalent to calling defaultRenderer::render with the same
 * arguments and discarding the second return value
 * \param font The Font that is used to render the provided text. If this argument is
 * <code>nullptr</code>, an assertion is thrown
 * \param pos The screen-space position (in pixel coordinates) that is used to render
 * the text
 * \param args The variable arguments that are the format string (the same as in printf)
 * and optional arguments afterwards.
 * \return The bounding box of the text that was printed
 */
template <typename... Args>
glm::vec2 RenderFont(ghoul::fontrendering::Font& font, glm::vec2 pos, Args... args) {
    return (ghoul::fontrendering::FontRenderer::defaultRenderer().render(
        font, pos, args...)).boundingBox;
}

/**
 * This helper method prints the passed arguments using the Font::render function of the
 * provided fontrenderer.
 * \oaram fontRenderer The FontRenderer that is to be used for the rendering of the font
 * \param font The Font that is used to render the provided text. If this argument is
 * <code>nullptr</code>, an assertion is thrown
 * \param pos The screen-space position (in pixel coordinates) that is used to render
 * the text
 * \param args The variable arguments that are the format string (the same as in printf)
 * and optional arguments afterwards.
 * \return The bounding box of the text that was printed
 */
template <typename... Args>
glm::vec2 RenderFont(ghoul::fontrendering::FontRenderer& renderer,
    ghoul::fontrendering::Font& font, glm::vec2 pos, Args... args)
{
    return (renderer.render(font, pos, args...)).boundingBox;
}

/**
 * This helper method prints the passed arguments using the Font::render function of the
 * default font and moves the pen position downwards after the call. It is equivalent to
 * calling defaultRenderer::render with the same arguments and then subtracting the
 * returned height from the pen position and discarding the second return value
 * \param font The Font that is used to render the provided text. If this argument is
 * <code>nullptr</code>, an assertion is thrown
 * \param pos The screen-space position (in pixel coordinates) that is used to render
 * the text
 * \param args The variable arguments that are the format string (the same as in printf)
 * and optional arguments afterwards.
 * \return The bounding box of the text that was printed
 */
template <typename... Args>
glm::vec2 RenderFontCr(ghoul::fontrendering::Font& font, glm::vec2& pos, Args... args) {
    ghoul::fontrendering::FontRenderer::BoundingBoxInformation res =
        ghoul::fontrendering::FontRenderer::defaultRenderer().render(font, pos, args...);

    pos.y -= res.numberOfLines * font.height();
    return res.boundingBox;
}

/**
 * This helper method prints the passed arguments using the Font::render function of the
 * provided fontrenderer and moves the pen position downwards after the call. It is
 * equivalent to calling the renderer's render with the same arguments and then
 * subtracting the returned height from the pen position and discarding the second return
 * value
 * \oaram fontRenderer The FontRenderer that is to be used for the rendering of the font
 * \param font The Font that is used to render the provided text. If this argument is
 * <code>nullptr</code>, an assertion is thrown
 * \param pos The screen-space position (in pixel coordinates) that is used to render
 * the text
 * \param args The variable arguments that are the format string (the same as in printf)
 * and optional arguments afterwards.
 * \return The bounding box of the text that was printed
 */
template <typename... Args>
glm::vec2 RenderFontCr(ghoul::fontrendering::FontRenderer& renderer,
    ghoul::fontrendering::Font& font, glm::vec2& pos, Args... args)
{
    ghoul::fontrendering::FontRenderer::BoundingBoxInformation res = renderer.render(
        font,
        pos,
        args...
    );
    pos.y -= res.numberOfLines * font.height();
    return res.boundingBox;
}

/**
 * This helper method prints the passed arguments using the Font::render function of the
 * default font and moves the pen position upwards after the call. It is equivalent to
 * calling defaultRenderer::render with the same arguments and then adding the
 * returned height to the pen position and discarding the second return value
 * \param font The Font that is used to render the provided text. If this argument is
 * <code>nullptr</code>, an assertion is thrown
 * \param pos The screen-space position (in pixel coordinates) that is used to render
 * the text
 * \param args The variable arguments that are the format string (the same as in printf)
 * and optional arguments afterwards.
 * \return The bounding box of the text that was printed
 */
template <typename... Args>
glm::vec2 RenderFontCrUp(ghoul::fontrendering::Font& font, glm::vec2& pos, Args... args) {
    ghoul::fontrendering::FontRenderer::BoundingBoxInformation res =
        ghoul::fontrendering::FontRenderer::defaultRenderer().render(font, pos, args...);

    pos.y += res.numberOfLines * font.height();
    return res.boundingBox;
}

/**
 * This helper method prints the passed arguments using the Font::render function of the
 * provided fontrenderer and moves the pen position upwards after the call. It is
 * equivalent to calling the renderer's render with the same arguments and then adding
 * the returned height to the pen position and discarding the second return value
 * \oaram fontRenderer The FontRenderer that is to be used for the rendering of the font
 * \param font The Font that is used to render the provided text. If this argument is
 * <code>nullptr</code>, an assertion is thrown
 * \param pos The screen-space position (in pixel coordinates) that is used to render
 * the text
 * \param args The variable arguments that are the format string (the same as in printf)
 * and optional arguments afterwards.
 * \return The bounding box of the text that was printed
 */
template <typename... Args>
glm::vec2 RenderFontCrUp(ghoul::fontrendering::FontRenderer& renderer,
    ghoul::fontrendering::Font& font, glm::vec2& pos, Args... args)
{
    ghoul::fontrendering::FontRenderer::BoundingBoxInformation res = renderer.render(
        font,
        pos,
        args...
    );
    pos.y += res.numberOfLines * font.height();
    return res.boundingBox;
}

} // namespace ghoul::fontrendering

#endif // __GHOUL___FONTRENDERER___H__
