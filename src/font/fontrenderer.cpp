/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2020                                                               *
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
#include <ghoul/misc/profiling.h>
#include <ghoul/opengl/programobject.h>
#include <ghoul/opengl/texture.h>
#include <ghoul/opengl/textureatlas.h>
#include <ghoul/opengl/textureunit.h>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <numeric>

namespace {
    constexpr const char* _loggerCat = "FontRenderer";

    constexpr const std::array<const char*, 5> UniformNames = {
        "baseColor", "outlineColor", "tex", "hasOutline", "projection"
    };

    constexpr const std::array<const char*, 7> UniformNamesProjection = {
        "baseColor", "outlineColor", "tex", "hasOutline", "modelViewTransform",
        "enableFalseDepth", "disableTransmittance"
    };

    constexpr const char* DefaultVertexShaderPath =
        "${TEMPORARY}/defaultfontrenderer_vs.glsl";
    constexpr const char* DefaultFragmentShaderPath =
        "${TEMPORARY}/defaultfontrenderer_fs.glsl";
    constexpr const char* ProjectionVertexShaderPath =
        "${TEMPORARY}/projectionfontrenderer_vs.glsl";
    constexpr const char* ProjectionFragmentShaderPath =
        "${TEMPORARY}/projectionfontrenderer_fs.glsl";

    constexpr const char* DefaultVertexShaderSource = R"(
    #version __CONTEXT__

    layout (location = 0) in vec2 in_position;
    layout (location = 1) in vec2 in_texCoords;
    layout (location = 2) in vec2 in_outlineTexCoords;

    out vec2 texCoords;
    out vec2 outlineTexCoords;

    uniform mat4 projection;

    void main() {
        texCoords = in_texCoords;
        outlineTexCoords = in_outlineTexCoords;
        gl_Position = projection * vec4(in_position, 0.0, 1.0);
    })";

    constexpr const char* DefaultFragmentShaderSource = R"(
    #version __CONTEXT__

    in vec2 texCoords;
    in vec2 outlineTexCoords;

    out vec4 FragColor;

    uniform sampler2D tex;
    uniform vec4 baseColor;
    uniform vec4 outlineColor;
    uniform bool hasOutline;

    void main() {
        if (hasOutline) {
            float inside = texture(tex, texCoords).r;
            float outline = texture(tex, outlineTexCoords).r;
            vec4 blend = mix(outlineColor, baseColor, inside);
            FragColor = blend * vec4(1.0, 1.0, 1.0, max(inside, outline));
        }
        else {
            FragColor = vec4(baseColor.rgb, baseColor.a * texture(tex, texCoords).r);
        }
    })";

    constexpr const char* ProjectionVertexShaderSource = R"(
    #version __CONTEXT__

    layout (location = 0) in vec3 in_position;
    layout (location = 1) in vec2 in_texCoords;
    layout (location = 2) in vec2 in_outlineTexCoords;

    out vec2 texCoords;
    out vec2 outlineTexCoords;

    uniform dmat4 mvpMatrix;
    uniform dmat4 modelViewTransform;

    out float depth;
    out vec3 vsPosition;
    void main() {
        texCoords = in_texCoords;
        outlineTexCoords = in_outlineTexCoords;
        vec4 finalPos = vec4(mvpMatrix * dvec4(in_position.xyz, 1.0));
        depth = finalPos.w;
        finalPos.z = 0.0;
        vsPosition = vec3(modelViewTransform * dvec4(in_position.xyz, 1.0));
        gl_Position = finalPos;
    })";

    constexpr const char* ProjectionFragmentShaderSource = R"(
    #version __CONTEXT__

    in vec2 texCoords;
    in vec2 outlineTexCoords;
    in float depth;
    in vec3 vsPosition;

    out vec4 FragColor;
    out vec4 gPosition;
    out vec4 gNormal;

    uniform sampler2D tex;
    uniform vec4 baseColor;
    uniform vec4 outlineColor;
    uniform bool hasOutline;
    uniform bool enableFalseDepth;
    uniform bool disableTransmittance;

    void main() {
        if (hasOutline) {
            float inside = texture(tex, texCoords).r;
            float outline = texture(tex, outlineTexCoords).r;
            vec4 blend = mix(outlineColor, baseColor, inside);
            FragColor = blend * vec4(1.0, 1.0, 1.0, max(inside, outline));
        }
        else {
            FragColor = vec4(baseColor.rgb, baseColor.a * texture(tex, texCoords).r);
        }
        if (FragColor.a < 0.1) {
            discard;
        }
        if (enableFalseDepth) {
            gl_FragDepth = 0.0;
        }
        else {
            if (depth > 1.0) {
                gl_FragDepth = depth / pow(10, 30);
            }
            else {
                gl_FragDepth = depth - 1.0;
            }
        }
        if (disableTransmittance) {
            gPosition = vec4(0.0, 0.0, -1.0, 1.0);
        }
        else {
            gPosition = vec4(vsPosition, 1.0);
        }
        // 4th coord of the gNormal is the water reflectance
        gNormal = vec4(0.0, 0.0, 1.0, 0.0);
    })";

    // Extracts the next line from the string view and returns it, the passed string_view
    // is modified to remove the new line *and* the \n character
    std::string_view extractLine(std::string_view& view) {
        std::string_view::size_type p = view.find('\n');
        if (p == std::string_view::npos) {
            // No new line found
            std::string_view res = view;
            view = std::string_view();
            return res;
        }

        std::string_view res = view.substr(0, p);
        view = view.substr(p + 2);
        return res;
    }

} // namespace

namespace ghoul::fontrendering {

std::unique_ptr<FontRenderer> FontRenderer::_defaultRenderer = nullptr;
std::unique_ptr<FontRenderer> FontRenderer::_defaultProjectionRenderer = nullptr;

FontRenderer::FontRenderer() {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ibo);

    _vertexBuffer.reserve(128 * 4);
    _indexBuffer.reserve(128 * 4);
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
    if (FileSys.fileExists(vsPath)) {
        LDEBUG(fmt::format("Skipping creation of existing vertex shader {}", vsPath));
    }
    else {
        LDEBUG(fmt::format("Writing default vertex shader to '{}'", vsPath));
        std::ofstream file(vsPath);
        file << DefaultVertexShaderSource;
    }

    std::string fsPath = absPath(DefaultFragmentShaderPath);
    if (FileSys.fileExists(fsPath)) {
        LDEBUG(fmt::format("Skipping creation of existing fragment shader {}", fsPath));
    }
    else {
        LDEBUG(fmt::format("Writing default fragment shader to '{}'", fsPath));
        std::ofstream file(fsPath);
        file << DefaultFragmentShaderSource;
    }
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

    ghoul::opengl::updateUniformLocations(*fr->_program, fr->_uniformCache, UniformNames);

    return std::unique_ptr<FontRenderer>(fr);
}

std::unique_ptr<FontRenderer> FontRenderer::createProjectionSubjectText() {
    std::string vsPath = absPath(ProjectionVertexShaderPath);
    if (FileSys.fileExists(vsPath)) {
        LDEBUG(fmt::format("Skipping creation of existing vertex shader {}", vsPath));
    }
    else {
        LDEBUG(fmt::format("Writing default vertex shader to '{}'", vsPath));
        std::ofstream file(vsPath);
        file << ProjectionVertexShaderSource;
    }

    std::string fsPath = absPath(ProjectionFragmentShaderPath);
    if (FileSys.fileExists(fsPath)) {
        LDEBUG(fmt::format("Skipping creation of existing fragment shader {}", vsPath));
    }
    else {
        LDEBUG(fmt::format("Writing default fragment shader to '{}'", fsPath));
        std::ofstream file(fsPath);
        file << ProjectionFragmentShaderSource;
    }
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

    ghoul::opengl::updateUniformLocations(*fr->_program, fr->_uniformCacheProjection,
        UniformNamesProjection);
    fr->_uniformMvp = fr->_program->uniformLocation("mvpMatrix");

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

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          const glm::vec2& pos,
                                                          std::string_view text,
                                                          const glm::vec4& color) const
{
    return render(font, pos, text, color, { 0.f, 0.f, 0.f, color.a });
}

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          const glm::vec2& pos,
                                                          std::string_view text,
                                                          const glm::vec4& color,
                                                      const glm::vec4& outlineColor) const
{
    const size_t lines = std::count(text.begin(), text.end(), '\n') + 1;

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
    
    _vertexBuffer.clear();
    _indexBuffer.clear();

    GLushort vertexIndex = 0;
    glm::vec2 size = glm::vec2(0.f);
    glm::vec2 movingPos = pos;
    do {
        std::string_view line = extractLine(text);
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

            // These variables are necessary as the insertion would otherwise produce a
            // narrowing error on clang
            const GLushort idx = vertexIndex;
            const GLushort idx1 = vertexIndex + 1;
            const GLushort idx2 = vertexIndex + 2;
            const GLushort idx3 = vertexIndex + 3;
            _indexBuffer.insert(_indexBuffer.end(), { idx, idx1, idx2, idx, idx2, idx3 });
            vertexIndex += 4;
            _vertexBuffer.insert(_vertexBuffer.end(), {
                x0, y0, s0, t0, outlineS0, outlineT0,
                x0, y1, s0, t1, outlineS0, outlineT1,
                x1, y1, s1, t1, outlineS1, outlineT1,
                x1, y0, s1, t0, outlineS1, outlineT0
            });
            movingPos.x += glyph->horizontalAdvance();

            width += glyph->horizontalAdvance();
            height = std::max(height, static_cast<float>(glyph->height()));
        }
        size.x = std::max(size.x, width);
        //size.y += height;
        movingPos.y -= font.height();
    } while (!text.empty());
    size.y = (lines - 1) * font.height();

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
        _vertexBuffer.size() * sizeof(float),
        _vertexBuffer.data(),
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        _indexBuffer.size() * sizeof(GLushort),
        _indexBuffer.data(),
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
        reinterpret_cast<const void*>(offsetof(Vertex, s)) // NOLINT
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, outlineS)) // NOLINT
    );

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(_indexBuffer.size()),
        GL_UNSIGNED_SHORT,
        nullptr
    );

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);

    return { size, static_cast<int>(lines) };
}

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          const glm::vec3& pos,
                                                          std::string_view text,
                                                          const glm::vec4& color,
                                                          const glm::vec4& outlineColor,
                                              const ProjectedLabelsInformation& labelInfo,
                                                            const glm::vec2& offset) const
{
    float h = font.height();

    _vertexBuffer.clear();
    _indexBuffer.clear();

    const size_t lines = std::count(text.begin(), text.end(), '\n') + 1;

    unsigned short vertexIndex = 0;
    glm::vec2 movingPos(offset);

    glm::vec2 size = glm::vec2(0.f);
    float heightInPixels = 0.f;
    do {
        std::string_view line = extractLine(text);
        //movingPos.x = 0.f;
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

            if (labelInfo.renderType == 0) {
                p0 = (x0 * labelInfo.orthoRight + y0 * labelInfo.orthoUp) *
                    labelInfo.scale + pos;
                p1 = (x0 * labelInfo.orthoRight + y1 * labelInfo.orthoUp) *
                    labelInfo.scale + pos;
                p2 = (x1 * labelInfo.orthoRight + y1 * labelInfo.orthoUp) *
                    labelInfo.scale + pos;
                p3 = (x1 * labelInfo.orthoRight + y0 * labelInfo.orthoUp) *
                    labelInfo.scale + pos;
            }
            else {
                glm::dvec3 normal = glm::normalize(labelInfo.cameraPos - glm::dvec3(pos));
                glm::vec3 newRight = glm::vec3(
                    glm::cross(labelInfo.cameraLookUp, normal)
                );
                glm::vec3 newUp = glm::vec3(glm::cross(normal, glm::dvec3(newRight)));

                p0 = (x0 * newRight + y0 * newUp) * labelInfo.scale + pos;
                p1 = (x0 * newRight + y1 * newUp) * labelInfo.scale + pos;
                p2 = (x1 * newRight + y1 * newUp) * labelInfo.scale + pos;
                p3 = (x1 * newRight + y0 * newUp) * labelInfo.scale + pos;
            }


            glm::vec4 projPos[2];
            projPos[0] = glm::vec4(labelInfo.mvpMatrix * glm::dvec4(p0, 1.0));
            projPos[1] = glm::vec4(labelInfo.mvpMatrix * glm::dvec4(p1, 1.0));
            glm::vec4 topLeft =
                (((projPos[0] / projPos[0].w) + glm::vec4(1.0)) / glm::vec4(2.0)) *
                glm::vec4(_framebufferSize.x, _framebufferSize.y, 1.0, 1.0);
            glm::vec4 bottomLeft =
                (((projPos[1] / projPos[1].w) + glm::vec4(1.0)) / glm::vec4(2.0)) *
                glm::vec4(_framebufferSize.x, _framebufferSize.y, 1.0, 1.0);

            // The billboard is bigger than the maximum size allowed:
            heightInPixels =
                std::equal_to<>()(heightInPixels, 0.0) ?
                glm::length(topLeft - bottomLeft) :
                heightInPixels;

            // Size-based culling
            if (heightInPixels < static_cast<float>(labelInfo.minSize) ||
                heightInPixels > _framebufferSize.x ||
                heightInPixels > _framebufferSize.y)
            {
                return { size, static_cast<int>(lines) };
            }

            if (heightInPixels > labelInfo.maxSize) {
                float scaleFix = static_cast<float>(labelInfo.maxSize) / heightInPixels;
                if (labelInfo.renderType == 0) {
                    p0 = (x0 * labelInfo.orthoRight + y0 * labelInfo.orthoUp) *
                        labelInfo.scale * scaleFix + pos;
                    p1 = (x0 * labelInfo.orthoRight + y1 * labelInfo.orthoUp) *
                        labelInfo.scale * scaleFix + pos;
                    p2 = (x1 * labelInfo.orthoRight + y1 * labelInfo.orthoUp) *
                        labelInfo.scale * scaleFix + pos;
                    p3 = (x1 * labelInfo.orthoRight + y0 * labelInfo.orthoUp) *
                        labelInfo.scale * scaleFix + pos;
                }
                else {
                    glm::dvec3 normal =
                        glm::normalize(labelInfo.cameraPos - glm::dvec3(pos));
                    glm::vec3 newRight =
                        glm::vec3(glm::cross(labelInfo.cameraLookUp, normal));
                    glm::vec3 newUp = glm::vec3(glm::cross(normal, glm::dvec3(newRight)));

                    p0 = (x0 * newRight + y0 * newUp) * labelInfo.scale * scaleFix + pos;
                    p1 = (x0 * newRight + y1 * newUp) * labelInfo.scale * scaleFix + pos;
                    p2 = (x1 * newRight + y1 * newUp) * labelInfo.scale * scaleFix + pos;
                    p3 = (x1 * newRight + y0 * newUp) * labelInfo.scale * scaleFix + pos;
                }
            }

            _vertexBuffer.insert(_vertexBuffer.end(), {
                p0.x, p0.y, p0.z, s0, t0, outlineS0, outlineT0,
                p1.x, p1.y, p1.z, s0, t1, outlineS0, outlineT1,
                p2.x, p2.y, p2.z, s1, t1, outlineS1, outlineT1,
                p3.x, p3.y, p3.z, s1, t0, outlineS1, outlineT0
            });

            const unsigned short vi = vertexIndex;
            const unsigned short vi1 = vertexIndex + 1;
            const unsigned short vi2 = vertexIndex + 2;
            const unsigned short vi3 = vertexIndex + 3;
            _indexBuffer.insert(_indexBuffer.end(), { vi, vi1, vi2, vi, vi2, vi3 });
            vertexIndex += 4;

            movingPos.x += glyph->horizontalAdvance();

            width += glyph->horizontalAdvance();
            height = std::max(height, static_cast<float>(glyph->height()));
        } // end of a line

        size.x = std::max(size.x, width);
        movingPos.y -= h;
    } while (!text.empty());
    size.y = (lines - 1) * font.height();

    if (!labelInfo.enableDepth) {
        glDisable(GL_DEPTH_TEST);
    }
    glEnablei(GL_BLEND, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _program->activate();

    opengl::TextureUnit atlasUnit;
    atlasUnit.activate();
    font.atlas().texture().bind();

    _program->setUniform(_uniformCacheProjection.baseColor, color);
    _program->setUniform(_uniformCacheProjection.outlineColor, outlineColor);
    _program->setUniform(_uniformCacheProjection.texture, atlasUnit);
    _program->setUniform(_uniformCacheProjection.hasOutline, font.hasOutline());
    _program->setUniform(_uniformMvp, labelInfo.mvpMatrix);
    _program->setUniform(
        _uniformCacheProjection.modelViewTransform,
        labelInfo.modelViewMatrix
    );
    _program->setUniform(
        _uniformCacheProjection.enableFalseDepth,
        labelInfo.enableFalseDepth
    );
    _program->setUniform(
        _uniformCacheProjection.disableTransmittance,
        labelInfo.disableTransmittance
    );

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        _vertexBuffer.size() * sizeof(float),
        _vertexBuffer.data(),
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        _indexBuffer.size() * sizeof(GLushort),
        _indexBuffer.data(),
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
        static_cast<GLsizei>(_indexBuffer.size()),
        GL_UNSIGNED_SHORT,
        nullptr
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if (!labelInfo.enableDepth) {
        glEnable(GL_DEPTH_TEST);
    }

    return { size, static_cast<int>(lines) };
}

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          const glm::vec3& pos,
                                                          std::string_view text,
                                                          const glm::vec4& color,
                                              const ProjectedLabelsInformation& labelInfo,
                                                            const glm::vec2& offset) const
{
    return render(
        font,
        pos,
        text,
        color,
        { 0.f, 0.f, 0.f, color.a },
        labelInfo,
        offset
    );
}

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          const glm::vec3& pos,
                                                          std::string_view text,
                                        const ProjectedLabelsInformation& labelInfo) const
{
    return render(
        font,
        pos,
        text,
        { 1.f, 1.f, 1.f, 1.f },
        { 0.f, 0.f, 0.f, 1.f },
        labelInfo
    );
}

void FontRenderer::setFramebufferSize(glm::vec2 framebufferSize) {
    _framebufferSize = std::move(framebufferSize);
}

} // namespace ghoul::fontrendering
