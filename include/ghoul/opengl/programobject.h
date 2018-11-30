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

#ifndef __GHOUL___PROGRAMOBJECT___H__
#define __GHOUL___PROGRAMOBJECT___H__

#include <ghoul/misc/boolean.h>
#include <ghoul/misc/exception.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/opengl/shaderobject.h>
#include <ghoul/glm.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ghoul::opengl {

/**
 * This class is a wrapper for an OpenGL program object storing an array of OpenGL
 * shaders. The usual restrictions for shader objects as described in the OpenGL and GLSL
 * language specifications apply. The ProgramObject can have a <code>name</code> that is
 * used in the logging feature and as an OpenGL object label, if OpenGL > 4.3 is
 * available. Before the ProgramObject can be used, ShaderObject%s have to be attached,
 * the ShaderObject%s have to be compiled (#compileShaderObjects will compile all attached
 * ShaderObject%s) and the ProgramObject has to be linked before it can be used (using
 * #activate and #deactivate). If the OpenGL name of the ProgramObject is required, it is
 * available as an overloaded operator to an <code>GLuint</code> type. The copy
 * constructor and the assignment operation will perform deep copies of the attached
 * ShaderObject%s. When setting uniforms, attributes, subroutines, and subroutine
 * uniforms, a flag is available (#setIgnoreUniformLocationError,
 * #setIgnoreAttributeLocationError, #setIgnoreSubroutineLocationError, and
 * #setIgnoreSubroutineUniformLocationError) that enables/disables the logging in case one
 * of the resources was not found during the function call. All other sanity checks will
 * only be performed if the library is compiled with the <code>GHL_DEBUG</code> macro
 * being defined.
 */
class ProgramObject {
public:
    BooleanType(IgnoreError);
    BooleanType(Transpose);

    /**
     * A type definition for a callback function that is called if any of the tracked
     * files is changed.
     */
    using ProgramObjectCallback = void(*)(ProgramObject*);

    /// Main exception that is thrown by methods of the ProgramObject class
    struct ProgramObjectError : public RuntimeError {
        explicit ProgramObjectError(std::string msg);
    };

    /// Exception that is thrown if the linking of a ProgramObject fails
    struct ProgramObjectLinkingError : public ProgramObjectError {
        ProgramObjectLinkingError(std::string msg, std::string name);

        /// The linker error that was returned from the GLSL linker
        std::string linkerError;

        /// The name of the program that caused the exception to be thrown
        std::string programName;
    };

    /**
     * Constructor that will automatically create a new OpenGL name for the new
     * ProgramObject. The internal name will be initialized as empty and the default state
     * for all location errors is that they will be logged.
     *
     * \throw ProgramObjectError If there was an error creating the OpenGL program object
     *        name
     */
    ProgramObject();

    /**
     * Constructor that will automatically create a new OpenGL name for the new
     * ProgramObject. The internal name (and debug name, if available) will be set to the
     * passed \p name. The default state for all location errors is that they will be
     * logged.
     *
     * \throw ProgramObjectError If there was an error creating the OpenGL program object
     *        name
     */
    ProgramObject(std::string name);

    /**
     * Copy constructor that will perform a deep copy of all the attached ShaderObject%s
     * and copies the state of the location errors and internal (and debug) name. The
     * copied ShaderObject%s' ownership will belong to the newly created ProgramObject
     * while the original`s ownership will be unchanged.
     *
     * \throw ProgramObjectError If there was an error creating the OpenGL program object
     *        name
     */
    ProgramObject(const ProgramObject& cpy);

    /**
     * Destructor that will delete the OpenGL name that is attached to the ProgramObject.
     * Please note, while the lifetime of the ProgramObject ends after the destructor
     * finishes, the OpenGL name might still be in use and is only deleted if the last
     * (OpenGL internal) reference to the program object is removed. This means that the
     * OpenGL name will leak and will be unreachable if the destructor is called while the
     * ProgramObject is still #activate%d.
     */
    ~ProgramObject();

    /**
     * This operator will reveal the OpenGL name of this ProgramObject so that it can be
     * directly used in native OpenGL functions.
     *
     * \return The OpenGL name of this ProgramObject
     */
    operator GLuint() const;

    /**
     * Assignment operator that will copy all the internal state of the right hand side's
     * ProgramObject and will perform a deep copy of all the attached ShaderObject%s.
     *
     * \param rhs The assignment source
     */
    ProgramObject& operator=(const ProgramObject& rhs);

    /**
     * Assignment operator that will move all the internal state of the right hand side's
     * ProgramObject and will invalidate the right hand side's object.
     *
     * \param rhs The assignment source
     */
    ProgramObject& operator=(ProgramObject&& rhs);

    /**
     * Sets the human readable name used for logging and (if available) the object label
     * for this ProgramObject. Besides this, the name is not used internally.
     *
     * \param name The human readable name of this ProgramObject
     */
    void setName(std::string name);

    /**
     * Returns the human readable name for this ProgramObject.
     *
     * \return The name for this ProgramObject
     */
    const std::string& name() const;

    /**
     * Sets the dictionary, without rebuilding the shader. The program object will be
     * dirty after this operation. (_builtLatest will be false)
     */
    void setDictionary(const Dictionary& dictionary);

    /**
     * Returns the current dictionary.
     *
     * \return The current dictionary
     */
    Dictionary dictionary();

    /**
     * Sets the shader object callback.
     *
     * \param changeCallback object
     */
    void setProgramObjectCallback(ProgramObjectCallback changeCallback);

    /**
     * Attaches the specified ShaderObject to this ProgramObject.
     *
     * \param shaderObject The ShaderObject that will be attached to this ProgramObject.
     *
     * \pre \p shaderObject must not be nullptr
     * \pre \p shaderObject must not have been registered before
     */
    void attachObject(std::shared_ptr<ShaderObject> shaderObject);

    /**
     * Detaches the specified ShaderObject from this ProgramObject.
     *
     * \param shaderObject The ShaderObject that should be detached.
     *
     * \pre \p shaderObject must not be nullptr
     * \pre \p shaderObject must have been registered before
     */
    void detachObject(const std::shared_ptr<ShaderObject>& shaderObject);

    /**
     * Compiles all the ShaderObject%s that are attached to this ProgramObject.
     *
     * \throw ShaderCompileError If there was an error compiling at least one of the
     *        attached ShaderObject%s
     */
    void compileShaderObjects();

    /**
     * Links all attached ShaderObject%s into a program object.
     *
     * \throw ProgramObjectLinkingError If there was an error linking the ProgramObject
     */
    void linkProgramObject();

    /**
     * Reloads and rebuilds all the attached ShaderObject%s from their respective files.
     * Will call ShaderObject::reloadFromFile on each attached ShaderObject. If an error
     * occurs while rebuilding, an exception is thrown, but the object that this method is
     * called on is unchanged. Only if the compiling and linking succeeds, this object is
     * modified.
     *
     * \throw ShaderCompileError If there was an error compiling at least one of the
     *        attached ShaderObject%s
     * \throw ProgramObjectLinkingError If there was an error linking the ProgramObject
     */
    void rebuildFromFile();

    /**
     * Returns whether this ProgramObject is dirty and needs to be recompiled and rebuilt
     * due to a change in the underlying file. A ProgramObject is automatically set to be
     * not dirty once it is linked correctly.
     *
     * \return <code>true</code> if this ProgramObject has to be recompiled and rebuilt
     */
    bool isDirty() const;

    /**
     * Activates this ProgramObject for use in the OpenGL. Will call the OpenGL function
     * <code>glUseProgram</code>.
     */
    void activate();

    /**
     * Deactivates this ProgramObject by activating OpenGL program object <code>0</code>.
     * Will call the OpenGL function <code>glUseProgram</code>.
     */
    void deactivate();

    /**
     * Constructs and links a ProgramObject built from the provided \p vertexShaderPath
     * vertex shader, and \p fragmentShaderPath fragment shader.
     *
     * \param name The human readable name of this ProgramObject
     * \param vertexShaderPath The name of the vertex shader file that will be used to
     *        load the source of this shader
     * \param fragmentShaderPath The name of the vertex shader file that will be used to
     *        load the source of this shader
     * \param dictionary The dictionary that is used for the created ShaderObject%s and
     *        ultimately for the ShaderPreprocessor
     * \return The constructed ProgramObject if successful
     *
     * \throw FileNotFoundError If any of the shader files could not be found
     * \throw ShaderCompileError If there was an error compiling at least one of the
     *        loaded shaders
     * \throw ProgramObjectLinkingError If there was an error linking the ProgramObject
     * \pre \p vertexShaderPath must not be empty
     * \pre \p vertexShaderPath must be a file that exists
     * \pre \p fragmentShaderPath must not be empty
     * \pre \p fragmentShaderPath must be a file that exists
     */
    static std::unique_ptr<ProgramObject> Build(const std::string& name,
        const std::string& vertexShaderPath, const std::string& fragmentShaderPath,
        Dictionary dictionary = Dictionary());

    /**
     * Constructs and links a ProgramObject built from the provided \p vertexShaderPath
     * vertex shader, \p geometryShaderPath geometry shader, and \p fragmentShaderPath
     * fragment shader.
     *
     * \param name The human readable name of this ProgramObject
     * \param vertexShaderPath The name of the vertex shader file that will be used to
     *        load the source of this shader
     * \param fragmentShaderPath The name of the vertex shader file that will be used to
     *        load the source of this shader
     * \param geometryShaderPath The name of the geometry shader file that will be used to
     *        load the source of this shader
     * \param dictionary The dictionary that is used for the created ShaderObject%s and
     *        ultimately for the ShaderPreprocessor
     * \return The constructed ProgramObject if successful
     *
     * \throw FileNotFoundError If any of the shader files could not be found
     * \throw ShaderCompileError If there was an error compiling at least one of the
     *        loaded shaders
     * \throw ProgramObjectLinkingError If there was an error linking the ProgramObject
     * \pre \p vertexShaderPath must not be empty
     * \pre \p vertexShaderPath must be a file that exists
     * \pre \p fragmentShaderPath must not be empty
     * \pre \p fragmentShaderPath must be a file that exists
     * \pre \p geometryShaderPath must not be empty
     * \pre \p geometryShaderPath must be a file that exists
     */
    static std::unique_ptr<ProgramObject> Build(const std::string& name,
        const std::string& vertexShaderPath, const std::string& fragmentShaderPath,
        const std::string& geometryShaderPath, Dictionary dictionary = Dictionary());

    /**
     * Constructs and links a ProgramObject built from the provided \p vertexShaderPath
     * vertex shader, \p geometryShaderPath geometry shader,
     * \p tessellationEvaluationShaderPath tessellation evaluation shader,
     * \p tessellationControlShaderPath tessellation control shader, and
     * \p fragmentShaderPath fragment shader.
     *
     * \param name The human readable name of this ProgramObject
     * \param vertexShaderPath The name of the vertex shader file that will be used to
     *        load the source of this shader
     * \param fragmentShaderPath The name of the vertex shader file that will be used to
     *        load the source of this shader
     * \param geometryShaderPath The name of the geometry shader file that will be used to
     *        load the source of this shader
     * \param tessellationEvaluationShaderPath The name of the tessellation evaluation
     *        shader file that will be used to load the source of this shader
     * \param tessellationControlShaderPath The name of the tessellation control shader
     *        file that will be used to load the source of this shader
     * \param dictionary The dictionary that is used for the created ShaderObject%s and
     *        ultimately for the ShaderPreprocessor
     * \return The constructed ProgramObject if successful
     *
     * \throw FileNotFoundError If any of the shader files could not be found
     * \throw ShaderCompileError If there was an error compiling at least one of the
     *        loaded shaders
     * \throw ProgramObjectLinkingError If there was an error linking the ProgramObject
     * \pre \p vertexShaderPath must not be empty
     * \pre \p vertexShaderPath must be a file that exists
     * \pre \p fragmentShaderPath must not be empty
     * \pre \p fragmentShaderPath must be a file that exists
     * \pre \p geometryShaderPath must not be empty
     * \pre \p geometryShaderPath must be a file that exists
     * \pre \p tessellationEvaluationShaderPath must not be empty
     * \pre \p tessellationEvaluationShaderPath must be a file that exists
     * \pre \p tessellationControlShaderPath must not be empty
     * \pre \p tessellationControlShaderPath must be a file that exists
     */
    static std::unique_ptr<ProgramObject> Build(const std::string& name,
        const std::string& vertexShaderPath, const std::string& fragmentShaderPath,
        const std::string& geometryShaderPath,
        const std::string& tessellationEvaluationShaderPath,
        const std::string& tessellationControlShaderPath,
        Dictionary dictionary = Dictionary());

    //////////////////////////////////////////////////////////////////////////////////////
    ////// Uniforms
    //////////////////////////////////////////////////////////////////////////////////////

    /**
     * Enables and disables the logging of a warning if the location of a uniform can not
     * be found in any of the attached ShaderObjects.
     *
     * \param ignoreError Should the location error be ignored?
     */
    void setIgnoreUniformLocationError(IgnoreError ignoreError);

    /**
     * Returns the state of this ProgramObject if the logging of location errors should be
     * enabled or not.
     *
     * \return <code>true</code> if location errors will be ignored
     */
    bool ignoreUniformLocationError() const;

    /**
     * Returns the location of the uniform specified by \p name. If the uniform can not be
     * found in any of the attached ShaderObject%s, <code>-1</code> is returned and
     * (provided it is not disabled) a warning will be logged. Will call the OpenGL
     * function <code>glGetUniformLocation</code>.
     *
     * \param name The name of the uniform for which the location should be fetched
     * \return The location of the uniform, or <code>-1</code> if the uniform could not be
     *         found
     *
     * \pre \p name must not be empty
     */
    GLint uniformLocation(const std::string& name) const;

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform1i</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, bool value);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1 and \p v2.
     * Returns <code>true</code> if the uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniform2i</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, bool v1, bool v2);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1, \p v2, and
     * \p v3. Returns <code>true</code> if the uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniform3i</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, bool v1, bool v2, bool v3);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1, \p v2, \p v3,
     * and \p v4. Returns <code>true</code> if the uniform could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform4i</code>.
     *
     * \param name The name of the uniform in the ShaderObjects%
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \param v4 The fourth value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, bool v1, bool v2, bool v3, bool v4);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform2iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::bvec2& value);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform3iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::bvec3& value);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform4iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::bvec4& value);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform1iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const bool* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform2iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::bvec2* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform3iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code>otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::bvec3* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform4iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::bvec4* values, int count = 1);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform1ui</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code>otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLuint value);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1 and \p v2.
     * Returns <code>true</code> if the uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniform2ui</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLuint v1, GLuint v2);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1, \p v2, and
     * \p v3. Returns <code>true</code> if the uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniform3ui</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLuint v1, GLuint v2, GLuint v3);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1, \p v2, \p v3,
     * and \p v4. Returns <code>true</code> if the uniform could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform4ui</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \param v4 The fourth value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLuint v1, GLuint v2, GLuint v3, GLuint v4);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform2uiv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::uvec2& value);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform3uiv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::uvec3& value);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform4uiv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::uvec4& value);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform1uiv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const GLuint* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform2uiv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::uvec2* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform3uiv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::uvec3* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform4uiv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::uvec4* values, int count = 1);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform1i</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLint value);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1 and \p v2.
     * Returns <code>true</code> if the uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniform2i</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLint v1, GLint v2);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1, \p v2, and
     * \p v3. Returns <code>true</code> if the uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniform3i</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLint v1, GLint v2, GLint v3);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1, \p v2, \p v3,
     * and \p v4. Returns <code>true</code> if the uniform could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform4i</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \param v4 The fourth value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLint v1, GLint v2, GLint v3, GLint v4);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform2iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::ivec2& value);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform3iv</code>.
     *
     * \param name The name of the uniform in the ShaderObjects
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::ivec3& value);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform4iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::ivec4& value);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform1iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const GLint* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform2iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::ivec2* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform3iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::ivec3* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform4iv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::ivec4* values, int count = 1);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform1f</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLfloat value);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1 and \p v2.
     * Returns <code>true</code> if the uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniform2f</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLfloat v1, GLfloat v2);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1, \p v2, and
     * \p v3. Returns <code>true</code> if the uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniform3f</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLfloat v1, GLfloat v2, GLfloat v3);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1, \p v2, \p v3,
     * and \p v4. Returns <code>true</code> if the uniform could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform4f</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \param v4 The fourth value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name,
        GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform2fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::vec2& value);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform3fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::vec3& value);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform4fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::vec4& value);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform1fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const GLfloat* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform2fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::vec2* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform3fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::vec3* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform4fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::vec4* values, int count = 1);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform1d</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLdouble value);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1 and \p v2.
     * Returns <code>true</code> if the uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniform2d</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLdouble v1, GLdouble v2);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1, \p v2, and
     * \p v3. Returns <code>true</code> if the uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniform3d</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, GLdouble v1, GLdouble v2, GLdouble v3);

    /**
     * Locates and sets the uniform \p name with the passed values \p v1, \p v2, \p v3,
     * and \p v4. Returns <code>true</code> if the uniform could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform4d</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \param v4 The fourth value that should be used to set the uniform
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name,
        GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform2dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dvec2& value);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform3dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dvec3& value);

    /**
     * Locates and sets the uniform \p name with the passed \p value. Returns
     * <code>true</code> if the uniform could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glProgramUniform4dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dvec4& value);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform1dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const GLdouble* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform2dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::dvec2* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform3dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::dvec3* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed values \p values, where the
     * array consists of \p count number of values. Returns <code>true</code> if the
     * uniform could be found; <code>false</code> otherwise. Will call the OpenGL function
     * <code>glProgramUniform4dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     * \return <code>true</code> if the uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, glm::dvec4* values, int count = 1);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::mat2x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2x3fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::mat2x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2x4fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::mat2x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3x2fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::mat3x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::mat3x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3x4fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::mat3x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4x2fv</code>.
     *
     * \param name The name of the uniform in the ShaderObjects
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::mat4x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4x3fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::mat4x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4fv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::mat4x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dmat2x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2x3dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dmat2x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2x4dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dmat2x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3x2dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dmat3x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dmat3x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3x4dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dmat3x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4x2dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dmat4x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4x3dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dmat4x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the uniform(s) \p name with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4dv</code>.
     *
     * \param name The name of the uniform in the ShaderObject%s
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the initial uniform was successfully located,
     *         <code>false</code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setUniform(const std::string& name, const glm::dmat4x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform1i</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, bool value);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, and \p v2.
     * Will call the OpenGL function <code>glProgramUniform2i</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, bool v1, bool v2);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, \p v2, and
     * \p v3. Will call the OpenGL function <code>glProgramUniform3i</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, bool v1, bool v2, bool v3);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, \p v2, \p v3,
     * and \p v4. Will call the OpenGL function <code>glProgramUniform4i</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \param v4 The fourth value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, bool v1, bool v2, bool v3, bool v4);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform2iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::bvec2& value);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform3iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::bvec3& value);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform4iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::bvec4& value);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform1iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const bool* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform2iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::bvec2* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform3iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::bvec3* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform4iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::bvec4* values, int count = 1);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform1ui</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLuint value);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, and \p v2.
     * Will call the OpenGL function <code>glProgramUniform2ui</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLuint v1, GLuint v2);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, \p v2, and
     * \p v3. Will call the OpenGL function <code>glProgramUniform3ui</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLuint v1, GLuint v2, GLuint v3);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, \p v2, \p v3,
     * and \p v4. Will call the OpenGL function <code>glProgramUniform4ui</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \param v4 The fourth value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLuint v1, GLuint v2, GLuint v3, GLuint v4);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform2uiv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::uvec2& value);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform3uiv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::uvec3& value);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform4uiv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::uvec4& value);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform1uiv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const GLuint* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform2uiv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::uvec2* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform3uiv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::uvec3* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform4uiv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::uvec4* values, int count = 1);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform1i</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLint value);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, and \p v2.
     * Will call the OpenGL function <code>glProgramUniform2i</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLint v1, GLint v2);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, \p v2, and
     * \p v3. Will call the OpenGL function <code>glProgramUniform3i</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLint v1, GLint v2, GLint v3);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, \p v2, \p v3,
     * and \p v4. Will call the OpenGL function <code>glProgramUniform4i</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \param v4 The fourth value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLint v1, GLint v2, GLint v3, GLint v4);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform2iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::ivec2& value);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform3iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::ivec3& value);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform4iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::ivec4& value);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform1iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const GLint* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform2iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::ivec2* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform3iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::ivec3* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform4iv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::ivec4* values, int count = 1);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform1f</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLfloat value);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, and \p v2.
     * Will call the OpenGL function <code>glProgramUniform2f</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLfloat v1, GLfloat v2);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, \p v2, and
     * \p v3. Will call the OpenGL function <code>glProgramUniform3f</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLfloat v1, GLfloat v2, GLfloat v3);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, \p v2, \p v3,
     * and \p v4. Will call the OpenGL function <code>glProgramUniform4f</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \param v4 The fourth value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform2fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::vec2& value);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform3fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::vec3& value);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform4fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::vec4& value);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform1fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const GLfloat* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform2fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::vec2* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform3fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::vec3* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform4fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::vec4* values, int count = 1);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform1d</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLdouble value);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, and \p v2.
     * Will call the OpenGL function <code>glProgramUniform2d</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLdouble v1, GLdouble v2);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, \p v2, and
     * \p v3. Will call the OpenGL function <code>glProgramUniform3d</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLdouble v1, GLdouble v2, GLdouble v3);

    /**
     * Sets the uniform located at \p location with the passed values \p v1, \p v2, \p v3,
     * and \p v4. Will call the OpenGL function <code>glProgramUniform4d</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param v1 The first value that should be used to set the uniform
     * \param v2 The second value that should be used to set the uniform
     * \param v3 The third value that should be used to set the uniform
     * \param v4 The fourth value that should be used to set the uniform
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform2dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dvec2& value);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform3dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dvec3& value);

    /**
     * Sets the uniform located at \p location with the passed \p value. Will call the
     * OpenGL function <code>glProgramUniform4dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dvec4& value);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform1dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const GLdouble* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform2dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::dvec2* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform3dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::dvec3* values, int count = 1);

    /**
     * Sets the uniform(s) initially located at \p location with the passed \p values,
     * where the array consists of \p count elements. Will call the OpenGL function
     * <code>glProgramUniform4dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param values The values the uniform(s) should be set to
     * \param count The number of values that are stored in values
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, glm::dvec4* values, int count = 1);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value.
     * Returns <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::mat2x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2x3fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::mat2x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2x4fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::mat2x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3x2fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::mat3x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::mat3x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3x4fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::mat3x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4x2fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::mat4x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4x3fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::mat4x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4fv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::mat4x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dmat2x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2x3dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dmat2x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix2x4dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dmat2x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3x2dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dmat3x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dmat3x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix3x4dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dmat3x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4x2dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dmat4x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise Will call the OpenGL function <code>glProgramUniformMatrix4x3dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dmat4x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the uniform(s) located at \p location with the passed value \p value. Returns
     * <code>true</code> if the initial uniform could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glProgramUniformMatrix4dv</code>.
     *
     * \param location The location of the uniform retrieved from #uniformLocation
     * \param value The value the uniform should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>-1</code>
     */
    void setUniform(GLint location, const glm::dmat4x4& value,
        Transpose transpose = Transpose::No);


    //////////////////////////////////////////////////////////////////////////////////////
    ////// SSBO Bindings
    //////////////////////////////////////////////////////////////////////////////////////

    /**
     * Locates and sets the SSBO \p name with the passed \p binding. Returns
     * <code>true</code> if the block could be found; <code>false</code> otherwise. Will
     * call the OpenGL function <code>glGetUniformBlockIndex</code>.
     *
     * \param name The name of the SSBO in the ShaderObject%s
     * \param binding The binding the block should be bound to
     * \return <code>true</code> if the block was successfully located, <code>false
     *         </code> otherwise
     *
     * \pre \p name must not be empty
     */
    bool setSsboBinding(const std::string& name, GLuint binding);

    /**
     * Sets the block with index \p index with the passed binding \p binding. Will call
     * the OpenGL function <code>glShaderStorageBlockBinding</code>.
     *
     * \param index The index of the block retrieved from
     *        <code>glGetUniformBlockIndex</code>
     *
     * \param binding The binding the block should be bound to
     */
    void setSsboBinding(GLuint index, GLuint binding);


    //////////////////////////////////////////////////////////////////////////////////////
    ////// Attributes
    //////////////////////////////////////////////////////////////////////////////////////

    /**
     * Enables and disables the logging of a warning if the location of an attribute can
     * not be found in any of the attached ShaderObjects.
     *
     * \param ignoreError Should the location error be ignored?
     */
    void setIgnoreAttributeLocationError(IgnoreError ignoreError);

    /**
     * Returns the state of this ProgramObject if the logging of location errors should be
     * enabled or not.
     *
     * \return <code>true</code> if location errors will be ignored
     */
    bool ignoreAttributeLocationError() const;

    /**
     * Returns the location of the attribute specified by \p name. If the attribute can
     * not be found in any of the attached ShaderObject%s, <code>-1</code> is returned and
     * (provided it is not disabled) a warning will be logged. Will call the OpenGL
     * function <code>glGetAttribLocation</code>.
     *
     * \param name The name of the attribute for which the location should be fetched
     * \return The location of the attribute, or <code>-1</code> if it could not be found
     *
     * \pre \p name must not be empty
     */
    GLuint attributeLocation(const std::string& name) const;

    /**
     * Binds the generic vertex attribute \p index to the attribute variable \p name. Will
     * call the OpenGL function <code>glBindAttribLocation</code>.
     *
     * \param name The name of the attribute variable
     * \param index The location of the generic vertex attribute that should be bound
     *
     * \pre \p name must not be empty
     */
    void bindAttributeLocation(const std::string& name, GLuint index);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribI1i</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, bool value);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1 and \p v2.
     * Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttribI2i</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, bool v1, bool v2);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1, \p v2,
     * and \p v3. Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttribI3i</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, bool v1, bool v2, bool v3);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1, \p v2,
     * \p v3, and \p v4. Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttribI4i</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \param v4 The fourth value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, bool v1, bool v2, bool v3, bool v4);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribI2iv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::bvec2& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribI3iv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::bvec3& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribI4iv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::bvec4& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribI1i</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, GLint value);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1 and \p v2.
     * Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttribI2i</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, GLint v1, GLint v2);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1, \p v2,
     * and \p v3. Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttribI3i</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, GLint v1, GLint v2, GLint v3);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1, \p v2,
     * \p v3, and \p v4. Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttribI4i</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \param v4 The fourth value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, GLint v1, GLint v2, GLint v3, GLint v4);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribI2iv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::ivec2& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribI3iv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::ivec3& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribI4iv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::ivec4& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib1f</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, GLfloat value);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1 and \p v2.
     * Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttrib2f</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, GLfloat v1, GLfloat v2);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1, \p v2,
     * and \p v3. Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttrib3f</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, GLfloat v1, GLfloat v2, GLfloat v3);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1, \p v2,
     * \p v3, and \p v4. Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttrib4f</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \param v4 The fourth value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name,
        GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib2fv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::vec2& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib3fv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::vec3& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib4fv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::vec4& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL1d</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, GLdouble value);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1 and \p v2.
     * Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttribL2d</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, GLdouble v1, GLdouble v2);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1, \p v2,
     * and \p v3. Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttribL3d</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, GLdouble v1, GLdouble v2, GLdouble v3);

    /**
     * Locates and sets the vertex attribute \p name to the passed values \p v1, \p v2,
     * \p v3, and \p v4. Returns <code>true</code> if the vertex attribute could be found;
     * <code>false</code> otherwise. Will call the OpenGL function
     * <code>glVertexAttribL4d</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \param v4 The fourth value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name,
        GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL2dv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dvec2& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL3dv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dvec3& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL4dv</code>.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dvec4& value);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib2fv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::mat2x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib3fv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::mat2x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib4fv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::mat2x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib2fv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::mat3x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib3fv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::mat3x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib4fv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::mat3x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib2fv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::mat4x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib3fv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::mat4x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttrib4fv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::mat4x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL2dv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dmat2x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL3dv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dmat2x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL4dv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dmat2x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL2dv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dmat3x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL3dv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dmat3x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL4dv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dmat3x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL2dv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dmat4x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL3dv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dmat4x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Locates and sets the vertex attribute \p name to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL4dv</code> on the
     * rows/columns.
     *
     * \param name The name of the vertex attribute to be set
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     * \return <code>true</code> if the attribute was successfully set, <code>false</code>
     *         otherwise
     *
     * \pre \p name must not be empty
     */
    bool setAttribute(const std::string& name, const glm::dmat4x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribI1i</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, bool value);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1 and \p v2.
     * Will call the OpenGL function <code>glVertexAttribI2i</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, bool v1, bool v2);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1, \p v2, and
     * \p v3. Will call the OpenGL function <code>glVertexAttribI3i</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, bool v1, bool v2, bool v3);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1, \p v2,
     * \p v3, and \p v4. Will call the OpenGL function <code>glVertexAttribI4i</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \param v4 The fourth value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, bool v1, bool v2, bool v3, bool v4);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribI2iv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::bvec2& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribI3iv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::bvec3& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribI4iv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::bvec4& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribI1i</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLint value);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1 and \p v2.
     * Will call the OpenGL function <code>glVertexAttribI2i</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLint v1, GLint v2);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1, \p v2, and
     * \p v3. Will call the OpenGL function <code>glVertexAttribI3i</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLint v1, GLint v2, GLint v3);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1, \p v2,
     * \p v3, and \p v4. Will call the OpenGL function <code>glVertexAttribI4i</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \param v4 The fourth value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLint v1, GLint v2, GLint v3, GLint v4);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribI2iv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::ivec2& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribI3iv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::ivec3& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribI4iv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::ivec4& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib1f</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLfloat value);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1 and \p v2.
     * Will call the OpenGL function <code>glVertexAttrib2f</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLfloat v1, GLfloat v2);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1, \p v2, and
     * \p v3. Will call the OpenGL function <code>glVertexAttrib3f</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLfloat v1, GLfloat v2, GLfloat v3);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1, \p v2,
     * \p v3, and \p v4. Will call the OpenGL function <code>glVertexAttrib4f</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \param v4 The fourth value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib2fv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::vec2& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib3fv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::vec3& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib4fv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::vec4& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL1d</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLdouble value);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1 and \p v2.
     * Will call the OpenGL function <code>glVertexAttribL2d</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLdouble v1, GLdouble v2);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1, \p v2, and
     * \p v3. Will call the OpenGL function <code>glVertexAttribL3d</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLdouble v1, GLdouble v2, GLdouble v3);

    /**
     * Sets the vertex attribute at the \p location to the passed values \p v1, \p v2,
     * \p v3, and \p v4. Will call the OpenGL function <code>glVertexAttribL4d</code>.
     *
     * \param location The location of the vertex attribute
     * \param v1 The first value the vertex attribute should be set to
     * \param v2 The second value the vertex attribute should be set to
     * \param v3 The third value the vertex attribute should be set to
     * \param v4 The fourth value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, GLdouble v1, GLdouble v2, GLdouble v3,
        GLdouble v4);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL2dv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dvec2& value);

    /**
     * Sets the vertex attribute \p location to the passed \p value. Returns
     * <code>true</code> if the vertex attribute could be found; <code>false</code>
     * otherwise. Will call the OpenGL function <code>glVertexAttribL3dv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dvec3& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL4dv</code>.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dvec4& value);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib2fv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::mat2x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib3fv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::mat2x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib4fv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::mat2x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib2fv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::mat3x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib3fv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::mat3x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib4fv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::mat3x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib2fv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::mat4x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib3fv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::mat4x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttrib4fv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::mat4x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute \p location to the passed \p value. Will call the OpenGL
     * function <code>glVertexAttribL2dv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dmat2x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL3dv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dmat2x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL4dv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dmat2x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL2dv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dmat3x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL3dv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dmat3x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL4dv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dmat3x4& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL2dv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dmat4x2& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL3dv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dmat4x3& value,
        Transpose transpose = Transpose::No);

    /**
     * Sets the vertex attribute at the \p location to the passed \p value. Will call the
     * OpenGL function <code>glVertexAttribL4dv</code> on the rows/columns.
     *
     * \param location The location of the vertex attribute
     * \param value The value the vertex attribute should be set to
     * \param transpose Transpose::Yes if the matrix should be set in row major order,
     *        Transpose::No if the matrix is in column major order
     *
     * \pre \p location must not be <code>GL_INVALID_INDEX</code>
     */
    void setAttribute(GLuint location, const glm::dmat4x4& value,
        Transpose transpose = Transpose::No);

    //////////////////////////////////////////////////////////////////////////////////////
    ////// Attributes
    //////////////////////////////////////////////////////////////////////////////////////

    /**
     * Enables and disables the logging of a warning if the location of a subroutine can
     * not be found in any of the attached ShaderObjects.
     *
     * \param ignoreError Should the location error be ignored?
     */
    void setIgnoreSubroutineLocationError(IgnoreError ignoreError);

    /**
     * Returns the state of this ProgramObject if the logging of location errors should be
     * enabled or not.
     *
     * \return <code>true</code> if location errors will be ignored
     */
    bool ignoreSubroutineLocationError() const;

    /**
     * Enables and disables the logging of a warning if the location of a subroutine
     * uniform can not be found in any of the attached ShaderObjects.
     *
     * \param ignoreError Should the location error be ignored?
     */
    void setIgnoreSubroutineUniformLocationError(IgnoreError ignoreError);

    /**
     * Returns the state of this ProgramObject if the logging of location errors should be
     * enabled or not.
     *
     * \return <code>true</code> if location errors will be ignored
     */
    bool ignoreSubroutineUniformLocationError() const;

    /**
     * Returns the index of the subroutine with the name \p name inside the attached
     * shader object of type \p shaderType. If the subroutine could not be found,
     * <code>GL_INVALID_INDEX</code> will be returned and (if activated) a warning will be
     * logged. This will call the OpenGL function <code>glGetSubroutineIndex</code>.
     *
     * \param shaderType The type of the shader object that will be queried
     * \param name The name of the subroutine for which the index will be queried
     * \return The index of the subroutine
     *
     * \pre \p name must not be empty
     */
    GLuint subroutineIndex(ShaderObject::ShaderType shaderType, const std::string& name);

    /**
     * Returns the location of the subroutine uniform with the name \p name inside the
     * attached shader object of type \p shaderType. If the subroutine uniform could not
     * be found, <code>-1</code> will be returned and (if activated) a warning will be
     * logged. This will call the OpenGL function
     * <code>glGetSubroutineUniformLocation</code>.
     *
     * \param shaderType The type of the shader object that will be queried
     * \param name The name of the subroutine uniform for which the location will be
     *        queried
     * \return The location of the subroutine uniform
     *
     * \pre \p name must not be empty
     */
    GLint subroutineUniformLocation(ShaderObject::ShaderType shaderType,
        const std::string& name);

    /**
     * Returns all the names of the active subroutine uniforms in the attached shader
     * object of type \p shaderType. The returned names can be used in subsequent calls to
     * #subroutineUniformLocation. This will call the OpenGL function
     * <code>glGetActiveSubroutineUniformName</code>.
     *
     * \param shaderType The type of shader object that will be queried for the active
     *        subroutine uniform names
     * \return The list of all active subroutine uniform names in the attached shader
     *         object(s)
     */
    std::vector<std::string> activeSubroutineUniformNames(
        ShaderObject::ShaderType shaderType);

    /**
     * Returns the names of all subroutines that are compatible with the subroutine
     * uniform at location \p subroutineUniformLocation inside an attached shader of type
     * \p shaderType. This method uses a limited buffer for subroutine uniform names
     * (1024 characters) and will log a warning in debug mode if this limit is exceeded.
     * This will call the OpenGL function <code>glGetActiveSubroutineUniformiv</code>.
     *
     * \param shaderType The type of shader object that will be queried for the compatible
     *        subroutine uniform names
     * \param subroutineUniformLocation The location of the subroutine uniform location
     *        that will be used to determine the compatible subroutines
     * \return A list of names of all subroutines that are compatible with the subroutine
     *         uniform at \p subroutineUniformLocation
     *
     * \pre \p subroutineUniformLocation must not be <code>GL_INVALID_INDEX</code>
     */
    std::vector<std::string> compatibleSubroutineNames(
        ShaderObject::ShaderType shaderType, GLuint subroutineUniformLocation);

    /**
     * Returns the names of all subroutines that are compatible with the subroutine
     * uniform with the name \p subroutineUniformName inside an attached shader of type
     * \p shaderType. This will call the OpenGL function
     * <code>glGetActiveSubroutineUniformiv</code>.
     *
     * \param shaderType The type of shader object that will be queried for the compatible
     *        subroutine uniform names
     * \param subroutineUniformName The name of the subroutine uniform that will be used
     *        to determine the compatible subroutines
     * \return A list of names of all subroutines that are compatible with the subroutine
     *         uniform at \p subroutineUniformLocation
     *
     * \pre \p subroutineUniformName must not be empty
     */
    std::vector<std::string> compatibleSubroutineNames(
        ShaderObject::ShaderType shaderType, const std::string& subroutineUniformName);

    /**
     * Sets the subroutine indices of all available subroutine uniforms. The vector of
     * \p indices has to be in the same order as the locations of uniform locations in the
     * attached shader object of type \p shaderType and has to contain as many indices as
     * there are subroutine uniforms in the shader object. I.e., the uniform subroutine at
     * location <code>i</code> will be set with the value <code>indices[i]</code>. This
     * method will return <code>true</code> if the number of elements in
     * <code>indices</code> equals the number of subroutine uniforms in the attached
     * shader. The checks for valid input are only performed if the <code>GHL_DEBUG</code>
     * macro is set. This method will call the OpenGL function
     * <code>glUniformSubroutinesuiv</code>.
     *
     * \param shaderType The type of shader object that will be queried for the compatible
     *        subroutine uniform names
     * \param indices The list of subroutine indices that will be used to set all of the
     *        subroutine uniforms in the attached shader object
     * \return <code>true</code> if all uniform subroutines were set correctly,
     *         <code>false</code> otherwise, i.e., if \p indices did not contain the
     *         correct number of indices.
     *
     * \pre \p indices must not be empty
     */
    bool setUniformSubroutines(ShaderObject::ShaderType shaderType,
        const std::vector<GLuint>& indices);

    /**
     * Sets the subroutine indices of all available subroutine uniforms. The map
     * \p values contains pairs of (subroutine uniform name, subroutine name)
     * that will be used to set the appropriate state in the attached shader object of
     * type \p shaderType. Each active subroutine uniform name in this shader has to have
     * an entry in the map, and each entry in the map has to specify a uniform subroutine
     * name in that shader. The checks for valid input are only performed if the
     * <code>GHL_DEBUG</code> macro is set. This method will call the OpenGL function
     * <code>glUniformSubroutinesuiv</code>.
     *
     * \param shaderType The type of shader object that will be queried for the compatible
     *        subroutine uniform names
     * \param values The map of (subroutine uniform name, subroutine name) specifying
     *        which subroutine uniform should be set to which subroutine
     * \return <code>true</code> if all uniform subroutines were set correctly,
     *         <code>false</code> otherwise, i.e., a subroutine uniform or a subroutine
     *         was not found
     *
     * \pre \p indices must not be empty
     */
    bool setUniformSubroutines(ShaderObject::ShaderType shaderType,
        const std::map<std::string, std::string>& values);

    /**
     * Binds the frag data out variable \p name to the fragment shader color number
     * \p colorNumber. This will call the OpenGL function
     * <code>glBindFragDataLocation</code>.
     *
     * \param name The name of the output variable that is to be bound
     * \param colorNumber The color number that will be bound
     *
     * \pre \p name must not be empty
     * \pre \p colorNumber must not be <code>GL_INVALID_INDEX</code>
     */
    void bindFragDataLocation(const std::string& name, GLuint colorNumber);

private:
    /// The OpenGL name of this program object.
    GLuint _id = 0;

    /// The internal name of this program name, used for the object label and logging.
    std::string _programName;

    /// The logger category that will be used, if a name has been specified.
    std::string _loggerCat;

    /// <code>true</code> if uniform location errors should be ignored.
    bool _ignoreUniformLocationError = false;

    /// <code>true</code> if attribute location errors should be ignored.
    bool _ignoreAttributeLocationError = false;

    /// <code>true</code> if subroutine location errors should be ignored.
    bool _ignoreSubroutineLocationError = false;

    /// <code>true</code> if subroutine uniform location errors should be ignored.
    bool _ignoreSubroutineUniformLocationError = false;

    /// All the ShaderObjects that are managed and attached to this ProgramObject.
    std::vector<std::shared_ptr<ShaderObject>> _shaderObjects;

    /// A flag indicating whether the program needs to be recompiled
    /// due to a change in the shader source or in the dictionary.
    bool _programIsDirty = true;
};

} // namespace ghoul::opengl

#endif // __GHOUL___PROGRAMOBJECT___H__
