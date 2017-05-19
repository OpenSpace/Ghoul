/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#include <ghoul/font/font.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/opengl/programobject.h>
#include <ghoul/opengl/textureunit.h>

#include <glm/gtc/matrix_transform.hpp>

#include <cstdarg>
#include <fstream>

#ifdef WIN32
#define vscprintf(f,a) _vscprintf(f,a)
#else
static int vscprintf(const char* format, va_list pargs) {
    va_list argcopy;
    va_copy(argcopy, pargs);
    int retval = vsnprintf(nullptr, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
#endif


namespace {
    const std::string _loggerCat = "FontRenderer";
    
    const std::string DefaultVertexShaderPath = "${TEMPORARY}/defaultfontrenderer_vs.glsl";
    const std::string DefaultFragmentShaderPath = "${TEMPORARY}/defaultfontrenderer_fs.glsl";

    const std::string DefaultVertexShaderSource = "\
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
    
    const std::string DefaultFragmentShaderSource = "\
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
            FragColor = blend * vec4(1.0, 1.0, 1.0, outline);\n\
        } \n\
        else { \n\
            FragColor = vec4(baseColor.rgb, baseColor.a * texture(tex, texCoords).r); \n\
        } \n\
    }";
}

namespace ghoul {
namespace fontrendering {
    
std::unique_ptr<FontRenderer> FontRenderer::_defaultRenderer = nullptr;
    
FontRenderer::FontRenderer()
    : _program(nullptr)
    , _vao(0)
    , _vbo(0)
    , _ibo(0)
{
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
    LDEBUG("Writing default vertex shader to '" << vsPath << "'");
    std::ofstream file(vsPath);
    file << DefaultVertexShaderSource;
    file.close();
    
    std::string fsPath = absPath(DefaultFragmentShaderPath);
    LDEBUG("Writing default fragment shader to '" << fsPath << "'");
    file.open(fsPath);
    file << DefaultFragmentShaderSource;
    file.close();
    
    using namespace opengl;
    std::unique_ptr<ProgramObject> program = std::make_unique<ProgramObject>("Font");
    program->attachObject(std::make_unique<ShaderObject>(ShaderObject::ShaderTypeVertex, vsPath));
    program->attachObject(std::make_unique<ShaderObject>(ShaderObject::ShaderTypeFragment, fsPath));

    LDEBUG("Compile default font shader");
    program->compileShaderObjects();
    
    LDEBUG("Link default font shader");
    program->linkProgramObject();

    auto fontRenderer = new FontRenderer;
    fontRenderer->_program = std::move(program);
    return std::unique_ptr<FontRenderer>(fontRenderer);
}

bool FontRenderer::initialize() {
    LDEBUG("Creating default FontRenderer");
    ghoul_assert(!_defaultRenderer, "Default FontRenderer was already initialized");    
    _defaultRenderer = createDefault(); 
    return true;
}
    
bool FontRenderer::deinitialize() {
    _defaultRenderer = nullptr;
    return true;
}
    
bool FontRenderer::isInitialized() {
    return _defaultRenderer != nullptr;
}
    
FontRenderer& FontRenderer::defaultRenderer() {
    ghoul_assert(_defaultRenderer != nullptr, "FontRenderer was not initialized");
    return *_defaultRenderer;
}
    
FontRenderer::BoundingBoxInformation FontRenderer::boundingBox(Font& font,
                                                               const char* format,
                                                               ...) const 
{
    ghoul_assert(format != nullptr, "No format is provided");

    va_list args;     // Pointer To List Of Arguments
    va_start(args, format); // Parses The String For Variables

    int s = 1 + vscprintf(format, args);
    char* buffer = new char[s];

    memset(buffer, 0, s);

#if (_MSC_VER >= 1400) //visual studio 2005 or later
    vsprintf_s(buffer, s, format, args);
#else
    vsprintf(buffer, format, args);
#endif
    va_end(args);

    float h = font.height();

    // Splitting the text into separate lines
    const char* start_line = buffer;
    std::vector<std::string> lines;
    const char* c;
    for (c = buffer; *c; c++) {
        if (*c == '\n') {
            std::string line;
            for (const char* n = start_line; n < c; ++n)
                line.append(1, *n);
            lines.push_back(line);
            start_line = c + 1;
        }
    }
    if (start_line) {
        std::string line;
        for (const char* n = start_line; n < c; ++n)
            line.append(1, *n);
        lines.push_back(line);
    }

    unsigned int vertexIndex = 0;
    std::vector<GLuint> indices;
    std::vector<GLfloat> vertices;
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

            if (j > 0)
                movingPos.x += glyph->kerning(line[j - 1]);

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

            indices.insert(indices.end(), {
                vertexIndex, vertexIndex + 1, vertexIndex + 2,
                vertexIndex, vertexIndex + 2, vertexIndex + 3
            });
            vertexIndex += 4;
            vertices.insert(vertices.end(), {
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
        size.y += height;
        movingPos.y -= h;
    }
    size.y = (lines.size() - 1) * font.height();

    return { size, static_cast<int>(lines.size()) };

}


// I wish I didn't have to copy-n-paste the render function, but *sigh* ---abock
FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          glm::vec2 pos,
                                                          glm::vec4 color,
                                                          glm::vec4 outlineColor,
                                                          const char* format, ...) const
{
    ghoul_assert(format != nullptr, "No format is provided");
    
    va_list args;     // Pointer To List Of Arguments
    va_start(args, format); // Parses The String For Variables
    
    int size = 1 + vscprintf(format, args);
    char* buffer = new char[size];
    
    memset(buffer, 0, size);
    
#if (_MSC_VER >= 1400) //visual studio 2005 or later
    vsprintf_s(buffer, size, format, args);
#else
    vsprintf(buffer, format, args);
#endif
    va_end(args);
    
    auto res = internalRender(
        font,
        std::move(pos),
        std::move(color),
        std::move(outlineColor),
        buffer
    );
    delete[] buffer;

    return res;
}
    
FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          glm::vec2 pos,
                                                          glm::vec4 color,
                                                          const char* format, ...) const
{
    ghoul_assert(format != nullptr, "No format is provided");
    
    va_list args;     // Pointer To List Of Arguments
    va_start(args, format); // Parses The String For Variables
    
    int size = 1 + vscprintf(format, args);
    std::vector<char> buffer(size);
    memset(buffer.data(), 0, size);
    
#if WIN32 //visual studio 2005 or later
    vsprintf_s(buffer.data(), size, format, args);
#else
    vsprintf(buffer.data(), format, args);
#endif
    va_end(args);
    
    auto res = internalRender(
        font,
        std::move(pos),
        color,
        glm::vec4(0.f, 0.f, 0.f, color.a),
        buffer.data()
    );
    
    return res;
}

FontRenderer::BoundingBoxInformation FontRenderer::render(Font& font,
                                                          glm::vec2 pos,
                                                          const char* format, ...) const
{
    ghoul_assert(format != nullptr, "No format is provided");
    
    va_list args;     // Pointer To List Of Arguments
    va_start(args, format); // Parses The String For Variables
    
    int size = 1 + vscprintf(format, args);
    std::vector<char> buffer(size);

    memset(buffer.data(), 0, size);
    
#if (_MSC_VER >= 1400) //visual studio 2005 or later
    vsprintf_s(buffer.data(), size, format, args);
#else
    vsprintf(buffer.data(), format, args);
#endif
    va_end(args);
    
    auto res = internalRender(
        font,
        std::move(pos),
        glm::vec4(1.f),
        glm::vec4(0.f, 0.f, 0.f, 1.f),
        buffer.data()
    );
    
    return res;
}
    
FontRenderer::BoundingBoxInformation FontRenderer::internalRender(Font& font,
                                                                  glm::vec2 pos,
                                                                  glm::vec4 color,
                                                                  glm::vec4 outlineColor,
                                                                  const char* buffer)
                                                                  const
{
    float h = font.height();
    
    // Splitting the text into separate lines
    const char* start_line = buffer;
    std::vector<std::string> lines;
    const char* c;
    for (c = buffer; *c; c++) {
        if (*c == '\n') {
            std::string line;
            for (const char* n = start_line; n < c; ++n)
                line.append(1, *n);
            lines.push_back(line);
            start_line = c+1;
        }
    }
    if (start_line) {
        std::string line;
        for(const char* n = start_line; n < c; ++n)
            line.append(1, *n);
        lines.push_back(line);
    }
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    _program->activate();
    
    unsigned int vertexIndex = 0;
    std::vector<GLuint> indices;
    std::vector<GLfloat> vertices;
    glm::vec2 movingPos = pos;

    glm::vec2 size = glm::vec2(0.f);
    for (const std::string& line : lines) {
        movingPos.x = pos.x;
        float width = 0.f;
        float height = 0.f;
        for (size_t j = 0 ; j < line.size(); ++j) {
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
        
            if (j > 0)
                movingPos.x += glyph->kerning(line[j-1]);
            
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
            
            indices.insert(indices.end(), {
                vertexIndex, vertexIndex + 1, vertexIndex + 2,
                vertexIndex, vertexIndex + 2, vertexIndex + 3
            });
            vertexIndex += 4;
            vertices.insert(vertices.end(), {
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
        size.y += height;
        movingPos.y -= h;
    }
    size.y = (lines.size() -  1) * font.height();
    
    glm::mat4 projection = glm::ortho(
        0.f,
        _framebufferSize.x,
        0.f,
        _framebufferSize.y
    );
    
    opengl::TextureUnit atlasUnit;
    atlasUnit.activate();
    font.atlas().texture().bind();
    
    _program->setIgnoreUniformLocationError(opengl::ProgramObject::IgnoreError::Yes);
    _program->setUniform("baseColor", color);
    _program->setUniform("outlineColor", outlineColor);
    _program->setUniform("tex", atlasUnit);
    _program->setUniform("projection", projection);
    _program->setUniform("hasOutline", font.hasOutline());
    _program->setIgnoreUniformLocationError(opengl::ProgramObject::IgnoreError::No);
    
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), reinterpret_cast<const void*>(2 * sizeof(float))
    );
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), reinterpret_cast<const void*>(4 * sizeof(float))
    );
    
    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(indices.size()),
        GL_UNSIGNED_INT,
        0
    );
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    
    return { size, static_cast<int>(lines.size()) };
}
    
void FontRenderer::setFramebufferSize(glm::vec2 framebufferSize) {
    _framebufferSize = std::move(framebufferSize);
}
    
} // namespace fontrendering
} // namespace ghoul
