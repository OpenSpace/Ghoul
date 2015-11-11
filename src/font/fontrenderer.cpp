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
 ****************************************************************************************/

#include <ghoul/font/fontrenderer.h>

#include <ghoul/misc/assert.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/opengl/programobject.h>

#include <ghoul/opengl/textureunit.h>

#include <glm/gtc/matrix_transform.hpp>

//Replacement for Visual Studio's _vscprintf function
#if (_MSC_VER < 1400) //if older than visual studio 2005
static int vscprintf (const char * format, va_list pargs) {
    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
#else
#define vscprintf(f,a) _vscprintf(f,a)
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
    uniform vec4 color; \n\
    uniform vec4 outlineColor; \n\
    uniform bool hasOutline; \n\
    \n\
    void main() { \n\
        if (hasOutline) { \n\
            float inside = texture(tex, texCoords).r;\n\
            float outline = texture(tex, outlineTexCoords).r;\n\
            vec4 blend = mix(outlineColor, color, inside);\n\
            FragColor = blend * vec4(1.0, 1.0, 1.0, outline);\n\
        } \n\
        else { \n\
            FragColor = vec4(color.rgb, color.a * texture(tex, texCoords).r); \n\
        } \n\
    }";
}

namespace ghoul {
namespace fontrendering {
    
FontRenderer* FontRenderer::_defaultRenderer = nullptr;
    
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
    
FontRenderer::FontRenderer(opengl::ProgramObject* program, glm::vec2 windowSize)
    : _program(program)
    , _vao(0)
    , _vbo(0)
    , _ibo(0)
{
    ghoul_assert(program != nullptr, "No program provided");
    setWindowSize(std::move(windowSize));
    
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ibo);
}
    
FontRenderer::~FontRenderer() {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_ibo);
    
    delete _program;
    _program = nullptr;
}

bool FontRenderer::initialize() {
    LDEBUG("Creating default FontRenderer");
    ghoul_assert(_defaultRenderer == nullptr, "Default FontRenderer was already initialized");

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
    ProgramObject* program = new ProgramObject("Font");
    ShaderObject* vertex = new ShaderObject(ShaderObject::ShaderTypeVertex, vsPath);
    program->attachObject(vertex);
    ShaderObject* fragment = new ShaderObject(ShaderObject::ShaderTypeFragment, fsPath);
    program->attachObject(fragment);

    LDEBUG("Compile default font shader");
    bool compileSuccess = program->compileShaderObjects();
    if (!compileSuccess) {
        delete program;
        return false;
    }
    
    LDEBUG("Link default font shader");
    bool linkSuccess = program->linkProgramObject();
    if (!linkSuccess) {
        delete program;
        return false;
    }
    
    _defaultRenderer = new FontRenderer;
    _defaultRenderer->_program = program;
    
    return true;
}
    
bool FontRenderer::deinitialize() {
    ghoul_assert(_defaultRenderer != nullptr, "FontRenderer was not initialized");
    delete _defaultRenderer;
    _defaultRenderer = nullptr;
    
    return true;
}
    
FontRenderer& FontRenderer::defaultRenderer() {
    ghoul_assert(_defaultRenderer != nullptr, "FontRenderer was not initialized");
    return *_defaultRenderer;
}
    
void FontRenderer::render(ghoul::fontrendering::Font& font, glm::vec2 pos, const glm::vec4& color, const glm::vec4& outlineColor, const char* format, ...) const
{
    if (format == nullptr)
        return;
    
    float h = font.height();
    
    va_list args;	 // Pointer To List Of Arguments
    va_start(args, format); // Parses The String For Variables
    
    int size = 1 + vscprintf(format, args);
    char* buffer = new (std::nothrow) char[size];
    if (buffer == nullptr) {
        LERROR("Error allocating buffer");
        return;
    }
    
    memset(buffer, 0, size);
    
#if (_MSC_VER >= 1400) //visual studio 2005 or later
    vsprintf_s(buffer, size, format, args);
#else
    vsprintf(buffer, format, args);
#endif
    va_end(args);
    
    
    //Here is some code to split the text that we have been
    //given into a set of lines.
    //This could be made much neater by using
    //a regular expression library such as the one avliable from
    //boost.org (I've only done it out by hand to avoid complicating
    //this tutorial with unnecessary library dependencies).
    const char* start_line = buffer;
    std::vector<std::string> lines;
    char* c;
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
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        
        glm::vec2 movingPos = pos;
        
        movingPos.y -= i * h;
        for (size_t j = 0 ; j < line.size(); ++j) {
            Font::Glyph* glyph = font.glyph(line[j]);
            if (glyph == nullptr) {
                LERROR("No glyph for '" << line[j] << " in font '" << font.name() << "'");
            }
            else {
                float kerning = 0.f;
                if (j > 0)
                    kerning = glyph->kerning(line[j-1]);
                
                movingPos.x += kerning;
                float x0 = movingPos.x + glyph->offsetX();
                float y0 = movingPos.y + glyph->offsetY();
                float x1 = x0 + glyph->width();
                float y1 = y0 - glyph->height();
                float s0 = glyph->texCoordTopLeft().x;
                float t0 = glyph->texCoordTopLeft().y;
                float s1 = glyph->texCoordBottomRight().x;
                float t1 = glyph->texCoordBottomRight().y;

                float outlineS0 = glyph->outlineTexCoordTopLeft().x;
                float outlineT0 = glyph->outlineTexCoordTopLeft().y;
                float outlineS1 = glyph->outlineTexCoordBottomRight().x;
                float outlineT1 = glyph->outlineTexCoordBottomRight().y;

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
                movingPos.x += glyph->advanceX();
            }
            
        }
    }
    
    glm::mat4 projection = glm::ortho(
        0.f,
        _windowSize.x,
        0.f,
        _windowSize.y
    );
    
    ghoul::opengl::TextureUnit atlasUnit;
    atlasUnit.activate();
    font.atlas().texture().bind();
    
    _program->setIgnoreUniformLocationError(true);
    _program->setUniform("color", color);
    _program->setUniform("outlineColor", outlineColor);
    _program->setUniform("tex", atlasUnit);
    _program->setUniform("projection", projection);
    _program->setUniform("hasOutline", font.outline());
    _program->setIgnoreUniformLocationError(false);
    
    glBindVertexArray(_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
          0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<const void*>(2 * sizeof(float))
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<const void*>(4 * sizeof(float))
    );

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    delete[] buffer;
}
    
void FontRenderer::setWindowSize(glm::vec2 windowSize) {
    _windowSize = std::move(windowSize);
}
    
} // namespace fontrendering
} // namespace ghoul
