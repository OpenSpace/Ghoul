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

#include <ghoul/font/fontrenderer.h>

#include <ghoul/fmt.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/font/font.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/misc.h>
#include <ghoul/opengl/programobject.h>
#include <ghoul/opengl/texture.h>
#include <ghoul/opengl/textureatlas.h>
#include <ghoul/opengl/textureunit.h>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <numeric>

namespace {
    constexpr const char* _loggerCat = "FontRenderer";

    constexpr const char* DefaultVertexShaderPath =
        "${TEMPORARY}/defaultfontrenderer_vs.glsl";
    constexpr const char* DefaultFragmentShaderPath =
        "${TEMPORARY}/defaultfontrenderer_fs.glsl";
    constexpr const char* ProjectionVertexShaderPath =
        "${TEMPORARY}/projectionfontrenderer_vs.glsl";
    constexpr const char* ProjectionFragmentShaderPath =
        "${TEMPORARY}/projectionfontrenderer_fs.glsl";

    constexpr const char* DefaultVertexShaderSource = "\
    #version __CONTEXT__ \n\
    \n\
    layout (location = 0) in vec2 in_position; \n\
    layout (location = 1) in vec2 in_texCoords; \n\
    layout (location = 2) in vec2 in_outlineTexCoords; \n\
    \n\
    layout (location = 0) out vec2 texCoords; \n\
    layout (location = 1) out vec2 outlineTexCoords; \n\
    \n\
    uniform mat4 projection; \n\
    \n\
    void main() { \n\
        texCoords = in_texCoords; \n\
        outlineTexCoords = in_outlineTexCoords; \n\
        gl_Position = projection * vec4(in_position, 0.0, 1.0); \n\
    } \n\
    ";

    constexpr const char* DefaultFragmentShaderSource = "\
    #version __CONTEXT__ \n\
    \n\
    layout (location = 0) in vec2 texCoords; \n\
    layout (location = 1) in vec2 outlineTexCoords; \n\
    \n\
    out vec4 FragColor; \n\
    \n\
    uniform sampler2D tex; \n\
    uniform vec4 baseColor; \n\
    uniform vec4 outlineColor; \n\
    uniform bool hasOutline; \n\
    \n\
    void main() { \n\
        if (hasOutline) { \n\
            float inside = texture(tex, texCoords).r;\n\
            float outline = texture(tex, outlineTexCoords).r;\n\
            vec4 blend = mix(outlineColor, baseColor, inside);\n\
            FragColor = blend * vec4(1.0, 1.0, 1.0, max(inside, outline));\n\
        } \n\
        else { \n\
            FragColor = vec4(baseColor.rgb, baseColor.a * texture(tex, texCoords).r); \n\
        } \n\
    }";

    constexpr const char* ProjectionVertexShaderSource = "\
    #version __CONTEXT__ \n\
    \n\
    layout (location = 0) in vec3 in_position; \n\
    layout (location = 1) in vec2 in_texCoords; \n\
    layout (location = 2) in vec2 in_outlineTexCoords; \n\
    \n\
    out vec2 texCoords; \n\
    out vec2 outlineTexCoords; \n\
    \n\
    uniform dmat4 mvpMatrix; \n\
    \n\
    out float depth; \n\
    void main() { \n\
        texCoords = in_texCoords; \n\
        outlineTexCoords = in_outlineTexCoords; \n\
        vec4 finalPos = vec4(mvpMatrix * dvec4(in_position.xyz, 1.0)); \n\
        depth = finalPos.w; \n\
        finalPos.z = 0.0; \n\
        gl_Position = finalPos; \n\
    } \n\
    ";

    constexpr const char* ProjectionFragmentShaderSource = "\
    #version __CONTEXT__ \n\
    \n\
    in vec2 texCoords; \n\
    in vec2 outlineTexCoords; \n\
    in float depth; \n\
    \n\
    out vec4 FragColor; \n\
    \n\
    uniform sampler2D tex; \n\
    uniform vec4 baseColor; \n\
    uniform vec4 outlineColor; \n\
    uniform bool hasOutline; \n\
    \n\
    void main() { \n\
        if (hasOutline) { \n\
            float inside = texture(tex, texCoords).r;\n\
            float outline = texture(tex, outlineTexCoords).r;\n\
            vec4 blend = mix(outlineColor, baseColor, inside);\n\
            FragColor = blend * vec4(1.0, 1.0, 1.0, max(inside, outline));\n\
        } \n\
        else { \n\
            FragColor = vec4(baseColor.rgb, baseColor.a * texture(tex, texCoords).r); \n\
        } \n\
        if (depth > 1.0) { \n\
            gl_FragDepth = depth / pow(10, 30);\n\
        } \n\
        else { \n\
            gl_FragDepth = depth - 1.0; \n\
        } \n\
    }";
} // namespace

namespace ghoul::fontrendering {

std::unique_ptr<FontRenderer> FontRenderer::_defaultRenderer = nullptr;
std::unique_ptr<FontRenderer> FontRenderer::_defaultProjectionRenderer = nullptr;

FontRenderer::FontRenderer() {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ibo);
}

FontRenderer::FontRenderer(std::unique_ptr<opengl::ProgramObject> program,
                           glm::vec2 framebufferSize)
    : FontRenderer()
{
    ghoul_assert(program != nullptr, "No program provided");

    _program = std::move(program);
    setFramebufferSize(std::move(framebufferSize));
}

FontRenderer::~FontRenderer() {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ibo);
}

std::unique_ptr<FontRenderer> FontRenderer::createDefault() {
    std::string vsPath = absPath(DefaultVertexShaderPath);
    LDEBUG(fmt::format("Writing default vertex shader to '{}'", vsPath));
    std::ofstream file(vsPath);
    file << DefaultVertexShaderSource;
    file.close();

    std::string fsPath = absPath(DefaultFragmentShaderPath);
    LDEBUG(fmt::format("Writing default fragment shader to '{}'", fsPath));
    file.open(fsPath);
    file << DefaultFragmentShaderSource;
    file.close();

    using namespace opengl;
    std::unique_ptr<ProgramObject> program = std::make_unique<ProgramObject>("Font");
    program->attachObject(
        std::make_unique<ShaderObject>(ShaderObject::ShaderType::Vertex, vsPath)
    );
    program->attachObject(
        std::make_unique<ShaderObject>(ShaderObject::ShaderType::Fragment, fsPath)
    );

    LDEBUG("Compile default font shader");
    program->compileShaderObjects();

    LDEBUG("Link default font shader");
    program->linkProgramObject();

    FontRenderer* fr = new FontRenderer;
    fr->_program = std::move(program);

    fr->_uniformCache.baseColor = fr->_program->uniformLocation("baseColor");
    fr->_uniformCache.outlineColor = fr->_program->uniformLocation("outlineColor");
    fr->_uniformCache.texture = fr->_program->uniformLocation("tex");
    fr->_uniformCache.hasOutline = fr->_program->uniformLocation("hasOutline");
    fr->_uniformCache.projection = fr->_program->uniformLocation("projection");

    return std::unique_ptr<FontRenderer>(fr);
}

std::unique_ptr<FontRenderer> FontRenderer::createProjectionSubjectText() {
    std::string vsPath = absPath(ProjectionVertexShaderPath);
    LDEBUG(fmt::format("Writing default vertex shader to '{}'", vsPath));
    std::ofstream file(vsPath);
    file << ProjectionVertexShaderSource;
    file.close();

    std::string fsPath = absPath(ProjectionFragmentShaderPath);
    LDEBUG(fmt::format("Writing default fragment shader to '{}'", fsPath));
    file.open(fsPath);
    file << ProjectionFragmentShaderSource;
    file.close();

    using namespace opengl;
    std::unique_ptr<ProgramObject> prog = std::make_unique<ProgramObject>(
        "ProjectionFont"
    );
    prog->attachObject(
        std::make_unique<ShaderObject>(ShaderObject::ShaderType::Vertex, vsPath)
    );
    prog->attachObject(
        std::make_unique<ShaderObject>(ShaderObject::ShaderType::Fragment, fsPath)
    );

    LDEBUG("Compile projection font shader");
    prog->compileShaderObjects();

    LDEBUG("Link projection font shader");
    prog->linkProgramObject();

    FontRenderer* fr = new FontRenderer;
    fr->_program = std::move(prog);

    fr->_uniformCache.baseColor = fr->_program->uniformLocation("baseColor");
    fr->_uniformCache.outlineColor = fr->_program->uniformLocation("outlineColor");
    fr->_uniformCache.texture = fr->_program->uniformLocation("tex");
    fr->_uniformCache.hasOutline = fr->_program->uniformLocation("hasOutline");
    fr->_uniformCache.mvpMatrix = fr->_program->uniformLocation("mvpMatrix");
    //fr->_uniformCache.textMinSize = fr->_program->uniformLocation("textMinSize");

    return std::unique_ptr<FontRenderer>(fr);
}

bool FontRenderer::initialize() {
    LDEBUG("Creating default FontRenderer");
    ghoul_assert(!_defaultRenderer, "Default FontRenderer was already initialized");
    _defaultRenderer = createDefault();

    LDEBUG("Creating default projection FontRenderer");
    ghoul_assert(
        !_defaultProjectionRenderer,
        "Default projection Fontrenderer was already initialized"
    );
    _defaultProjectionRenderer = createProjectionSubjectText();

    return true;
}

bool FontRenderer::deinitialize() {
    _defaultRenderer = nullptr;
    _defaultProjectionRenderer = nullptr;
    return true;
}

bool FontRenderer::isInitialized() {
    return _defaultRenderer != nullptr;
}

FontRenderer& FontRenderer::defaultRenderer() {
    ghoul_assert(_defaultRenderer != nullptr, "FontRenderer was not initialized");
    return *_defaultRenderer;
}

FontRenderer& FontRenderer::defaultProjectionRenderer() {
    ghoul_assert(
        _defaultProjectionRenderer != nullptr,
        "Projection FontRenderer was not initialized"
    );
    return *_defaultProjectionRenderer;
}

FontRenderer::BoundingBoxInformation FontRenderer::boundingBox(Font& font,
                                                            const std::string& text) const
{
    const std::vector<std::string>& lines = ghoul::tokenizeString(text, '\n');

    float h = font.height();

    //unsigned int vertexIndex = 0;
    std::vector<GLuint> indices;
    glm::vec2 movingPos = glm::vec2(0.f);

    glm::vec2 size = glm::vec2(0.f);
    for (const std::string& line : lines) {
        movingPos.x = 0.f;
        float width = 0.f;
        float height = 0.f;
        for (size_t j = 0; j < line.size(); ++j) {
            wchar_t character = line[j];
            if (character == wchar_t('\t'))
                character = wchar_t(' ');
            const Font::Glyph* glyph;

            try {
                glyph = font.glyph(character);
            }
            catch (const Font::FontException&) {
                glyph = font.glyph(wchar_t(' '));
            }

            width += glyph->horizontalAdvance();
            height = std::max(height, static_cast<float>(glyph->height()));
        }
        size.x = std::max(size.x, width);
        size.y += height;
        movingPos.y -= h;
    }
    size.y = lines.size() * font.height();

    return { size, static_cast<int>(lines.size()) };
}

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          const glm::vec2& pos,
                                                          const std::string& text,
                                                          const glm::vec4& color) const
{
    return render(font, pos, text, color, { 0.f, 0.f, 0.f, color.a });
}

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          const glm::vec2& pos,
                                                          const std::string& text,
                                                          const glm::vec4& color,
                                                      const glm::vec4& outlineColor) const
{
    const std::vector<std::string>& lines = ghoul::tokenizeString(text, '\n');

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _program->activate();

    struct Vertex {
        float x;
        float y;
        float s;
        float t;
        float outlineS;
        float outlineT;
    };

    int nCharacters = std::accumulate(
        lines.begin(),
        lines.end(),
        0,
        [](int lhs, const std::string& rhs) { return static_cast<int>(rhs.size()) + lhs; }
    );
    std::vector<Vertex> vertices;
    vertices.reserve(nCharacters * 4); // each character is four vertices

    std::vector<GLushort> indices;
    indices.reserve(nCharacters * 3); // each character is two triangles


    GLushort vertexIndex = 0;
    glm::vec2 size = glm::vec2(0.f);
    glm::vec2 movingPos = pos;
    for (const std::string& line : lines) {
        movingPos.x = pos.x;
        float width = 0.f;
        float height = 0.f;
        for (size_t j = 0; j < line.size(); ++j) {
            wchar_t character = line[j];
            if (character == wchar_t('\t')) {
                character = wchar_t(' ');
            }

            const Font::Glyph* glyph;
            try {
                glyph = font.glyph(character);
            }
            catch (const Font::FontException&) {
                glyph = font.glyph(wchar_t(' '));
            }

            if (j > 0) {
                movingPos.x += glyph->kerning(line[j - 1]);
            }

            const float x0 = movingPos.x + glyph->leftBearing();
            const float y0 = movingPos.y + glyph->topBearing();
            const float s0 = glyph->topLeft().x;
            const float t0 = glyph->topLeft().y;
            const float outlineS0 = glyph->outlineTopLeft().x;
            const float outlineT0 = glyph->outlineTopLeft().y;

            const float x1 = x0 + glyph->width();
            const float y1 = y0 - glyph->height();
            const float s1 = glyph->bottomRight().x;
            const float t1 = glyph->bottomRight().y;
            const float outlineS1 = glyph->outlineBottomRight().x;
            const float outlineT1 = glyph->outlineBottomRight().y;

            indices.insert(indices.end(), {
                vertexIndex, vertexIndex + 1u, vertexIndex + 2u,
                vertexIndex, vertexIndex + 2u, vertexIndex + 3u
            });
            vertexIndex += 4;
            vertices.insert(vertices.end(), {
                { x0, y0, s0, t0, outlineS0, outlineT0 },
                { x0, y1, s0, t1, outlineS0, outlineT1 },
                { x1, y1, s1, t1, outlineS1, outlineT1 },
                { x1, y0, s1, t0, outlineS1, outlineT0 }
            });
            movingPos.x += glyph->horizontalAdvance();

            width += glyph->horizontalAdvance();
            height = std::max(height, static_cast<float>(glyph->height()));
        }
        size.x = std::max(size.x, width);
        //size.y += height;
        movingPos.y -= font.height();
    }
    size.y = (lines.size() - 1) * font.height();

    opengl::TextureUnit atlasUnit;
    atlasUnit.activate();
    font.atlas().texture().bind();

    _program->setUniform(_uniformCache.baseColor, color);
    _program->setUniform(_uniformCache.outlineColor, outlineColor);
    _program->setUniform(_uniformCache.texture, atlasUnit);
    _program->setUniform(_uniformCache.hasOutline, font.hasOutline());
    _program->setUniform(
        _uniformCache.projection,
        glm::ortho(0.f, _framebufferSize.x, 0.f, _framebufferSize.y)
    );

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(GLushort),
        indices.data(),
        GL_DYNAMIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, s))
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, outlineS))
    );

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(indices.size()),
        GL_UNSIGNED_SHORT,
        nullptr
    );

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    return { size, static_cast<int>(lines.size()) };
}

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          const glm::vec3& pos,
                                                          const std::string& text,
                                                          const glm::vec4& color,
                                                          const glm::vec4& outlineColor,
                                                          float textScale,
                                                          int textMinSize,
                                                          int textMaxSize,
                                                          const glm::dmat4& mvpMatrix,
                                                        const glm::vec3& orthonormalRight,
                                                         const glm::vec3& orthonormalUp,
                                                         const glm::dvec3& cameraPos,
                                                         const glm::dvec3& cameraLookUp,
                                                         int renderType) const
{
    float h = font.height();

    const std::vector<std::string>& lines = ghoul::tokenizeString(text, '\n');

    unsigned int vertexIndex = 0;
    std::vector<GLuint> indices;
    std::vector<GLfloat> vertices;
    //glm::vec3 movingPos = pos;
    // TODO: review y starting position
    glm::vec2 movingPos(0.f);

    glm::vec2 size = glm::vec2(0.f);
    float heightInPixels = 0.0f;

    for (const std::string& line : lines) {
        movingPos.x = 0.f;
        //movingPos.x = pos.x;
        float width = 0.f;
        float height = 0.f;
        for (size_t j = 0; j < line.size(); ++j) {
            wchar_t character = line[j];
            if (character == wchar_t('\t')) {
                character = wchar_t(' ');
            }

            const Font::Glyph* glyph;
            try {
                glyph = font.glyph(character);
            }
            catch (const Font::FontException&) {
                glyph = font.glyph(wchar_t(' '));
            }

            if (j > 0) {
                movingPos.x += glyph->kerning(line[j - 1]);
            }

            float x0 = movingPos.x + glyph->leftBearing();
            float y0 = movingPos.y + glyph->topBearing();
            float s0 = glyph->topLeft().x;
            float t0 = glyph->topLeft().y;
            float outlineS0 = glyph->outlineTopLeft().x;
            float outlineT0 = glyph->outlineTopLeft().y;

            float x1 = x0 + glyph->width();
            float y1 = y0 - glyph->height();
            float s1 = glyph->bottomRight().x;
            float t1 = glyph->bottomRight().y;
            float outlineS1 = glyph->outlineBottomRight().x;
            float outlineT1 = glyph->outlineBottomRight().y;

            glm::vec3 p0 = glm::vec3(0.0);
            glm::vec3 p1 = glm::vec3(0.0);
            glm::vec3 p2 = glm::vec3(0.0);
            glm::vec3 p3 = glm::vec3(0.0);

            if (renderType == 0) {
                p0 = (x0 * orthonormalRight + y0 * orthonormalUp) * textScale + pos;
                p1 = (x0 * orthonormalRight + y1 * orthonormalUp) * textScale + pos;
                p2 = (x1 * orthonormalRight + y1 * orthonormalUp) * textScale + pos;
                p3 = (x1 * orthonormalRight + y0 * orthonormalUp) * textScale + pos;
            }
            else {
                glm::dvec3 normal = glm::normalize(cameraPos - glm::dvec3(pos));
                glm::vec3 newRight = glm::vec3(glm::cross(cameraLookUp, normal));
                glm::vec3 newUp = glm::vec3(glm::cross(normal, glm::dvec3(newRight)));

                p0 = (x0 * newRight + y0 * newUp) * textScale + pos;
                p1 = (x0 * newRight + y1 * newUp) * textScale + pos;
                p2 = (x1 * newRight + y1 * newUp) * textScale + pos;
                p3 = (x1 * newRight + y0 * newUp) * textScale + pos;
            }


            glm::vec4 projPos[2];
            projPos[0] = glm::vec4(mvpMatrix * glm::dvec4(p0, 1.0));
            projPos[1] = glm::vec4(mvpMatrix * glm::dvec4(p1, 1.0));
            glm::vec4 topLeft =
                (((projPos[0] / projPos[0].w) + glm::vec4(1.0)) / glm::vec4(2.0)) *
                glm::vec4(_framebufferSize.x, _framebufferSize.y, 1.0, 1.0);
            glm::vec4 bottomLeft =
                (((projPos[1] / projPos[1].w) + glm::vec4(1.0)) / glm::vec4(2.0)) *
                glm::vec4(_framebufferSize.x, _framebufferSize.y, 1.0, 1.0);

            // The billboard is bigger than the maximum size allowed:
            heightInPixels =
                heightInPixels == 0.0f ?
                glm::length(topLeft - bottomLeft) :
                heightInPixels;

            // Size-based culling
            if (heightInPixels < static_cast<float>(textMinSize) ||
                heightInPixels > _framebufferSize.x ||
                heightInPixels > _framebufferSize.y)
            {
                return { size, static_cast<int>(lines.size()) };
            }

            if (heightInPixels > textMaxSize) {
                float scaleFix = static_cast<float>(textMaxSize) / heightInPixels;
                if (renderType == 0) {
                    p0 = (x0 * orthonormalRight + y0 * orthonormalUp) *
                        textScale * scaleFix + pos;
                    p1 = (x0 * orthonormalRight + y1 * orthonormalUp) *
                        textScale * scaleFix + pos;
                    p2 = (x1 * orthonormalRight + y1 * orthonormalUp) *
                        textScale * scaleFix + pos;
                    p3 = (x1 * orthonormalRight + y0 * orthonormalUp) *
                        textScale * scaleFix + pos;
                }
                else {
                    glm::dvec3 normal = glm::normalize(cameraPos - glm::dvec3(pos));
                    glm::vec3 newRight = glm::vec3(glm::cross(cameraLookUp, normal));
                    glm::vec3 newUp = glm::vec3(glm::cross(normal, glm::dvec3(newRight)));

                    p0 = (x0 * newRight + y0 * newUp) * textScale * scaleFix + pos;
                    p1 = (x0 * newRight + y1 * newUp) * textScale * scaleFix + pos;
                    p2 = (x1 * newRight + y1 * newUp) * textScale * scaleFix + pos;
                    p3 = (x1 * newRight + y0 * newUp) * textScale * scaleFix + pos;
                }
            }

            vertices.insert(vertices.end(), {
                p0.x, p0.y, p0.z, s0, t0, outlineS0, outlineT0,
                p1.x, p1.y, p1.z, s0, t1, outlineS0, outlineT1,
                p2.x, p2.y, p2.z, s1, t1, outlineS1, outlineT1,
                p3.x, p3.y, p3.z, s1, t0, outlineS1, outlineT0
            });

            indices.insert(indices.end(), {
                vertexIndex, vertexIndex + 1, vertexIndex + 2,
                vertexIndex, vertexIndex + 2, vertexIndex + 3
            });
            vertexIndex += 4;

            movingPos.x += glyph->horizontalAdvance();

            width += glyph->horizontalAdvance();
            height = std::max(height, static_cast<float>(glyph->height()));
        } // end of a line

        size.x = std::max(size.x, width);
        size.y += height;
        movingPos.y -= h;
    }
    size.y = (lines.size() - 1) * font.height();

    /*glm::mat4 projection = glm::ortho(
    0.f,
    _framebufferSize.x,
    0.f,
    _framebufferSize.y
    );*/

    glDisable(GL_DEPTH_TEST);
    glEnablei(GL_BLEND, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _program->activate();

    opengl::TextureUnit atlasUnit;
    atlasUnit.activate();
    font.atlas().texture().bind();

    _program->setUniform(_uniformCache.baseColor, color);
    _program->setUniform(_uniformCache.outlineColor, outlineColor);
    _program->setUniform(_uniformCache.texture, atlasUnit);
    _program->setUniform(_uniformCache.hasOutline, font.hasOutline());
    _program->setUniform(_uniformCache.mvpMatrix, mvpMatrix);
    //_program->setUniform(_uniformCache.textMinSize, textMinSize);

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(GLuint),
        indices.data(),
        GL_DYNAMIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        7 * sizeof(float), reinterpret_cast<const void*>(3 * sizeof(float))
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE,
        7 * sizeof(float), reinterpret_cast<const void*>(5 * sizeof(float))
    );

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(indices.size()),
        GL_UNSIGNED_INT,
        nullptr
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);

    return { size, static_cast<int>(lines.size()) };
}

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          const glm::vec3& pos,
                                                          const std::string& text,
                                                          const glm::vec4& color,
                                                          float textScale,
                                                          int textMinSize,
                                                          int textMaxSize,
                                                          const glm::dmat4& mvpMatrix,
                                                        const glm::vec3& orthonormalRight,
                                                          const glm::vec3& orthonormalUp,
                                                          const glm::dvec3& cameraPos,
                                                          const glm::dvec3& cameraLookUp,
                                                          int renderType) const
{
    return render(
        font,
        pos,
        text,
        color,
        { 0.f, 0.f, 0.f, color.a },
        textScale,
        textMinSize,
        textMaxSize,
        mvpMatrix,
        orthonormalRight,
        orthonormalUp,
        cameraPos,
        cameraLookUp,
        renderType
    );
}

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          const glm::vec3& pos,
                                                          const std::string& text,
                                                          float textScale,
                                                          int textMinSize,
                                                          int textMaxSize,
                                                          const glm::dmat4& mvpMatrix,
                                                        const glm::vec3& orthonormalRight,
                                                          const glm::vec3& orthonormalUp,
                                                          const glm::dvec3& cameraPos,
                                                          const glm::dvec3& cameraLookUp,
                                                          int renderType) const
{
    return render(
        font,
        pos,
        text,
        { 1.f, 1.f, 1.f, 1.f },
        { 0.f, 0.f, 0.f, 1.f },
        textScale,
        textMinSize,
        textMaxSize,
        mvpMatrix,
        orthonormalRight,
        orthonormalUp,
        cameraPos,
        cameraLookUp,
        renderType
    );
}

void FontRenderer::setFramebufferSize(glm::vec2 framebufferSize) {
    _framebufferSize = std::move(framebufferSize);
}

} // namespace ghoul::fontrendering
