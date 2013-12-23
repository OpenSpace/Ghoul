/**************************************************************************************************
 * Variant                                                                              *
 *                                                                                                *
 * Copyright (c) 2012 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 *************************************************************************************************/

#ifndef __VARIANT_H__
#define __VARIANT_H__

#ifdef VRN_MODULE_PYTHON
#include <Python.h>
#endif

#include "voreen/core/io/serialization/serializable.h"
#include "voreen/core/voreencoredefine.h"
#include "voreen/core/utils/exception.h"

#include "tgt/matrix.h"
#include "tgt/vector.h"

#include <vector>
#include <string>

namespace tgt {
    class Camera;
}

namespace voreen {

class TransFunc;
class VolumeHandle;
class VolumeCollection;
class ShaderSource;

/**
 * This class is a basic wrapper for many common types so that they can be handled uniformly. The variant always
 * contains one value of specific type which can be inspected by getType(). <br>
 * <h3>Native types</h3>
 * The Variant supports a number of native types which are:  boolean, double, float, integer, long, std::string
 * tgt::ivec2, tgt::ivec3, tgt::ivec4, tgt::vec2, tgt::vec3, tgt::vec4, tgt::dvec2, tgt::dvec3, tgt::dvec4,
 * tgt::mat2, tgt::mat3, tgt::mat4, tgt::Camera, tgt::ShaderSource, VolumeHandle, VolumeCollection.
 * All types except tgt::Camera, tgt::ShaderSource, VolumeHandle and VolumeCollection are stored as copies inside
 * the Variant. The other types only store a pointer to the object because they are considered non-trivial to
 * copy.<br>
 * All these types have their own access methods and constructors and basic conversions are implemented:<br>
 * <table>
 *  <tr>
 *   <td></td><td>Bool</td><td>Double</td><td>Float</td><td>Integer</td><td>Long</td><td>String</td><td>ivecX</td>
 *   <td>vecX</td><td>dvecX</td><td>matX</td>
 *  </tr>
 *  <tr>
 *   <td>Bool</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>N</td><td>N</td><td>N</td><td>N</td>
 *  </tr>
 *  <tr>
 *   <td>Double</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>N</td><td>N</td><td>N</td><td>N</td>
 *  </tr>
 *  <tr>
 *   <td>Float</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>N</td><td>N</td><td>N</td><td>N</td>
 *  </tr>
 *  <tr>
 *   <td>Integer</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>N</td><td>N</td><td>N</td><td>N</td>
 *  </tr>
 *  <tr>
 *   <td>Long</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>N</td><td>N</td><td>N</td><td>N</td>
 *  </tr>
 *  <tr>
 *   <td>String</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td>
 *  </tr>
 *  <tr>
 *   <td>ivecX</td><td>N</td><td>N</td><td>N</td><td>N</td><td>N</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>N</td>
 *  </tr>
 *  <tr>
 *   <td>vecX</td><td>N</td><td>N</td><td>N</td><td>N</td><td>N</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>N</td>
 *  </tr>
 *  <tr>
 *   <td>dvecX</td><td>N</td><td>N</td><td>N</td><td>N</td><td>N</td><td>Y</td><td>Y</td><td>Y</td><td>Y</td><td>N</td>
 *  </tr>
 *  <tr>
 *   <td>matX</td><td>N</td><td>N</td><td>N</td><td>N</td><td>N</td><td>Y</td><td>N</td><td>N</td><td>N</td><td>Y</td>
 *  </tr>
 * </table>
 * All other types cannot be converted into any other type.
 * 
 * <h3>User Types</h3>
 * User types are added to a variatn with the methods Variant::get<T> and Variant::set<T>. Please note that only the
 * pointer of a user type will be stored and that there is neither the possibility for serialization or performing a
 * Variant::deepCopy. Furthermore, no conversion is possible between user types.<br>
 * Every user type should have its own VariantType number, beginning from Variant::VariantTypeUserType counting upwards.
 * Using this, it is guaranteed that the Variant(const Variant&), canConvert(VariantType, VariantType) and
 * operator=(const Variant&) methods work properly.
 */
class VRN_CORE_API Variant : public Serializable {
public:
    enum VariantType {
        // Warning: This list has to stay in the same order as the canConvertMatrix
        // and the numbers have to be in an uninterrupted, ascending order
        VariantTypeInvalid = 0,
        VariantTypeBool = 1,
        VariantTypeDouble = 2,
        VariantTypeFloat = 3,
        VariantTypeInteger = 4,
        VariantTypeLong = 5,
        VariantTypeString = 6,
        VariantTypeIVec2 = 7,
        VariantTypeIVec3 = 8,
        VariantTypeIVec4 = 9,
        VariantTypeVec2 = 10,
        VariantTypeVec3 = 11,
        VariantTypeVec4 = 12,
        VariantTypeDVec2 = 13,
        VariantTypeDVec3 = 14,
        VariantTypeDVec4 = 15,
        VariantTypeMat2 = 16,
        VariantTypeMat3 = 17,
        VariantTypeMat4 = 18,
        VariantTypeCamera = 19,
        VariantTypeShaderSource = 20,
        VariantTypeTransFunc = 21,
        VariantTypeVolumeHandle = 22,
        VariantTypeVolumeCollection = 23,
        VariantTypePythonObject = 24, // only used if the python module is compiled
        VariantTypeLastBaseType = VariantTypeVolumeCollection,
        VariantTypeUserType = 64
    };

    /**
     * This exception will be thrown if a conversion is executed which hasn't been implemented.
     */
    class NoSuchTransformationException : public VoreenException {
    public:
        NoSuchTransformationException(const std::string& what = "") : VoreenException(what) {}
    };

    /**
     * This exception is thrown if a conversion fails because of specific reasons instead of general
     * incompatibility. One example is string -> int. Although the conversion is theoretically possible
     * it will only work if the string contains an actual numerical value
     */
    class ConversionFailedException : public VoreenException {
    public:
        ConversionFailedException(const std::string& what = "") : VoreenException(what) {}
    };

    /**
     * This exception is thrown if an operation is executed on an invalid variant.
     */
    class OperationNotDefinedForInvalidVariantException : public VoreenException {
    public:
        OperationNotDefinedForInvalidVariantException(const std::string& what = "") : VoreenException(what) {}
    };

    /**
     * This constructor creates an empty, initially invalid Variant which can be filled later.
     */
    Variant() throw ();

    /**
    * This copy constructor creates a regular copy (i.e. deep copy for all types except tgt::Camera
    * tgt::ShaderSource, tgt::TransFunc, tgt::VolumeHandle and tgt::VolumeCollection and copying the
    * pointer for those types) of the passed Variant which is allowed to be invalid as well.
    * \param obj The original Variant which should be copied
    */
    Variant(const Variant& obj) throw ();

    explicit Variant(bool value) throw ();
    explicit Variant(double value) throw ();
    explicit Variant(float value) throw ();
    explicit Variant(int value) throw ();
    explicit Variant(long value) throw ();
    explicit Variant(const char* value) throw ();
    explicit Variant(const std::string& value) throw ();
    explicit Variant(const tgt::ivec2& value) throw ();
    explicit Variant(const tgt::ivec3& value) throw ();
    explicit Variant(const tgt::ivec4& value) throw ();
    explicit Variant(const tgt::vec2& value) throw ();
    explicit Variant(const tgt::vec3& value) throw ();
    explicit Variant(const tgt::vec4& value) throw ();
    explicit Variant(const tgt::dvec2& value) throw ();
    explicit Variant(const tgt::dvec3& value) throw ();
    explicit Variant(const tgt::dvec4& value) throw ();
    explicit Variant(const tgt::mat2& value) throw ();
    explicit Variant(const tgt::mat3& value) throw ();
    explicit Variant(const tgt::mat4& value) throw ();

    /**
     * Constructs a Variant from a ShaderSource. The passed object is not copied, but a pointer is stored instead.
     */
    explicit Variant(const ShaderSource* value) throw() ;

    /**
     * Constructs a Variant from a TransFunc. The passed object is not copied, but a pointer is stored instead.
     */
    explicit Variant(const TransFunc* value) throw ();

    /**
     * Constructs a Variant from a tgt::Camera. The passed object is not copied, but a pointer is stored instead.
     */
    explicit Variant(const tgt::Camera* value) throw ();

    /**
     * Constructs a Variant from a VolumeHandle. The passed object is not copied, but a pointer is stored instead.
     */
    explicit Variant(const VolumeHandle* value) throw ();

    /**
     * Constructs a Variant from a VolumeCollection. The passed object is not copied, but a pointer is stored instead.
     */
    explicit Variant(const VolumeCollection* value) throw ();

#ifdef VRN_MODULE_PYTHON
    /**
     * Constructs a Variant from a PyObject which stores the given type.
     *
     * \throw ConversionFailedException This exception is thrown if the PyObject did not contain a valid object
     * of type
     */
    explicit Variant(PyObject* obj, VariantType type) throw (VoreenException);

    /**
     * Constructs a Variant from a PyObject which stores the given type.
     *
     * \throw ConversionFailedException This exception is thrown if the PyObject did not contain a valid object
     * of type
     */
    explicit Variant(PyObject* obj, int type) throw (VoreenException);
#endif

    /**
     * Performs and returns a deep copy of the Variant. Will be the same as the copy constructor
     * in most cases, but will be different for ShaderSource, Transfunc, tgt::Camera, VolumeHandle, VolumeCollection.
     *
     * \return A deep copied version of this Variant
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if this method was called on
     * an invalid Variant, which is not defined behavior
     */
    Variant deepCopy() const throw (VoreenException);

    /**
     * The destructor will delete all values which are not stored as pointers only (i.e. ShaderSource, Transfunc,
     * tgt::Camera, VolumeHandle, and VolumeCollection will not be deleted if a Variant containing them is).
     */
    ~Variant();

    /**
     * Returns the data type of the value that is currently stored in the Variant.
     */
    VariantType getType() const throw ();

    /**
     * Returns true if the Variant contains a valid value.
     */
    bool isValid() const throw ();

    /**
     * Returns the string representation for the given VariantType. All user-defined VariantTypes will return
     * "user-defined" and the invalid Variant will return "<undefined>".
     */
    static std::string typeToName(VariantType type) throw ();

    /**
     * Returns the VariantType for the given type string. It the typeName doesn't match any VariantType,
     * VariantTypeInvalid is returned.
     */
    static VariantType nameToType(const std::string& typeName) throw ();

    /**
     * Will call canConvert(VariantType, VariantType) with <code>this</code> as the first parameter.
     */
    bool canConvert(VariantType t) const throw ();

    /**
     * Will call canConvert(VariantType, VariantType) with <code>this</code> as the first parameter.
     */
    bool canConvert(int t) const throw ();

    /**
     * Checks if VariantType <code>t</code> is convertible into VariantType <code>s</code>. Invalid
     * types are never convertible and user-defined types are only compatible with the same user-defined
     * type.
     */
    static bool canConvert(VariantType t, VariantType s) throw ();

    /**
     * Overloaded method for canConvert(VariantType, VariantType)
     */
    static bool canConvert(VariantType t, int s) throw ();

    /**
     * Overloaded method for canConvert(VariantType, VariantType)
     */
    static bool canConvert(int t, VariantType s) throw ();

    /**
     * Overloaded method for canConvert(VariantType, VariantType)
     */
    static bool canConvert(int t, int s) throw ();

    /**
     * Returns the value stored within this Variant as a bool and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    bool getBool() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a double and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    double getDouble() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a float and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    float getFloat() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as an int and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    int getInt() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a long and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    long getLong() const throw (VoreenException);

    /**
    * Returns the value stored within this Variant as a std::string and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    std::string getString() const throw (VoreenException);
    
    /**
     * Returns the value stored within this Variant as a tgt::ivec2 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::ivec2 getIVec2() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::ivec3 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::ivec3 getIVec3() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::ivec4 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::ivec4 getIVec4() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::vec2 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::vec2 getVec2() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::vec3 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::vec3 getVec3() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::vec4 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::vec4 getVec4() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::dvec2 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::dvec2 getDVec2() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::dvec3 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::dvec3 getDVec3() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::dvec4 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::dvec4 getDVec4() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::mat2 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::mat2 getMat2() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::mat3 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::mat3 getMat3() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::mat4 and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::mat4 getMat4() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a ShaderSource and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    ShaderSource* getShader() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a TransFunc and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    TransFunc* getTransFunc() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a tgt::Camera and performs a conversion first (if necessary).
     * If a conversion is necessary, a new object is created and returned so that the caller has the responsibility
     * to dispose of the object afterwards.
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    tgt::Camera* getCamera() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a VolumeHandle and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    VolumeHandle* getVolumeHandle() const throw (VoreenException);

    /**
     * Returns the value stored within this Variant as a VolumeCollection and performs a conversion first (if necessary)
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    VolumeCollection* getVolumeCollection() const throw (VoreenException);

#ifdef VRN_MODULE_PYTHON
    /**
     * Creates and returns a PyObject from the stored value.
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    PyObject* getPythonObject() const throw (VoreenException);

    /**
     * Returns the stored value as a std::string which is parseable by PyArg_ParseTuple
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if the method was called
     * on an invalid Variant, which is not defined behavior
     * \throw NoSuchTransformationException This exception is thrown if there is no possible transformation
     * between the stored type and the requested type.
     */
    std::string getPythonString() const throw (VoreenException);
#endif

    /// Stores the given bool in this variant and overwrites the old value
    void setBool(const bool& value) throw ();

    /// Stores the given double in this variant and overwrites the old value
    void setDouble(const double& value) throw ();

    /// Stores the given float in this variant and overwrites the old value
    void setFloat(const float& value) throw ();

    /// Stores the given int in this variant and overwrites the old value
    void setInt(const int& value) throw ();

    /// Stores the given long in this variant and overwrites the old value
    void setLong(const long& value) throw ();

    /// Stores the given std::string in this variant and overwrites the old value
    void setString(const std::string& value) throw ();

    /// Stores the given tgt::ivec2 in this variant and overwrites the old value
    void setIVec2(const tgt::ivec2& value) throw ();

    /// Stores the given tgt::ivec3 in this variant and overwrites the old value
    void setIVec3(const tgt::ivec3& value) throw ();

    /// Stores the given tgt::ivec4 in this variant and overwrites the old value
    void setIVec4(const tgt::ivec4& value) throw ();

    /// Stores the given tgt::vec2 in this variant and overwrites the old value
    void setVec2(const tgt::vec2& value) throw ();

    /// Stores the given tgt::vec3 in this variant and overwrites the old value
    void setVec3(const tgt::vec3& value) throw ();

    /// Stores the given tgt::vec4 in this variant and overwrites the old value
    void setVec4(const tgt::vec4& value) throw ();

    /// Stores the given tgt::dvec2 in this variant and overwrites the old value
    void setDVec2(const tgt::dvec2& value) throw ();

    /// Stores the given tgt::dvec3 in this variant and overwrites the old value
    void setDVec3(const tgt::dvec3& value) throw ();

    /// Stores the given tgt::dvec4 in this variant and overwrites the old value
    void setDVec4(const tgt::dvec4& value) throw ();

    /// Stores the given tgt::mat2 in this variant and overwrites the old value
    void setMat2(const tgt::mat2& value) throw ();

    /// Stores the given tgt::mat3 in this variant and overwrites the old value
    void setMat3(const tgt::mat3& value) throw ();

    /// Stores the given tgt::mat4 in this variant and overwrites the old value
    void setMat4(const tgt::mat4& value) throw ();

    /// Stores the given ShaderSource in this variant and overwrites the old value.
    /// The value is not copied, but only the pointer is stored
    void setShader(const ShaderSource* value) throw ();

    /// Stores the given TransFunc in this variant and overwrites the old value
    /// The value is not copied, but only the pointer is stored
    void setTransFunc(const TransFunc* value) throw ();

    /// Stores the given Camera in this variant and overwrites the old value
    /// The value is not copied, but only the pointer is stored
    void setCamera(const tgt::Camera* value) throw ();

    /// Stores the given VolumeHandle in this variant and overwrites the old value
    /// The value is not copied, but only the pointer is stored
    void setVolumeHandle(const VolumeHandle* value) throw ();

    /// Stores the given VolumeCollection in this variant and overwrites the old value
    /// The value is not copied, but only the pointer is stored
    void setVolumeCollection(const VolumeCollection* value) throw ();

    /**
     * Generic setter which should be used only for user-defined types. Only the pointer
     * is stored within this variant and not the complete data. Furthermore, this object
     * will not assume ownership and will therefore not delete the object if the Variant
     * is destroyed.
     */
    template<class T>
    inline void set(const T& value, VariantType type) throw ();

    /**
     * Overloaded method which will call set<T>(const T&, VariantType)
     */
    template<class T>
    inline void set(const T& value, int type) throw ();

    /**
     * Generic getter method which returns the stored value in the requested type. This
     * should only be used for user-defined types and does no type-checking.
     */
    template<class T>
    inline T get() const throw ();

    /**
     * Serializes the stored values together with the type. This only works for the native
     * types and not the user-defined types which CANNOT be serialized.
     *
     * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if an
     * invalid Variant is tried to be serialized
     * \throw NoSuchTransformationException This exception is thrown if a user-defined Variant
     * should be serialized
     */
    void serialize(XmlSerializer& s) const throw (VoreenException);

    /**
    * Deserializes the stored values. This only works for the native
    * types and not the user-defined types which CANNOT be serialized.
    *
    * \throw OperationNotDefinedForInvalidVariantException This exception is thrown if an
    * invalid Variant is tried to be deserialized
    * \throw NoSuchTransformationException This exception is thrown if a user-defined Variant
    * should be deserialized
    */
    void deserialize(XmlDeserializer& d) throw (VoreenException);

    /// Sets this Variant based on the information stored in the right-hand-side Variant
    Variant& operator= (const Variant& rhs);

    /// Stores the given bool in this variant and overwrites the old value
    Variant& operator= (const bool& rhs) throw ();

    /// Stores the given double in this variant and overwrites the old value
    Variant& operator= (const double& rhs) throw ();

    /// Stores the given float in this variant and overwrites the old value
    Variant& operator= (const float& rhs) throw ();

    /// Stores the given int in this variant and overwrites the old value
    Variant& operator= (const int& rhs) throw ();

    /// Stores the given long in this variant and overwrites the old value
    Variant& operator= (const long& rhs) throw ();

    /// Stores the given const char* as a string in this Variant and overwrites the old value
    Variant& operator= (const char* rhs) throw ();

    /// Stores the given std::string in this variant and overwrites the old value
    Variant& operator= (const std::string& rhs) throw ();

    /// Stores the given tgt::ivec2 in this variant and overwrites the old value
    Variant& operator= (const tgt::ivec2& rhs) throw ();

    /// Stores the given tgt::ivec3 in this variant and overwrites the old value
    Variant& operator= (const tgt::ivec3& rhs) throw ();

    /// Stores the given tgt::ivec4 in this variant and overwrites the old value
    Variant& operator= (const tgt::ivec4& rhs) throw ();

    /// Stores the given tgt::vec2 in this variant and overwrites the old value
    Variant& operator= (const tgt::vec2& rhs) throw ();

    /// Stores the given tgt::vec3 in this variant and overwrites the old value
    Variant& operator= (const tgt::vec3& rhs) throw ();

    /// Stores the given tgt::vec4 in this variant and overwrites the old value
    Variant& operator= (const tgt::vec4& rhs) throw ();

    /// Stores the given tgt::dvec2 in this variant and overwrites the old value
    Variant& operator= (const tgt::dvec2& rhs) throw ();

    /// Stores the given tgt::dvec3 in this variant and overwrites the old value
    Variant& operator= (const tgt::dvec3& rhs) throw ();

    /// Stores the given tgt::dvec4 in this variant and overwrites the old value
    Variant& operator= (const tgt::dvec4& rhs) throw ();

    /// Stores the given tgt::mat2 in this variant and overwrites the old value
    Variant& operator= (const tgt::mat2& rhs) throw ();

    /// Stores the given tgt::mat3 in this variant and overwrites the old value
    Variant& operator= (const tgt::mat3& rhs) throw ();

    /// Stores the given tgt::mat4 in this variant and overwrites the old value
    Variant& operator= (const tgt::mat4& rhs) throw ();

    /// Stores the given ShaderSource in this variant and overwrites the old value
    /// The value is not copied, but only the pointer is stored
    Variant& operator= (const ShaderSource* rhs) throw ();

    /// Stores the given TransFunc in this variant and overwrites the old value
    /// The value is not copied, but only the pointer is stored
    Variant& operator= (const TransFunc* rhs) throw ();

    /// Stores the given tgt::Camera in this variant and overwrites the old value
    /// The value is not copied, but only the pointer is stored
    Variant& operator= (const tgt::Camera* rhs) throw ();

    /// Stores the given VolumeHandle in this variant and overwrites the old value
    /// The value is not copied, but only the pointer is stored
    Variant& operator= (const VolumeHandle* rhs) throw ();

    /// Stores the given VolumeCollection in this variant and overwrites the old value
    /// The value is not copied, but only the pointer is stored
    Variant& operator= (const VolumeCollection* rhs) throw ();

    /**
     * Compares the two Variants with each other. All types except VolumeHandle, VolumeCollection
     * and user-defined types are compared by value. TransFunc is only compared by value if both
     * Variants contain TransFuncIntensity objects, because the TransFunc class doesn't define
     * an operator==. If either of the TransFunc objects is no TransFuncIntensity, the pointers
     * are compared instead.
     */
    bool operator== (const Variant& rhs) const throw ();

    /// Compares the two Variants for inequality. Is always the inverse of operator==(const Variant&)
    bool operator!= (const Variant& rhs) const throw ();

private:
    /// Frees the currently stored value, if necessary.
    void deleteValue();

    void set(const ShaderSource& value, VariantType type);
    void set(const TransFunc& value, VariantType type);
    void set(const tgt::Camera& value, VariantType type);
    void set(const VolumeHandle& value, VariantType type);
    void set(const VolumeCollection& value, VariantType type);

#ifdef VRN_MODULE_PYTHON
    void setPython(PyObject* obj, VariantType type) throw (VoreenException);
#endif

    template<class T>
    const std::string toString(const T& value) const;

    void* value_;                           ///< The currently stored value.
    VariantType currentType_;               ///< The currently stored type as an int to allow for UserTypes
};


//-------------------------------------------------------------------------------------------------

#define VP(a) (*(a*)value_)   //< value pointer; casts the void* value_ into an instance of a

template<class T>
void Variant::set(const T& value, VariantType type) throw () {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    if (value_ == 0) {
        value_ = new T;
    }
    *(T*)value_ = value;
}

template<class T>
void Variant::set(const T& value, int type) throw () {
    set<T>(value, VariantType(type));
}

template<class T>
T Variant::get() const throw () {
    return VP(T);
}

} // namespace

#endif // VRN_VARIANT_H
