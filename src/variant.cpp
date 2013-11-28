/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Created between 2005 and 2012 by The Voreen Team                   *
 * as listed in CREDITS.TXT <http://www.voreen.org>                   *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/utils/variant.h"

#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/datastructures/transfunc/transfuncintensity.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/datastructures/volume/volumecollection.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/volumehandleproperty.h"
#include "voreen/core/utils/exception.h"
#include "voreen/core/datastructures/transfunc/transfuncfactory.h"

#include "tgt/camera.h"
#include "tgt/types.h"

#include <sstream>

using std::string;
using std::vector;
using std::pair;
using std::stringstream;
using tgt::col4;
using tgt::vec2;
using tgt::vec3;
using tgt::vec4;
using tgt::ivec2;
using tgt::ivec3;
using tgt::ivec4;
using tgt::dvec2;
using tgt::dvec3;
using tgt::dvec4;
using tgt::mat2;
using tgt::mat3;
using tgt::mat4;
using tgt::Camera;

#define IVEC2STRINGFORMAT "( %i %i )"
#define IVEC3STRINGFORMAT "( %i %i %i )"
#define IVEC4STRINGFORMAT "( %i %i %i %i )"
#define VEC2STRINGFORMAT "( %f %f )"
#define VEC3STRINGFORMAT "( %f %f %f )"
#define VEC4STRINGFORMAT "( %f %f %f %f )"
#define DVEC2STRINGFORMAT "( %f %f )"
#define DVEC3STRINGFORMAT "( %f %f %f )"
#define DVEC4STRINGFORMAT "( %f %f %f %f )"
#define MAT2STRINGFORMAT "( ( %f %f ) ( %f %f ) )" // row-major
#define MAT3STRINGFORMAT "( ( %f %f %f ) ( %f %f %f ) ( %f %f %f ) )" // row-major
#define MAT4STRINGFORMAT "( ( %f %f %f %f ) ( %f %f %f %f ) ( %f %f %f %f ) ( %f %f %f %f ) )" // row-major
#define CAMERASTRINGFORMAT "( ( %f %f %f ) ( %f %f %f ) ( %f %f %f ) )"  // [position] [focus] [up]
// pseudo-transfer function format:   [# ( %f %i %i %i %i %i %i %i ) ... ]    number ( intensity [color] isSplit [rightColor] )
#define VALUETOSTRINGBUFFERSIZE 2048

namespace voreen {

static const int32_t canConvertMatrix[Variant::VariantTypeLastBaseType + 1] =
{
//  Matrix for checking if the transformation
//  Left side -> Right side 
//  is allowed in general
//  WARNING: This list has to be in the same
//  order as the enum VariantType
/*Invalid*/             0,

/*Bool*/                1 << Variant::VariantTypeDouble     | 1 << Variant::VariantTypeFloat    | 1 << Variant::VariantTypeInteger  |
                        1 << Variant::VariantTypeLong       | 1 << Variant::VariantTypeString   | 1 << Variant::VariantTypePythonObject,

/*Double*/              1 << Variant::VariantTypeBool       | 1 << Variant::VariantTypeFloat    | 1 << Variant::VariantTypeInteger  |
                        1 << Variant::VariantTypeLong       | 1 << Variant::VariantTypeString   | 1 << Variant::VariantTypePythonObject,

/*Float*/               1 << Variant::VariantTypeBool       | 1 << Variant::VariantTypeDouble   | 1 << Variant::VariantTypeInteger  |
                        1 << Variant::VariantTypeLong       | 1 << Variant::VariantTypeString   | 1 << Variant::VariantTypePythonObject,

/*Integer*/             1 << Variant::VariantTypeBool       | 1 << Variant::VariantTypeDouble   | 1 << Variant::VariantTypeFloat    |
                        1 << Variant::VariantTypeLong       | 1 << Variant::VariantTypeString   | 1 << Variant::VariantTypePythonObject,

/*Long*/                1 << Variant::VariantTypeBool       | 1 << Variant::VariantTypeDouble   | 1 << Variant::VariantTypeFloat    |
                        1 << Variant::VariantTypeInteger    | 1 << Variant::VariantTypeString   | 1 << Variant::VariantTypePythonObject,

/*String*/              1 << Variant::VariantTypeBool       | 1 << Variant::VariantTypeDouble   | 1 << Variant::VariantTypeFloat    |
                        1 << Variant::VariantTypeInteger    | 1 << Variant::VariantTypeLong     | 1 << Variant::VariantTypeIVec2    |
                        1 << Variant::VariantTypeIVec3      | 1 << Variant::VariantTypeIVec4    | 1 << Variant::VariantTypeVec2     |
                        1 << Variant::VariantTypeVec3       | 1 << Variant::VariantTypeVec4     | 1 << Variant::VariantTypeDVec2    |
                        1 << Variant::VariantTypeDVec3      | 1 << Variant::VariantTypeDVec4    | 1 << Variant::VariantTypeMat2     |
                        1 << Variant::VariantTypeMat3       | 1 << Variant::VariantTypeMat4     | 1 << Variant::VariantTypeCamera   |
                        1 << Variant::VariantTypeTransFunc  | 1  << Variant::VariantTypePythonObject,

/*IVec2*/               1 << Variant::VariantTypeIVec3      | 1 << Variant::VariantTypeIVec4    | 1 << Variant::VariantTypeVec2     |
                        1 << Variant::VariantTypeVec3       | 1 << Variant::VariantTypeVec4     | 1 << Variant::VariantTypeDVec2    |
                        1 << Variant::VariantTypeDVec3      | 1 << Variant::VariantTypeDVec4    | 1 << Variant::VariantTypeString   |
                        1 << Variant::VariantTypePythonObject,

/*IVec3*/               1 << Variant::VariantTypeIVec2      | 1 << Variant::VariantTypeIVec4    | 1 << Variant::VariantTypeVec2     |
                        1 << Variant::VariantTypeVec3       | 1 << Variant::VariantTypeVec4     | 1 << Variant::VariantTypeDVec2    |
                        1 << Variant::VariantTypeDVec3      | 1 << Variant::VariantTypeDVec4    | 1 << Variant::VariantTypeString   |
                        1 << Variant::VariantTypePythonObject,

/*IVec4*/               1 << Variant::VariantTypeIVec2      | 1 << Variant::VariantTypeIVec3    | 1 << Variant::VariantTypeVec2     |
                        1 << Variant::VariantTypeVec3       | 1 << Variant::VariantTypeVec4     | 1 << Variant::VariantTypeDVec2    |
                        1 << Variant::VariantTypeDVec3      | 1 << Variant::VariantTypeDVec4    | 1 << Variant::VariantTypeString   |
                        1 << Variant::VariantTypePythonObject,

/*Vec2*/                1 << Variant::VariantTypeIVec2      | 1 << Variant::VariantTypeIVec3    | 1 << Variant::VariantTypeIVec4    |
                        1 << Variant::VariantTypeVec3       | 1 << Variant::VariantTypeVec4     | 1 << Variant::VariantTypeDVec2    |
                        1 << Variant::VariantTypeDVec3      | 1 << Variant::VariantTypeDVec4    | 1 << Variant::VariantTypeString   |
                        1 << Variant::VariantTypePythonObject,

/*Vec3*/                1 << Variant::VariantTypeIVec2      | 1 << Variant::VariantTypeIVec3    | 1 << Variant::VariantTypeIVec4    |
                        1 << Variant::VariantTypeVec2       | 1 << Variant::VariantTypeVec4     | 1 << Variant::VariantTypeDVec2    |
                        1 << Variant::VariantTypeDVec3      | 1 << Variant::VariantTypeDVec4    | 1 << Variant::VariantTypeString   |
                        1 << Variant::VariantTypePythonObject,

/*Vec4*/                1 << Variant::VariantTypeIVec2      | 1 << Variant::VariantTypeIVec3    | 1 << Variant::VariantTypeIVec4    |
                        1 << Variant::VariantTypeVec2       | 1 << Variant::VariantTypeVec3     | 1 << Variant::VariantTypeDVec2    |
                        1 << Variant::VariantTypeDVec3      | 1 << Variant::VariantTypeDVec4    | 1 << Variant::VariantTypeString   |

/*DVec2*/               1 << Variant::VariantTypeIVec2      | 1 << Variant::VariantTypeIVec3    | 1 << Variant::VariantTypeIVec4    |
                        1 << Variant::VariantTypeVec2       | 1 << Variant::VariantTypeVec3     | 1 << Variant::VariantTypeVec4     |
                        1 << Variant::VariantTypeDVec3      | 1 << Variant::VariantTypeDVec4    | 1 << Variant::VariantTypeString   |
                        1 << Variant::VariantTypePythonObject,

/*DVec3*/               1 << Variant::VariantTypeIVec2      | 1 << Variant::VariantTypeIVec3    | 1 << Variant::VariantTypeIVec4    |
                        1 << Variant::VariantTypeVec2       | 1 << Variant::VariantTypeVec3     | 1 << Variant::VariantTypeVec4     |
                        1 << Variant::VariantTypeDVec2      | 1 << Variant::VariantTypeDVec4    | 1 << Variant::VariantTypeString   |
                        1 << Variant::VariantTypePythonObject,

/*DVec4*/               1 << Variant::VariantTypeIVec2      | 1 << Variant::VariantTypeIVec3    | 1 << Variant::VariantTypeIVec4    |
                        1 << Variant::VariantTypeVec2       | 1 << Variant::VariantTypeVec3     | 1 << Variant::VariantTypeVec4     |
                        1 << Variant::VariantTypeDVec2      | 1 << Variant::VariantTypeDVec3    | 1 << Variant::VariantTypeString   |
                        1 << Variant::VariantTypePythonObject,

/*Mat2*/                1 << Variant::VariantTypeString     | 1 << Variant::VariantTypePythonObject,

/*Mat3*/                1 << Variant::VariantTypeString     | 1 << Variant::VariantTypePythonObject,

/*Mat4*/                1 << Variant::VariantTypeString     | 1 << Variant::VariantTypePythonObject,

/*Camera*/              1 << Variant::VariantTypeString     | 1 << Variant::VariantTypePythonObject,

/*Shader*/              0,

/*TransFunc*/           1 << Variant::VariantTypeString     | 1 << Variant::VariantTypePythonObject,

/*VolumeHandle*/        0,

/*VolumeCollection*/    0,

/*PyObject*/            0
};

void Variant::set(const ShaderSource& value, VariantType type) {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    value_ = (void*)&value;
}

void Variant::set(const TransFunc& value, VariantType type) {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    value_ = (void*)&value;
}

void Variant::set(const Camera& value, VariantType type) {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    value_ = (void*)&value;
}

void Variant::set(const VolumeHandle& value, VariantType type) {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    value_ = (void*)&value;
}

void Variant::set(const VolumeCollection& value, VariantType type) {
    if (type != currentType_) {
        deleteValue();
        currentType_ = type;
    }
    value_ = (void*)&value;
}

#ifdef VRN_MODULE_PYTHON
void Variant::setPython(PyObject* obj, VariantType type) throw (VoreenException) {
    switch (type) {
    case VariantTypeBool:
        {
            bool s = static_cast<bool>(PyInt_AsLong(obj));
            if (PyErr_Occurred())
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            else
                set<bool>(s, type);
            break;
        }
    case VariantTypeDouble:
        {
            double s = PyFloat_AsDouble(obj);
            if (PyErr_Occurred())
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            else
                set<double>(s, type);
            break;
        }
    case VariantTypeFloat:
        {
            float s = static_cast<float>(PyFloat_AsDouble(obj));
            if (PyErr_Occurred())
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            else
                set<float>(s, type);
            break;
        }
    case VariantTypeInteger:
        {
            int s = static_cast<int>(PyInt_AsLong(obj));
            if (PyErr_Occurred())
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            else
                set<int>(s, type);
            break;
        }
    case VariantTypeLong:
        {
            long s = PyInt_AsLong(obj);
            if (PyErr_Occurred())
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            else
                set<long>(s, type);
            break;
        }
    case VariantTypeString:
        {
            char* s = PyString_AsString(obj);
            if (PyErr_Occurred())
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            else
                set<string>(string(s), type);
            break;
        }
    case VariantTypeIVec2:
        {
            ivec2 v;
            bool success = PyArg_ParseTuple(obj, "ii" , &v.x, &v.y);
            if (success)
                set<ivec2>(v, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeIVec3:
        {
            ivec3 v;
            bool success = PyArg_ParseTuple(obj, "iii", &v.x, &v.y, &v.z);
            if (success)
                set<ivec3>(v, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeIVec4:
        {
            ivec4 v;
            bool success = PyArg_ParseTuple(obj, "iiii", &v.x, &v.y, &v.z, &v.w);
            if (success)
                set<ivec4>(v, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeVec2:
        {
            vec2 v;
            bool success = PyArg_ParseTuple(obj, "fff", &v.x, &v.y);
            if (success)
                set<vec2>(v, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeVec3:
        {
            vec3 v;
            bool success = PyArg_ParseTuple(obj, "fff", &v.x, &v.y, &v.z);
            if (success)
                set<vec3>(v, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeVec4:
        {
            vec4 v;
            bool success = PyArg_ParseTuple(obj, "ffff", &v.x, &v.y, &v.z, &v.w);
            if (success)
                set<vec4>(v, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeDVec2:
        {
            dvec2 v;
            bool success = PyArg_ParseTuple(obj, "dd", &v.x, &v.y);
            if (success)
                set<dvec2>(v, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeDVec3:
        {
            dvec3 v;
            bool success = PyArg_ParseTuple(obj, "ddd", &v.x, &v.y, &v.z);
            if (success)
                set<dvec3>(v, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeDVec4:
        {
            dvec4 v;
            bool success = PyArg_ParseTuple(obj, "dddd", &v.x, &v.y, &v.z, &v.w);
            if (success)
                set<dvec4>(v, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeMat2:
        {
            mat2 m;
            bool success = PyArg_ParseTuple(obj, "(ff)(ff)", &m[0][0], &m[0][1],
                                                               &m[1][0], &m[1][1]);
            if (success)
                set<mat2>(m, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeMat3:
        {
            mat3 m;
            bool success = PyArg_ParseTuple(obj, "(fff)(fff)(fff)", &m[0][0], &m[0][1], &m[0][2],
                                                                      &m[1][0], &m[1][1], &m[1][2],
                                                                      &m[2][0], &m[2][1], &m[2][2]);
            if (success)
                set<mat3>(m, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeMat4:
        {
            mat4 m;
            bool success = PyArg_ParseTuple(obj, "(ffff)(ffff)(ffff)(ffff)", &m[0][0], &m[0][1], &m[0][2], &m[0][3],
                                                                               &m[1][0], &m[1][1], &m[1][2], &m[1][3],
                                                                               &m[2][0], &m[2][1], &m[2][2], &m[2][3],
                                                                               &m[3][0], &m[3][1], &m[3][2], &m[3][3]);
            if (success)
                set<mat4>(m, type);
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeCamera:
        {
            vec3 position, focus, upVector;
            bool success = PyArg_ParseTuple(obj, "(fff)(fff)(fff)", &position.x, &position.y, &position.z,
                                                                      &focus.x, &focus.y, &focus.z,
                                                                      &upVector.x, &upVector.y, &upVector.z);
            if (success) {
                Camera* camera = new Camera(position, focus, upVector);
                set<Camera>(*camera, type);
            }
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeTransFunc:
        {
            int numKeys = PyTuple_Size(obj) - 1;
            if (PyTuple_Size(obj) > 0) {
                PyObject* widthObj = PyTuple_GetItem(obj, 0);
                int width = static_cast<int>(PyInt_AsLong(widthObj));
                TransFuncIntensity* transFunc = new TransFuncIntensity(width);
                transFunc->clearKeys();
                for (int i = 1; i <= numKeys; ++i) {
                    PyObject* currentObj = PyTuple_GetItem(obj, i);
                    float intensity;
                    int colorLR, colorLG, colorLB, colorLA, colorRR, colorRG, colorRB, colorRA;
                    int isSplit;
                    bool success = PyArg_ParseTuple(currentObj, "fiiiiiiiii", 
                        &intensity,
                        &colorLR, &colorLG, &colorLB, &colorLA,
                        &isSplit,
                        &colorRR, &colorRG, &colorRB, &colorRA);
                    if (success) {
                        col4 colorL(static_cast<uint8_t>(colorLR), static_cast<uint8_t>(colorLG), static_cast<uint8_t>(colorLB), static_cast<uint8_t>(colorLA));
                        TransFuncMappingKey* mappingKey = new TransFuncMappingKey(intensity, colorL);
                        if (static_cast<bool>(isSplit)) {
                            col4 colorR(static_cast<uint8_t>(colorRR), static_cast<uint8_t>(colorRG), static_cast<uint8_t>(colorRB), static_cast<uint8_t>(colorRA));
                            mappingKey->setSplit(true);
                            mappingKey->setColorR(colorR);
                        }
                        transFunc->addKey(mappingKey);

                    }
                    else
                        throw ConversionFailedException("Error occurred while parsing PyObject for TransFuncMappingKey");
                }
                set(*transFunc, VariantTypeTransFunc);
            }
            else
                throw ConversionFailedException("PyObject did not contain a Variant of type " + type);
            break;
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from PyObject to " + typeToName(currentType_) + " not implemented");
    }
}
#endif

template<class T>
const string Variant::toString(const T& value) const {
    stringstream s;
    string result;
    s << value;
    s >> result;
    return result;
}

Variant::Variant() throw ()
    : Serializable()
    , value_(0)
    , currentType_(VariantTypeInvalid)
{}

Variant::Variant(const Variant& obj) throw ()
    : Serializable()
    , value_(0)
{
    switch (obj.getType()) {
    case VariantTypeInvalid:
        currentType_ = VariantTypeInvalid;
        break;
    case VariantTypeBool:
        set<bool>(obj.getBool(), VariantTypeBool);
        break;
    case VariantTypeCamera:
        set(*(obj.getCamera()), VariantTypeCamera);
        break;
    case VariantTypeDouble:
        set<double>(obj.getDouble(), VariantTypeDouble);
        break;
    case VariantTypeFloat:
        set<float>(obj.getFloat(), VariantTypeFloat);
        break;
    case VariantTypeInteger:
        set<int>(obj.getInt(), VariantTypeInteger);
        break;
    case VariantTypeLong:
        set<long>(obj.getLong(), VariantTypeLong);
        break;
    case VariantTypeString:
        set<string>(obj.getString(), VariantTypeString);
        break;
    case VariantTypeIVec2:
        set<ivec2>(obj.getIVec2(), VariantTypeIVec2);
        break;
    case VariantTypeIVec3:
        set<ivec3>(obj.getIVec3(), VariantTypeIVec3);
        break;
    case VariantTypeIVec4:
        set<ivec4>(obj.getIVec4(), VariantTypeIVec4);
        break;
    case VariantTypeVec2:
        set<vec2>(obj.getVec2(), VariantTypeVec2);
        break;
    case VariantTypeVec3:
        set<vec3>(obj.getVec3(), VariantTypeVec3);
        break;
    case VariantTypeVec4:
        set<vec4>(obj.getVec4(), VariantTypeVec4);
        break;
    case VariantTypeDVec2:
        set<dvec2>(obj.getDVec2(), VariantTypeDVec2);
        break;
    case VariantTypeDVec3:
        set<dvec3>(obj.getDVec3(), VariantTypeDVec3);
        break;
    case VariantTypeDVec4:
        set<dvec4>(obj.getDVec4(), VariantTypeDVec4);
        break;
    case VariantTypeMat2:
        set<mat2>(obj.getMat2(), VariantTypeMat2);
        break;
    case VariantTypeMat3:
        set<mat3>(obj.getMat3(), VariantTypeMat3);
        break;
    case VariantTypeMat4:
        set<mat4>(obj.getMat4(), VariantTypeMat4);
        break;
    case VariantTypeShaderSource:
        set(*obj.getShader(), VariantTypeShaderSource);
        break;
    case VariantTypeTransFunc:
        set(*obj.getTransFunc(), VariantTypeTransFunc);
        break;
    case VariantTypeVolumeHandle:
        set(*obj.getVolumeHandle(), VariantTypeVolumeHandle);
        break;
    case VariantTypeVolumeCollection:
        set(*obj.getVolumeCollection(), VariantTypeVolumeCollection);
        break;
    default:
        ;
    }
    if (obj.getType() >= VariantTypeUserType) {
        currentType_ = obj.currentType_;
        value_ = obj.value_;
    }
}

Variant::Variant(bool value) throw () : value_(0), currentType_(VariantTypeBool) {
    set<bool>(value, VariantTypeBool);
}

Variant::Variant(const Camera* value) throw () : value_(0), currentType_(VariantTypeCamera) {
    set(*value, VariantTypeCamera);
}

Variant::Variant(double value) throw () : value_(0), currentType_(VariantTypeDouble) {
    set<double>(value, VariantTypeDouble);
}

Variant::Variant(float value) throw () : value_(0), currentType_(VariantTypeFloat) {
    set<float>(value, VariantTypeFloat);
}

Variant::Variant(int value) throw () : value_(0), currentType_(VariantTypeInteger) {
    set<int>(value, VariantTypeInteger);
}

Variant::Variant(long value) throw () : value_(0), currentType_(VariantTypeLong) {
    set<long>(value, VariantTypeLong);
}

Variant::Variant(const ShaderSource* value) throw () : value_(0), currentType_(VariantTypeShaderSource) {
    set(*value, VariantTypeShaderSource);
}

Variant::Variant(const char* value) throw () : value_(0), currentType_(VariantTypeString) {
    set<string>(string(value), VariantTypeString);
}

Variant::Variant(const string& value) throw () : value_(0), currentType_(VariantTypeString) {
    set<string>(value, VariantTypeString);
}

Variant::Variant(const ivec2& value) throw () : value_(0), currentType_(VariantTypeIVec2) {
    set<ivec2>(value, VariantTypeIVec2);
}

Variant::Variant(const ivec3& value) throw () : value_(0), currentType_(VariantTypeIVec3) {
    set<ivec3>(value, VariantTypeIVec3);
}

Variant::Variant(const ivec4& value) throw () : value_(0), currentType_(VariantTypeIVec4) {
    set<ivec4>(value, VariantTypeIVec4);
}

Variant::Variant(const TransFunc* value) throw () : value_(0), currentType_(VariantTypeTransFunc) {
    set(*value, VariantTypeTransFunc);
}

Variant::Variant(const vec2& value) throw () : value_(0), currentType_(VariantTypeVec2) {
    set<vec2>(value, VariantTypeVec2);
}

Variant::Variant(const vec3& value) throw () : value_(0), currentType_(VariantTypeVec3) {
    set<vec3>(value, VariantTypeVec3);
}

Variant::Variant(const vec4& value) throw () : value_(0), currentType_(VariantTypeVec4) {
    set<vec4>(value, VariantTypeVec4);
}

Variant::Variant(const dvec2& value) throw () : value_(0), currentType_(VariantTypeDVec2) {
    set<dvec2>(value, VariantTypeDVec2);
}

Variant::Variant(const dvec3& value) throw () : value_(0), currentType_(VariantTypeDVec3) {
    set<dvec3>(value, VariantTypeDVec3);
}

Variant::Variant(const dvec4& value) throw () : value_(0), currentType_(VariantTypeDVec4) {
    set<dvec4>(value, VariantTypeDVec4);
}

Variant::Variant(const mat2& value) throw () : value_(0), currentType_(VariantTypeMat2) {
    set<mat2>(value, VariantTypeMat2);
}

Variant::Variant(const mat3& value) throw () : value_(0), currentType_(VariantTypeMat3) {
    set<mat3>(value, VariantTypeMat3);
}

Variant::Variant(const mat4& value) throw () : value_(0), currentType_(VariantTypeMat4) {
    set<mat4>(value, VariantTypeMat4);
}

Variant::Variant(const VolumeHandle* value) throw () : value_(0), currentType_(VariantTypeVolumeHandle) {
    set(*value, VariantTypeVolumeHandle);
}

Variant::Variant(const VolumeCollection* value) throw () : value_(0), currentType_(VariantTypeVolumeCollection) {
    set(*value, VariantTypeVolumeCollection);
}

#ifdef VRN_MODULE_PYTHON
Variant::Variant(PyObject* obj, VariantType type) throw (VoreenException)
    : value_(0)
    , currentType_(type)
{
    setPython(obj, type);
}

Variant::Variant(PyObject* obj, int type) throw (VoreenException)
    : value_(0)
    , currentType_(VariantType(type))
{
    setPython(obj, VariantType(type));
}
#endif

Variant Variant::deepCopy() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeTransFunc:
        {
            const TransFunc* trans = getTransFunc();
            TransFunc* result = trans->clone();
            return Variant(result);
        }
    case VariantTypeCamera:
        {
            const Camera* camera = getCamera();
            Camera* result = camera->clone();
            return Variant(result);
        }
    case VariantTypeShaderSource:
        {
            const ShaderSource* shader = getShader();
            const ShaderSource* result = new ShaderSource(*shader);
            return Variant(result);
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException("Tried to create a deep copy of an invalid variant");
    default:
        return Variant(*this);
    }
}

Variant::~Variant() {
    deleteValue();
}

void Variant::deleteValue() {
    if (value_ != 0) {
        switch (currentType_) {
        case VariantTypeInvalid:
            break;
        case VariantTypeBool:
            delete static_cast<bool*>(value_);
            break;
        case VariantTypeCamera:
            //delete static_cast<Camera*>(value_);
            break;
        case VariantTypeDouble:
            delete static_cast<double*>(value_);
            break;
        case VariantTypeFloat:
            delete static_cast<float*>(value_);
            break;
        case VariantTypeInteger:
            delete static_cast<int*>(value_);
            break;
        case VariantTypeLong:
            delete static_cast<long*>(value_);
            break;
        case VariantTypeShaderSource:
            //delete static_cast<ShaderSource*>(value_);
            break;
        case VariantTypeString:
            delete static_cast<string*>(value_);
            break;
        case VariantTypeIVec2:
            delete static_cast<ivec2*>(value_);
            break;
        case VariantTypeIVec3:
            delete static_cast<ivec3*>(value_);
            break;
        case VariantTypeIVec4:
            delete static_cast<ivec4*>(value_);
            break;
        case VariantTypeTransFunc:
            //delete static_cast<TransFuncIntensity*>(value_);
            break;
        case VariantTypeVec2:
            delete static_cast<vec2*>(value_);
            break;
        case VariantTypeVec3:
            delete static_cast<vec3*>(value_);
            break;
        case VariantTypeVec4:
            delete static_cast<vec4*>(value_);
            break;
        case VariantTypeDVec2:
            delete static_cast<dvec2*>(value_);
            break;
        case VariantTypeDVec3:
            delete static_cast<dvec3*>(value_);
            break;
        case VariantTypeDVec4:
            delete static_cast<dvec4*>(value_);
            break;
        case VariantTypeMat2:
            delete static_cast<mat2*>(value_);
            break;
        case VariantTypeMat3:
            delete static_cast<mat3*>(value_);
            break;
        case VariantTypeMat4:
            delete static_cast<mat4*>(value_);
            break;
        case VariantTypeVolumeHandle:
            //delete static_cast<VolumeHandle*>(value_);
            break;
        case VariantTypeVolumeCollection:
            //delete static_cast<VolumeCollection*>(value_);
            break;
        default:
            ;
        }
        value_ = 0;
    }
}

Variant::VariantType Variant::getType() const throw () {
    return currentType_;
}

bool Variant::isValid() const throw () {
    return getType() != VariantTypeInvalid;
}

std::string Variant::typeToName(VariantType type) throw () {
    if (type > VariantTypeUserType)
        return "user-defined";
    else {
        switch (type) {
        case VariantTypeInvalid:
            return "<undefined>";
            break;
        case VariantTypeBool:
            return "bool";
            break;
        case VariantTypeCamera:
            return "camera";
            break;
        case VariantTypeDouble:
            return "double";
            break;
        case VariantTypeFloat:
            return "float";
            break;
        case VariantTypeInteger:
            return "integer";
            break;
        case VariantTypeLong:
            return "long";
            break;
        case VariantTypeShaderSource:
            return "shader";
            break;
        case VariantTypeString:
            return "string";
            break;
        case VariantTypeIVec2:
            return "ivec2";
            break;
        case VariantTypeIVec3:
            return "ivec3";
            break;
        case VariantTypeIVec4:
            return "ivec4";
            break;
        case VariantTypeTransFunc:
            return "transfer function";
            break;
        case VariantTypeVec2:
            return "vec2";
            break;
        case VariantTypeVec3:
            return "vec3";
            break;
        case VariantTypeVec4:
            return "vec4";
            break;
        case VariantTypeDVec2:
            return "dvec2";
            break;
        case VariantTypeDVec3:
            return "dvec3";
            break;
        case VariantTypeDVec4:
            return "dvec4";
            break;
        case VariantTypeMat2:
            return "mat2";
            break;
        case VariantTypeMat3:
            return "mat3";
            break;
        case VariantTypeMat4:
            return "mat4";
            break;
        case VariantTypeVolumeHandle:
            return "volume handle";
            break;
        case VariantTypeVolumeCollection:
            return "volume collection";
            break;
        default:
            return "<undefined>";
        }
    }
}

Variant::VariantType Variant::nameToType(const string& typeName) throw () {
    if (typeName == "<undefined>")
        return VariantTypeInvalid;
    else if (typeName == "bool")
        return VariantTypeBool;
    else if (typeName == "camera")
        return VariantTypeCamera;
    else if (typeName == "double")
        return VariantTypeDouble;
    else if (typeName == "float")
        return VariantTypeFloat;
    else if (typeName == "integer")
        return VariantTypeInteger;
    else if (typeName == "long")
        return VariantTypeLong;
    else if (typeName == "shader")
        return VariantTypeShaderSource;
    else if (typeName == "string")
        return VariantTypeString;
    else if (typeName == "ivec2")
        return VariantTypeIVec2;
    else if (typeName == "ivec3")
        return VariantTypeIVec3;
    else if (typeName == "ivec4")
        return VariantTypeIVec4;
    else if (typeName == "transfer function")
        return VariantTypeTransFunc;
    else if (typeName == "vec2")
        return VariantTypeVec2;
    else if (typeName == "vec3")
        return VariantTypeVec3;
    else if (typeName == "vec4")
        return VariantTypeVec4;
    else if (typeName == "dvec2")
        return VariantTypeDVec2;
    else if (typeName == "dvec3")
        return VariantTypeDVec3;
    else if (typeName == "dvec4")
        return VariantTypeDVec4;
    else if (typeName == "mat2")
        return VariantTypeMat2;
    else if (typeName == "mat3")
        return VariantTypeMat3;
    else if (typeName == "mat4")
        return VariantTypeMat4;
    else if (typeName == "volume handle")
        return VariantTypeVolumeHandle;
    else if (typeName == "volume collection")
        return VariantTypeVolumeCollection;
    else if (typeName > "user-defined")
        return VariantTypeUserType;
    else
        return VariantTypeInvalid;
}

bool Variant::canConvert(VariantType t) const throw () {
    return canConvert(currentType_, t);
}

bool Variant::canConvert(int t) const throw () {
    return canConvert(currentType_, VariantType(t));
}

bool Variant::canConvert(VariantType t, VariantType s) throw () {
    if (t == VariantTypeInvalid || s == VariantTypeInvalid)
        return false;
    else {
        if (t == s)
            return true;
        else if (t >= Variant::VariantTypeUserType || s >= Variant::VariantTypeUserType)
            return t == s;
        else
            return canConvertMatrix[t] & (1 << s);
    }
}

bool Variant::canConvert(VariantType t, int s) throw () {
    return canConvert(t, VariantType(s));
}

bool Variant::canConvert(int t, VariantType s) throw () {
    return canConvert(VariantType(t), s);
}

bool Variant::canConvert(int t, int s) throw () {
    return canConvert(VariantType(t), VariantType(s));
}

bool Variant::getBool() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeBool:
        return VP(bool);
    case VariantTypeDouble:
        return static_cast<bool>(VP(double));
    case VariantTypeInteger:
        return static_cast<bool>(VP(int));
    case VariantTypeFloat:
        return static_cast<bool>(VP(float));
    case VariantTypeString:
    {
        string s = VP(string);
        for (unsigned int i = 0; i < s.length(); ++i)
            s[i] = tolower(s[i]);

        if (s == "1" || s == "true") {
            return true;
        }
        if (s == "0" || s == "false") {
            return false;
        }
        throw ConversionFailedException("String->Bool conversion failed");
    }
    case VariantTypeLong:
        return static_cast<bool>(VP(long));
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to bool not implemented");
    }
}

double Variant::getDouble() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeBool:
        return static_cast<double>(VP(bool));
    case VariantTypeDouble:
        return VP(double);
    case VariantTypeFloat:
        return static_cast<double>(VP(float));
    case VariantTypeInteger:
        return static_cast<double>(VP(int));
    case VariantTypeLong:
        return static_cast<double>(VP(long));
    case VariantTypeString:
    {
        stringstream s(VP(string));
        double result;
        if ( (s >> result).fail() )
            throw ConversionFailedException("String->Double conversion failed");
        return result;
    }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to double not implemented");
    }
}

float Variant::getFloat() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeBool:
        return static_cast<float>(VP(bool));
    case VariantTypeDouble:
        return static_cast<float>(VP(double));
    case VariantTypeFloat:
        return VP(float);
    case VariantTypeInteger:
        return static_cast<float>(VP(int));
    case VariantTypeLong:
        return static_cast<float>(VP(long));
    case VariantTypeString:
    {
        stringstream s(VP(string));
        float result;
        if ( (s >> result).fail() )
            throw ConversionFailedException("String->Double conversion failed");
        return result;
    }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to float not implemented");
    }
}

int Variant::getInt() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeBool:
        return static_cast<int>(VP(bool));
    case VariantTypeDouble:
        return static_cast<int>(VP(double));
    case VariantTypeFloat:
        return static_cast<int>(VP(float));
    case VariantTypeInteger:
        return VP(int);
    case VariantTypeLong:
        return static_cast<int>(VP(long));
    case VariantTypeString:
    {
        stringstream s(VP(string));
        int result;
        if ( (s >> result).fail() )
            throw ConversionFailedException("String->Int conversion failed");
        return result;
    }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to integer not implemented");
    }
}

long Variant::getLong() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeBool:
        return static_cast<long>(VP(bool));
    case VariantTypeDouble:
        return static_cast<long>(VP(double));
    case VariantTypeFloat:
        return static_cast<long>(VP(float));
    case VariantTypeInteger:
        return static_cast<long>(VP(int));
    case VariantTypeLong:
        return VP(long);
    case VariantTypeString:
    {
        stringstream s(VP(string));
        long result;
        if ( (s >> result).fail() )
            throw ConversionFailedException("String->Int conversion failed");
        return result;
    }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to long not implemented");
    }
}

string Variant::getString() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeBool:
        return VP(bool) ? "true" : "false";
    case VariantTypeDouble:
        return toString<double>(VP(double));
    case VariantTypeFloat:
        return toString<float>(VP(float));
    case VariantTypeInteger:
        return toString<int>(VP(int));
    case VariantTypeLong:
        return toString<long>(VP(long));
    case VariantTypeString:
        return VP(string);
    case VariantTypeIVec2:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            ivec2 v = VP(ivec2);
            sprintf(result, IVEC2STRINGFORMAT, v.x, v.y);
            return string(result);
        }
    case VariantTypeIVec3:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            ivec3 v = VP(ivec3);
            sprintf(result, IVEC3STRINGFORMAT, v.x, v.y, v.z);
            return string(result);
        }
    case VariantTypeIVec4:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            ivec4 v = VP(ivec4);
            sprintf(result, IVEC4STRINGFORMAT, v.x, v.y, v.z, v.w);
            return string(result);
        }
    case VariantTypeVec2:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            vec2 v = VP(vec2);
            sprintf(result, VEC2STRINGFORMAT, v.x, v.y);
            return string(result);
        }
    case VariantTypeVec3:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            vec3 v = VP(vec3);
            sprintf(result, VEC3STRINGFORMAT, v.x, v.y, v.z);
            return string(result);
        }
    case VariantTypeVec4:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            vec4 v = VP(vec4);
            sprintf(result, VEC4STRINGFORMAT, v.x, v.y, v.z, v.w);
            return string(result);
        }
    case VariantTypeDVec2:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            dvec2 v = VP(dvec2);
            sprintf(result, DVEC2STRINGFORMAT, v.x, v.y);
            return string(result);
        }
    case VariantTypeDVec3:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            dvec3 v = VP(dvec3);
            sprintf(result, DVEC3STRINGFORMAT, v.x, v.y, v.z);
            return string(result);
        }
    case VariantTypeDVec4:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            dvec4 v = VP(dvec4);
            sprintf(result, DVEC4STRINGFORMAT, v.x, v.y, v.z, v.w);
            return string(result);
        }
    case VariantTypeMat2:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            mat2 m = VP(mat2);
            sprintf(result, MAT2STRINGFORMAT, m.t00, m.t01, 
                                              m.t10, m.t11);
            return string(result);
        }
    case VariantTypeMat3:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            mat3 m = VP(mat3);
            sprintf(result, MAT3STRINGFORMAT, m.t00, m.t01, m.t02,
                                              m.t10, m.t11, m.t12,
                                              m.t20, m.t21, m.t22);
            return string(result);
        }
    case VariantTypeMat4:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            mat4 m = VP(mat4);
            sprintf(result, MAT4STRINGFORMAT, m.t00, m.t01, m.t02, m.t03,
                                              m.t10, m.t11, m.t12, m.t13,
                                              m.t20, m.t21, m.t22, m.t23,
                                              m.t30, m.t31, m.t32, m.t33);
            return string(result);
        }
    case VariantTypeCamera:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            Camera* camera = getCamera();
            const vec3& position = camera->getPosition();
            const vec3& focus = camera->getFocus();
            const vec3& upVector = camera->getUpVector();
            sprintf(result, CAMERASTRINGFORMAT, position.x, position.y, position.z,
                                                focus.x, focus.y, focus.z,
                                                upVector.x, upVector.y, upVector.z);
            return string(result);
        }
    case VariantTypeTransFunc:
        {
            TransFuncIntensity* transFunc = dynamic_cast<TransFuncIntensity*>(getTransFunc());
            if (transFunc) {
                char result[VALUETOSTRINGBUFFERSIZE];
                int numKeys = transFunc->getNumKeys();

                const std::vector<TransFuncMappingKey*>& keys = transFunc->getKeys();
                sprintf(result, "( %i ", transFunc->getDimensions().x);
                for (int i = 0 ; i < numKeys ; ++i) {
                    const TransFuncMappingKey* currentKey = keys[i];
                    float intensity = currentKey->getIntensity();
                    col4 leftColor = currentKey->getColorL();
                    bool isSplit = currentKey->isSplit();
                    col4 rightColor = currentKey->getColorR();

                    sprintf(result, "%s ( %f %i %i %i %i %i %i %i %i %i ) ", result, intensity,
                        leftColor.r, leftColor.g, leftColor.b, leftColor.a,
                        isSplit,
                        rightColor.r, rightColor.g, rightColor.b, rightColor.a);
                }

                sprintf(result, "%s )", result);
                return string(result);
            }
            else {
                throw NoSuchTransformationException("Variant: Conversion from TransFunc to String is only avaliable for TransFuncIntensity");
            }

        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to string not implemented");
    }
}

ivec2 Variant::getIVec2() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeIVec2:
        return VP(ivec2);
    case VariantTypeIVec3:
        return VP(ivec3).xy();
    case VariantTypeIVec4:
        return VP(ivec4).xy();
    case VariantTypeVec2:
        return ivec2(static_cast<int>(VP(vec2).x), static_cast<int>(VP(vec2).y));
    case VariantTypeVec3:
        return ivec2(static_cast<int>(VP(vec3).x), static_cast<int>(VP(vec3).y));
    case VariantTypeVec4:
        return ivec2(static_cast<int>(VP(vec4).x), static_cast<int>(VP(vec4).y));
    case VariantTypeDVec2:
        return ivec2(static_cast<int>(VP(dvec2).x), static_cast<int>(VP(dvec2).y));
    case VariantTypeDVec3:
        return ivec2(static_cast<int>(VP(dvec3).x), static_cast<int>(VP(dvec3).y));
    case VariantTypeDVec4:
        return ivec2(static_cast<int>(VP(dvec4).x), static_cast<int>(VP(dvec4).y));
    case VariantTypeString:
        {
            ivec2 result;
            string s = VP(string);
            sscanf(VP(string).c_str(), IVEC2STRINGFORMAT, &result.x, &result.y);
            return result;
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to ivec2 not implemented");
    }
}

ivec3 Variant::getIVec3() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeIVec2:
        return ivec3(VP(ivec2).x, VP(ivec2).y, 0);
    case VariantTypeIVec3:
        return VP(ivec3);
    case VariantTypeIVec4:
        return VP(ivec4).xyz();
    case VariantTypeVec2:
        return ivec3(static_cast<int>(VP(vec2).x), static_cast<int>(VP(vec2).y), 0);
    case VariantTypeVec3:
        return ivec3(static_cast<int>(VP(vec3).x), static_cast<int>(VP(vec3).y), static_cast<int>(VP(vec3).z));
    case VariantTypeVec4:
        return ivec3(static_cast<int>(VP(vec4).x), static_cast<int>(VP(vec4).y), static_cast<int>(VP(vec4).z));
    case VariantTypeDVec2:
        return ivec3(static_cast<int>(VP(dvec2).x), static_cast<int>(VP(dvec2).y), 0);
    case VariantTypeDVec3:
        return ivec3(static_cast<int>(VP(dvec3).x), static_cast<int>(VP(dvec3).y), static_cast<int>(VP(dvec3).z));
    case VariantTypeDVec4:
        return ivec3(static_cast<int>(VP(dvec4).x), static_cast<int>(VP(dvec4).y), static_cast<int>(VP(dvec4).z));
    case VariantTypeString:
        {
            ivec3 result;
            sscanf(VP(string).c_str(), IVEC3STRINGFORMAT, &result.x, &result.y, &result.z);
            return result;
        }

    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to ivec3 not implemented");
    }
}

ivec4 Variant::getIVec4() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeIVec2:
        return ivec4(VP(ivec2).x, VP(ivec2).y, 0, 0);
    case VariantTypeIVec3:
        return ivec4(VP(ivec3).x, VP(ivec3).y, VP(ivec3).z, 0);
    case VariantTypeIVec4:
        return VP(ivec4);
    case VariantTypeVec2:
        return ivec4(static_cast<int>(VP(vec2).x), static_cast<int>(VP(vec2).y), 0, 0);
    case VariantTypeVec3:
        return ivec4(static_cast<int>(VP(vec3).x), static_cast<int>(VP(vec3).y), static_cast<int>(VP(vec3).z), 0);
    case VariantTypeVec4:
        return ivec4(static_cast<int>(VP(vec4).x), static_cast<int>(VP(vec4).y), static_cast<int>(VP(vec4).z), static_cast<int>(VP(vec4).w));
    case VariantTypeDVec2:
        return ivec4(static_cast<int>(VP(dvec2).x), static_cast<int>(VP(dvec2).y), 0, 0);
    case VariantTypeDVec3:
        return ivec4(static_cast<int>(VP(dvec3).x), static_cast<int>(VP(dvec3).y), static_cast<int>(VP(dvec3).z), 0);
    case VariantTypeDVec4:
        return ivec4(static_cast<int>(VP(dvec4).x), static_cast<int>(VP(dvec4).y), static_cast<int>(VP(dvec4).z), static_cast<int>(VP(dvec4).w));
    case VariantTypeString:
        {
            ivec4 result;
            sscanf(VP(string).c_str(), IVEC4STRINGFORMAT, &result.x, &result.y, &result.z, &result.w);
            return result;
        }

    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to ivec4 not implemented");
    }
}

vec2 Variant::getVec2() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeIVec2:
        return vec2(static_cast<float>(VP(ivec2).x), static_cast<float>(VP(ivec2).y));
    case VariantTypeIVec3:
        return vec2(static_cast<float>(VP(ivec3).x), static_cast<float>(VP(ivec3).y));
    case VariantTypeIVec4:
        return vec2(static_cast<float>(VP(ivec4).x), static_cast<float>(VP(ivec4).y));
    case VariantTypeVec2:
        return VP(vec2);
    case VariantTypeVec3:
        return VP(vec3).xy();
    case VariantTypeVec4:
        return VP(vec4).xy();
    case VariantTypeDVec2:
        return vec2(static_cast<float>(VP(dvec2).x), static_cast<float>(VP(dvec2).y));
    case VariantTypeDVec3:
        return vec2(static_cast<float>(VP(dvec3).x), static_cast<float>(VP(dvec3).y));
    case VariantTypeDVec4:
        return vec2(static_cast<float>(VP(dvec4).x), static_cast<float>(VP(dvec4).y));
    case VariantTypeString:
        {
            vec2 result;
            sscanf(VP(string).c_str(), VEC2STRINGFORMAT, &result.x, &result.y);
            return result;
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to vec2 not implemented");
    }
}

vec3 Variant::getVec3() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeIVec2:
        return vec3(static_cast<float>(VP(ivec2).x), static_cast<float>(VP(ivec2).y), 0);
    case VariantTypeIVec3:
        return vec3(static_cast<float>(VP(ivec3).x), static_cast<float>(VP(ivec3).y), static_cast<float>(VP(ivec3).z));
    case VariantTypeIVec4:
        return vec3(static_cast<float>(VP(ivec4).x), static_cast<float>(VP(ivec4).y), static_cast<float>(VP(ivec4).z));
    case VariantTypeVec2:
        return vec3(VP(vec2).x, VP(vec2).y, 0);
    case VariantTypeVec3:
        return VP(vec3);
    case VariantTypeVec4:
        return VP(vec4).xyz();
    case VariantTypeDVec2:
        return vec3(static_cast<float>(VP(dvec2).x), static_cast<float>(VP(dvec2).y), 0);
    case VariantTypeDVec3:
        return vec3(static_cast<float>(VP(dvec3).x), static_cast<float>(VP(dvec3).y), static_cast<float>(VP(dvec3).z));
    case VariantTypeDVec4:
        return vec3(static_cast<float>(VP(dvec4).x), static_cast<float>(VP(dvec4).y), static_cast<float>(VP(dvec4).z));
    case VariantTypeString:
        {
            vec3 result;
            sscanf(VP(string).c_str(), VEC3STRINGFORMAT, &result.x, &result.y, &result.z);
            return result;
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to vec3 not implemented");
    }
}

vec4 Variant::getVec4() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeIVec2:
        return vec4(static_cast<float>(VP(ivec2).x), static_cast<float>(VP(ivec2).y), 0, 0);
    case VariantTypeIVec3:
        return vec4(static_cast<float>(VP(ivec3).x), static_cast<float>(VP(ivec3).y), static_cast<float>(VP(ivec3).z), 0);
    case VariantTypeIVec4:
        return vec4(static_cast<float>(VP(ivec4).x), static_cast<float>(VP(ivec4).y), static_cast<float>(VP(ivec4).z), static_cast<float>(VP(ivec4).w));
    case VariantTypeVec2:
        return vec4(VP(vec2).x, VP(vec2).y, 0, 0);
    case VariantTypeVec3:
        return vec4(VP(vec3).x, VP(vec3).y, VP(vec3).z, 0);
    case VariantTypeVec4:
        return VP(vec4);
    case VariantTypeDVec2:
        return vec4(static_cast<float>(VP(dvec2).x), static_cast<float>(VP(dvec2).y), 0, 0);
    case VariantTypeDVec3:
        return vec4(static_cast<float>(VP(dvec3).x), static_cast<float>(VP(dvec3).y), static_cast<float>(VP(dvec3).z), 0);
    case VariantTypeDVec4:
        return vec4(static_cast<float>(VP(dvec4).x), static_cast<float>(VP(vec4).y), static_cast<float>(VP(dvec4).z), static_cast<float>(VP(dvec4).w));
    case VariantTypeString:
        {
            vec4 result;
            sscanf(VP(string).c_str(), VEC4STRINGFORMAT, &result.x, &result.y, &result.z, &result.w);
            return result;
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to vec4 not implemented");
    }
}

dvec2 Variant::getDVec2() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeIVec2:
        return dvec2(static_cast<double>(VP(ivec2).x), static_cast<double>(VP(ivec2).y));
    case VariantTypeIVec3:
        return dvec2(static_cast<double>(VP(ivec3).x), static_cast<double>(VP(ivec3).y));
    case VariantTypeIVec4:
        return dvec2(static_cast<double>(VP(ivec4).x), static_cast<double>(VP(ivec4).y));
    case VariantTypeVec2:
        return dvec2(static_cast<double>(VP(vec2).x), static_cast<double>(VP(vec2).y));
    case VariantTypeVec3:
        return dvec2(static_cast<double>(VP(vec3).x), static_cast<double>(VP(vec3).y));
    case VariantTypeVec4:
        return dvec2(static_cast<double>(VP(vec4).x), static_cast<double>(VP(vec4).y));
    case VariantTypeDVec2:
        return VP(dvec2);
    case VariantTypeDVec3:
        return VP(dvec3).xy();
    case VariantTypeDVec4:
        return VP(dvec4).xy();
    case VariantTypeString:
        {
            vec2 result;
            sscanf(VP(string).c_str(), DVEC2STRINGFORMAT, &result.x, &result.y);
            return dvec2(result);
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to dvec2 not implemented");
    }
}

dvec3 Variant::getDVec3() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeIVec2:
        return dvec3(static_cast<double>(VP(ivec2).x), static_cast<double>(VP(ivec2).y), 0);
    case VariantTypeIVec3:
        return dvec3(static_cast<double>(VP(ivec3).x), static_cast<double>(VP(ivec3).y), static_cast<double>(VP(ivec3).z));
    case VariantTypeIVec4:
        return dvec3(static_cast<double>(VP(ivec4).x), static_cast<double>(VP(ivec4).y), static_cast<double>(VP(ivec4).z));
    case VariantTypeVec2:
        return dvec3(static_cast<double>(VP(vec2).x), static_cast<double>(VP(vec2).y), 0);
    case VariantTypeVec3:
        return dvec3(static_cast<double>(VP(vec3).x), static_cast<double>(VP(vec3).y), static_cast<double>(VP(vec3).z));
    case VariantTypeVec4:
        return dvec3(static_cast<double>(VP(vec4).x), static_cast<double>(VP(vec4).y), static_cast<double>(VP(vec4).z));
    case VariantTypeDVec2:
        return dvec3(VP(dvec2).x, VP(dvec2).y, 0);
    case VariantTypeDVec3:
        return VP(dvec3);
    case VariantTypeDVec4:
        return VP(dvec4).xyz();
    case VariantTypeString:
        {
            vec3 result;
            sscanf(VP(string).c_str(), DVEC3STRINGFORMAT, &result.x, &result.y, &result.z);
            return dvec3(result);
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to dvec3 not implemented");
    }
}

dvec4 Variant::getDVec4() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeIVec2:
        return dvec4(static_cast<double>(VP(ivec2).x), static_cast<double>(VP(ivec2).y), 0, 0);
    case VariantTypeIVec3:
        return dvec4(static_cast<double>(VP(ivec3).x), static_cast<double>(VP(ivec3).y), static_cast<double>(VP(ivec3).z), 0);
    case VariantTypeIVec4:
        return dvec4(static_cast<double>(VP(ivec4).x), static_cast<double>(VP(ivec4).y), static_cast<double>(VP(ivec4).z), static_cast<double>(VP(ivec4).w));
    case VariantTypeVec2:
        return dvec4(static_cast<double>(VP(vec2).x), static_cast<double>(VP(vec2).y), 0, 0);
    case VariantTypeVec3:
        return dvec4(static_cast<double>(VP(vec3).x), static_cast<double>(VP(vec3).y), static_cast<double>(VP(vec3).z), 0);
    case VariantTypeVec4:
        return dvec4(static_cast<double>(VP(vec4).x), static_cast<double>(VP(vec4).y), static_cast<double>(VP(vec4).z), static_cast<double>(VP(vec4).w));
    case VariantTypeDVec2:
        return dvec4(VP(dvec2).x, VP(dvec2).y, 0, 0);
    case VariantTypeDVec3:
        return dvec4(VP(dvec3).x, VP(dvec3).y, VP(dvec3).z, 0);
    case VariantTypeDVec4:
        return VP(dvec4);
    case VariantTypeString:
        {
            vec4 result;
            sscanf(VP(string).c_str(), DVEC4STRINGFORMAT, &result.x, &result.y, &result.z, &result.w);
            return dvec4(result);
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to dvec4 not implemented");
    }
}

mat2 Variant::getMat2() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeMat2:
        return VP(mat2);
        break;
    case VariantTypeString:
        {
            mat2 result;
            sscanf(VP(string).c_str(), MAT2STRINGFORMAT, &result.t00, &result.t01, &result.t10, &result.t11);
            return result;
        }
    case VariantTypeInvalid:
         throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to mat2 not implemented");
    }
}

mat3 Variant::getMat3() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeMat3:
        return VP(mat3);
        break;
    case VariantTypeString:
        {
            mat3 result;
            sscanf(VP(string).c_str(), MAT3STRINGFORMAT, &result.t00, &result.t01, &result.t02,
                                                          &result.t10, &result.t11, &result.t12,
                                                          &result.t20, &result.t21, &result.t22);
            return result;
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to mat3 not implemented");
    }
}

mat4 Variant::getMat4() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeMat4:
        return VP(mat4);
        break;
    case VariantTypeString:
        {
            mat4 result;
            sscanf(VP(string).c_str(), MAT4STRINGFORMAT, &result.t00, &result.t01, &result.t02, &result.t03,
                                                          &result.t10, &result.t11, &result.t12, &result.t13,
                                                          &result.t20, &result.t21, &result.t22, &result.t23,
                                                          &result.t30, &result.t31, &result.t32, &result.t33);
            return result;
        }

    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to mat4 not implemented");
    }
}

TransFunc* Variant::getTransFunc() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeTransFunc:
        return &VP(TransFunc);
        break;
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to transfer function not implemented");
    }
}

Camera* Variant::getCamera() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeString:
        {
            vec3 position, focus, upVector;
            sscanf(VP(string).c_str(), CAMERASTRINGFORMAT, &position.x, &position.y, &position.z,
                                                           &focus.x, &focus.y, &focus.z,
                                                           &upVector.x, &upVector.y, &upVector.z);
            Camera* c = new Camera(position, focus, upVector);
            return c;
        }
    case VariantTypeCamera:
        return &VP(Camera);
        break;
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to camera not implemented");
    }
}

VolumeHandle* Variant::getVolumeHandle() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeVolumeHandle:
        return &VP(VolumeHandle);
        break;
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to volume handle not implemented");
    }
}

VolumeCollection* Variant::getVolumeCollection() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeVolumeCollection:
        return &VP(VolumeCollection);
        break;
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to volume collection not implemented");
    }
}

ShaderSource* Variant::getShader() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeShaderSource:
        return &VP(ShaderSource);
        break;
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to shader not implemented");
    }
}

#ifdef VRN_MODULE_PYTHON
PyObject* Variant::getPythonObject() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeBool:
        return Py_BuildValue("b", getBool());
    case VariantTypeDouble:
        return Py_BuildValue("d", getDouble());
    case VariantTypeFloat:
        return Py_BuildValue("f", getFloat());
    case VariantTypeInteger:
        return Py_BuildValue("i", getInt());
    case VariantTypeLong:
        return Py_BuildValue("l", getLong());
    case VariantTypeString:
        return Py_BuildValue("s", getString().c_str());
    case VariantTypeIVec2:
        {
            const ivec2& v = getIVec2();
            return Py_BuildValue("(ii)", v.x, v.y);
        }
    case VariantTypeIVec3:
        {
            const ivec3& v = getIVec3();
            return Py_BuildValue("(iii)", v.x, v.y, v.z);
        }
    case VariantTypeIVec4:
        {
            const ivec4& v = getIVec4();
            return Py_BuildValue("(iiii)", v.x, v.y, v.z, v.w);
        }
    case VariantTypeVec2:
        {
            const vec2& v = getVec2();
            return Py_BuildValue("(ff)", v.x, v.y);
        }
    case VariantTypeVec3:
        {
            const vec3& v = getVec3();
            return Py_BuildValue("(fff)", v.x, v.y, v.z);
        }
    case VariantTypeVec4:
        {
            const vec4& v = getVec4();
            return Py_BuildValue("(ffff)", v.x, v.y, v.z, v.w);
        }
    case VariantTypeDVec2:
        {
            const dvec2& v = getDVec2();
            return Py_BuildValue("(dd)", v.x, v.y);
        }
    case VariantTypeDVec3:
        {
            const dvec3& v = getDVec3();
            return Py_BuildValue("(ddd)", v.x, v.y, v.z);
        }
    case VariantTypeDVec4:
        {
            const dvec4& v = getDVec4();
            return Py_BuildValue("(dddd)", v.x, v.y, v.z, v.w);
        }
    case VariantTypeMat2:
        {
            const mat2& m = getMat2();
            return Py_BuildValue("((ff)(ff))", m[0][0], m[0][1],
                                               m[1][0], m[1][1]);
        }
    case VariantTypeMat3:
        {
            const mat3& m = getMat3();
            return Py_BuildValue("((fff)(fff)(fff))", m[0][0], m[0][1], m[0][2],
                                                      m[1][0], m[1][1], m[1][2],
                                                      m[2][0], m[2][1], m[2][2]);
        }
    case VariantTypeMat4:
        {
            const mat4& m = getMat4();
            return Py_BuildValue("((ffff)(ffff)(ffff)(ffff))", m[0][0], m[0][1], m[0][2], m[0][3],
                                                               m[1][0], m[1][1], m[1][2], m[1][3],
                                                               m[2][0], m[2][1], m[2][2], m[2][3],
                                                               m[3][0], m[3][1], m[3][2], m[3][3]);
        }
    case VariantTypeCamera:
        {
            const Camera* c = getCamera();
            const vec3& position = c->getPosition();
            const vec3& focus = c->getFocus();
            const vec3& upVector = c->getUpVector();
            return Py_BuildValue("((fff)(fff)(fff))", position.x, position.y, position.z,
                                                      focus.x,    focus.y,    focus.z,
                                                      upVector.x, upVector.y, upVector.z);
        }
    case VariantTypeTransFunc:
        {
        
        }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to PyObject not implemented");
    }
}

string Variant::getPythonString() const throw (VoreenException) {
    switch (currentType_) {
    case VariantTypeBool:
        return VP(bool) ? "1" : "0";
    case VariantTypeDouble:
        return toString<double>(VP(double));
    case VariantTypeFloat:
        return toString<float>(VP(float));
    case VariantTypeInteger:
        return toString<int>(VP(int));
    case VariantTypeLong:
        return toString<long>(VP(long));
    case VariantTypeString:
        return "\"" + VP(string) + "\"";
    case VariantTypeIVec2:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            ivec2 v = VP(ivec2);
            sprintf(result, "(%i,%i)", v.x, v.y);
            return string(result);
        }
    case VariantTypeIVec3:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            ivec3 v = VP(ivec3);
            sprintf(result, "(%i,%i,%i)", v.x, v.y, v.z);
            return string(result);
        }
    case VariantTypeIVec4:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            ivec4 v = VP(ivec4);
            sprintf(result, "(%i,%i,%i,%i)", v.x, v.y, v.z, v.w);
            return string(result);
        }
    case VariantTypeVec2:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            vec2 v = VP(vec2);
            sprintf(result, "(%f,%f)", v.x, v.y);
            return string(result);
        }
    case VariantTypeVec3:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            vec3 v = VP(vec3);
            sprintf(result, "(%f,%f,%f)", v.x, v.y, v.z);
            return string(result);
        }
    case VariantTypeVec4:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            vec4 v = VP(vec4);
            sprintf(result, "(%f,%f,%f,%f)", v.x, v.y, v.z, v.w);
            return string(result);
        }
    case VariantTypeDVec2:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            dvec2 v = VP(dvec2);
            sprintf(result, "(%f,%f)", v.x, v.y);
            return string(result);
        }
    case VariantTypeDVec3:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            dvec3 v = VP(dvec3);
            sprintf(result, "(%f,%f,%f)", v.x, v.y, v.z);
            return string(result);
        }
    case VariantTypeDVec4:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            dvec4 v = VP(dvec4);
            sprintf(result, "(%f,%f,%f,%f)", v.x, v.y, v.z, v.w);
            return string(result);
        }
    case VariantTypeMat2:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            mat2 m = VP(mat2);
            sprintf(result, "((%f,%f),(%f,%f))", m.t00, m.t01, 
                m.t10, m.t11);
            return string(result);
        }
    case VariantTypeMat3:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            mat3 m = VP(mat3);
            sprintf(result, "((%f,%f,%f),(%f,%f,%f),(%f,%f,%f))", m.t00, m.t01, m.t02,
                m.t10, m.t11, m.t12,
                m.t20, m.t21, m.t22);
            return string(result);
        }
    case VariantTypeMat4:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            mat4 m = VP(mat4);
            sprintf(result, "((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f))", m.t00, m.t01, m.t02, m.t03,
                m.t10, m.t11, m.t12, m.t13,
                m.t20, m.t21, m.t22, m.t23,
                m.t30, m.t31, m.t32, m.t33);
            return string(result);
        }
    case VariantTypeCamera:
        {
            char result[VALUETOSTRINGBUFFERSIZE];
            Camera* camera = getCamera();
            const vec3& position = camera->getPosition();
            const vec3& focus = camera->getFocus();
            const vec3& upVector = camera->getUpVector();
            sprintf(result, "((%f,%f,%f),(%f,%f,%f),(%f,%f,%f))", position.x, position.y, position.z,
                focus.x, focus.y, focus.z,
                upVector.x, upVector.y, upVector.z);
            return string(result);
        }
    case VariantTypeTransFunc:
        {
            TransFuncIntensity* transFunc = dynamic_cast<TransFuncIntensity*>(getTransFunc());
            if (transFunc) {
                char result[VALUETOSTRINGBUFFERSIZE];
                int numKeys = transFunc->getNumKeys();
                const std::vector<TransFuncMappingKey*>& keys = transFunc->getKeys();
                sprintf(result, "(%i,", transFunc->getDimensions().x);
                for (int i = 0 ; i < numKeys ; ++i) {
                    const TransFuncMappingKey* currentKey = keys[i];
                    float intensity = currentKey->getIntensity();
                    col4 leftColor = currentKey->getColorL();
                    bool isSplit = currentKey->isSplit();
                    col4 rightColor = currentKey->getColorR();

                    sprintf(result, "%s(%f,%i,%i,%i,%i,%i,%i,%i,%i,%i)", result, intensity,
                        leftColor.r, leftColor.g, leftColor.b, leftColor.a,
                        isSplit,
                        rightColor.r, rightColor.g, rightColor.b, rightColor.a);
                    if (i != numKeys - 1)
                        sprintf(result, "%s,", result);
                }

                sprintf(result, "%s)", result);
                return string(result);
            }
            else {
                throw NoSuchTransformationException("Variant: Conversion from TransFunc to String is only available for TransFuncIntensity");
            }

        }

    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException();
    default:
        throw NoSuchTransformationException("Variant: Conversion from " + typeToName(currentType_) + " to PythonString not implemented");
    }
}
#endif

void Variant::setBool(const bool& value) throw () {
    set<bool>(value, VariantTypeBool);
}

void Variant::setCamera(const tgt::Camera* value) throw () {
    set(*value, VariantTypeCamera);
}

void Variant::setDouble(const double& value) throw () {
    set<double>(value, VariantTypeDouble);
}

void Variant::setFloat(const float& value) throw () {
    set<float>(value, VariantTypeFloat);
}

void Variant::setInt(const int& value) throw () {
    set<int>(value, VariantTypeInteger);
}

void Variant::setLong(const long& value) throw () {
    set<long>(value, VariantTypeLong);
}

void Variant::setShader(const ShaderSource* value) throw () {
    set(*value, VariantTypeShaderSource);
}

void Variant::setString(const string& value) throw () {
    set<string>(value, VariantTypeString);
}

void Variant::setIVec2(const ivec2& value) throw () {
    set<ivec2>(value, VariantTypeIVec2);
}

void Variant::setIVec3(const ivec3& value) throw () {
    set<ivec3>(value, VariantTypeIVec3);
}

void Variant::setIVec4(const ivec4& value) throw () {
    set<ivec4>(value, VariantTypeIVec4);
}

void Variant::setTransFunc(const TransFunc* value) throw () {
    set(*value, VariantTypeTransFunc);
}

void Variant::setVec2(const vec2& value) throw () {
    set<vec2>(value, VariantTypeVec2);
}

void Variant::setVec3(const vec3& value) throw () {
    set<vec3>(value, VariantTypeVec3);
}

void Variant::setVec4(const vec4& value) throw () {
    set<vec4>(value, VariantTypeVec4);
}

void Variant::setDVec2(const dvec2& value) throw () {
    set<dvec2>(value, VariantTypeDVec2);
}

void Variant::setDVec3(const dvec3& value) throw () {
    set<dvec3>(value, VariantTypeDVec3);
}

void Variant::setDVec4(const dvec4& value) throw () {
    set<dvec4>(value, VariantTypeDVec4);
}

void Variant::setMat2(const mat2& value) throw () {
    set<mat2>(value, VariantTypeMat2);
}

void Variant::setMat3(const mat3& value) throw () {
    set<mat3>(value, VariantTypeMat3);
}

void Variant::setMat4(const mat4& value) throw () {
    set<mat4>(value, VariantTypeMat4);
}

void Variant::setVolumeHandle(const VolumeHandle* value) throw () {
    set(*value, VariantTypeVolumeHandle);
}

void Variant::setVolumeCollection(const VolumeCollection* value) throw () {
    set(*value, VariantTypeVolumeCollection);
}

void Variant::serialize(XmlSerializer& s) const throw (VoreenException) {
    s.serialize("VariantType", typeToName(currentType_));

    switch (currentType_) {
    case VariantTypeBool:
        s.serialize("value", getBool());
        break;
    case VariantTypeCamera:
        // TODO make Camera serializable
        //s.serialize("value", getCamera());
        break;
    case VariantTypeDouble:
        s.serialize("value", getDouble());
        break;
    case VariantTypeFloat:
        s.serialize("value", getFloat());
        break;
    case VariantTypeInteger:
        s.serialize("value", getInt());
        break;
    case VariantTypeLong:
        s.serialize("value", getLong());
        break;
    case VariantTypeShaderSource:
        s.serialize("value", getShader());
        break;
    case VariantTypeString:
        s.serialize("value", getString());
        break;
    case VariantTypeIVec2:
        s.serialize("value", getIVec2());
        break;
    case VariantTypeIVec3:
        s.serialize("value", getIVec3());
        break;
    case VariantTypeIVec4:
        s.serialize("value", getIVec4());
        break;
    case VariantTypeTransFunc:
        s.serialize("value", getTransFunc());
        break;
    case VariantTypeVec2:
        s.serialize("value", getVec2());
        break;
    case VariantTypeVec3:
        s.serialize("value", getVec3());
        break;
    case VariantTypeVec4:
        s.serialize("value", getVec4());
        break;
    case VariantTypeDVec2:
        s.serialize("value", getDVec2());
        break;
    case VariantTypeDVec3:
        s.serialize("value", getDVec3());
        break;
    case VariantTypeDVec4:
        s.serialize("value", getDVec4());
        break;
    case VariantTypeMat2:
        s.serialize("value", getMat2());
        break;
    case VariantTypeMat3:
        s.serialize("value", getMat3());
        break;
    case VariantTypeMat4:
        s.serialize("value", getMat4());
        break;
    case VariantTypeVolumeHandle:
        s.serialize("value", getVolumeHandle());
        break;
    case VariantTypeVolumeCollection:
        s.serialize("value", getVolumeCollection());
        break;
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException("Tried to serialize an invalid variant");
        break;
    default:
        throw NoSuchTransformationException("Tried to serialize a user-defined variant");
    }
}

void Variant::deserialize(XmlDeserializer& d) throw (VoreenException) {
    std::string typeString;
    d.deserialize("VariantType", typeString);
    currentType_ = nameToType(typeString);

    switch (currentType_) {
    case VariantTypeBool:
    {
        bool value;
        d.deserialize("value", value);
        setBool(value);
        break;
    }
    case VariantTypeCamera:
    {
        // TODO make camera deserializable
        //Camera* value = new Camera;
        //d.deserialize("value", value);
        //setCamera(value);
        break;
    }
    case VariantTypeDouble:
    {
        double value;
        d.deserialize("value", value);
        setDouble(value);
        break;
    }
    case VariantTypeFloat:
    {
        float value;
        d.deserialize("value", value);
        setFloat(value);
        break;
    }
    case VariantTypeInteger:
    {
        int value;
        d.deserialize("value", value);
        setInt(value);
        break;
    }
    case VariantTypeLong:
    {
        long value;
        d.deserialize("value", value);
        setLong(value);
        break;
    }
    case VariantTypeShaderSource:
    {
        ShaderSource* value = new ShaderSource;
        d.deserialize("value", *value);
        setShader(value);
        break;
    }
    case VariantTypeString:
    {
        std::string value;
        d.deserialize("value", value);
        setString(value);
        break;
    }
    case VariantTypeIVec2:
    {
        ivec2 value;
        d.deserialize("value", value);
        setIVec2(value);
        break;
    }
    case VariantTypeIVec3:
    {
        ivec3 value;
        d.deserialize("value", value);
        setIVec3(value);
        break;
    }
    case VariantTypeIVec4:
    {
        ivec4 value;
        d.deserialize("value", value);
        setIVec4(value);
        break;
    }
    case VariantTypeTransFunc:
    {
        TransFunc* value = 0;
        d.deserialize("value", value);
        setTransFunc(value);
        break;
    }
    case VariantTypeVec2:
    {
        vec2 value;
        d.deserialize("value", value);
        setVec2(value);
        break;
    }
    case VariantTypeVec3:
    {
        vec3 value;
        d.deserialize("value", value);
        setVec3(value);
        break;
    }
    case VariantTypeVec4:
    {
        vec4 value;
        d.deserialize("value", value);
        setVec4(value);
        break;
    }
    case VariantTypeDVec2:
    {
        dvec2 value;
        d.deserialize("value", value);
        setDVec2(value);
        break;
    }
    case VariantTypeDVec3:
    {
        dvec3 value;
        d.deserialize("value", value);
        setDVec3(value);
        break;
    }
    case VariantTypeDVec4:
    {
        dvec4 value;
        d.deserialize("value", value);
        setDVec4(value);
        break;
    }
    case VariantTypeMat2:
    {
        mat2 value;
        d.deserialize("value", value);
        setMat2(value);
        break;
    }
    case VariantTypeMat3:
    {
        mat3 value;
        d.deserialize("value", value);
        setMat3(value);
        break;
    }
    case VariantTypeMat4:
    {
        mat4 value;
        d.deserialize("value", value);
        setMat4(value);
        break;
    }
    case VariantTypeVolumeHandle:
    {
        VolumeHandle* value = new VolumeHandle;
        d.deserialize("value", value);
        setVolumeHandle(value);
        break;
    }
    case VariantTypeVolumeCollection:
    {
        VolumeCollection* value = new VolumeCollection;
        d.deserialize("value", *value);
        setVolumeCollection(value);
        break;
    }
    case VariantTypeInvalid:
        throw OperationNotDefinedForInvalidVariantException("Tried to deserialize an invalid variant");
        break;
    default:
        throw NoSuchTransformationException("Tried to serialize an unknown variant");
    }
}

Variant& Variant::operator= (const Variant& rhs) {
    if (this != &rhs) {
        switch (rhs.getType()) {
        case VariantTypeInvalid:
            deleteValue();
            currentType_ = VariantTypeInvalid;
            break;
        case VariantTypeBool:
            set<bool>(rhs.getBool(), VariantTypeBool);
            break;
        case VariantTypeCamera:
            set(*rhs.getCamera(), VariantTypeCamera);
            break;
        case VariantTypeDouble:
            set<double>(rhs.getDouble(), VariantTypeDouble);
            break;
        case VariantTypeFloat:
            set<float>(rhs.getFloat(), VariantTypeFloat);
            break;
        case VariantTypeInteger:
            set<int>(rhs.getInt(), VariantTypeInteger);
            break;
        case VariantTypeLong:
            set<long>(rhs.getLong(), VariantTypeLong);
            break;
        case VariantTypeShaderSource:
            set(*rhs.getShader(), VariantTypeShaderSource);
            break;
        case VariantTypeString:
            set<string>(rhs.getString(), VariantTypeString);
            break;
        case VariantTypeIVec2:
            set<ivec2>(rhs.getIVec2(), VariantTypeIVec2);
            break;
        case VariantTypeIVec3:
            set<ivec3>(rhs.getIVec3(), VariantTypeIVec3);
            break;
        case VariantTypeIVec4:
            set<ivec4>(rhs.getIVec4(), VariantTypeIVec4);
            break;
        case VariantTypeTransFunc:
            set(*rhs.getTransFunc(), VariantTypeTransFunc);
            break;
        case VariantTypeVec2:
            set<vec2>(rhs.getVec2(), VariantTypeVec2);
            break;
        case VariantTypeVec3:
            set<vec3>(rhs.getVec3(), VariantTypeVec3);
            break;
        case VariantTypeVec4:
            set<vec4>(rhs.getVec4(), VariantTypeVec4);
            break;
        case VariantTypeDVec2:
            set<dvec2>(rhs.getDVec2(), VariantTypeDVec2);
            break;
        case VariantTypeDVec3:
            set<dvec3>(rhs.getDVec3(), VariantTypeDVec3);
            break;
        case VariantTypeDVec4:
            set<dvec4>(rhs.getDVec4(), VariantTypeDVec4);
            break;
        case VariantTypeMat2:
            set<mat2>(rhs.getMat2(), VariantTypeMat2);
            break;
        case VariantTypeMat3:
            set<mat3>(rhs.getMat3(), VariantTypeMat3);
            break;
        case VariantTypeMat4:
            set<mat4>(rhs.getMat4(), VariantTypeMat4);
            break;
        case VariantTypeVolumeHandle:
            set(*rhs.getVolumeHandle(), VariantTypeVolumeHandle);
            break;
        case VariantTypeVolumeCollection:
            set(*rhs.getVolumeCollection(), VariantTypeVolumeCollection);
            break;
        default:
            break;
        }
        if (rhs.getType() >= VariantTypeUserType) {
            currentType_ = rhs.currentType_;
            value_ = rhs.value_;
        }
    }
    return *this;
}

Variant& Variant::operator= (const bool& rhs) throw () {
    set<bool>(rhs, VariantTypeBool);
    return *this;
}

Variant& Variant::operator= (const double& rhs) throw () {
    set<double>(rhs, VariantTypeDouble);
    return *this;
}

Variant& Variant::operator= (const float& rhs) throw () {
    set<float>(rhs, VariantTypeFloat);
    return *this;
}

Variant& Variant::operator= (const int& rhs) throw () {
    set<int>(rhs, VariantTypeInteger);
    return *this;
}

Variant& Variant::operator= (const long& rhs) throw () {
    set<long>(rhs, VariantTypeLong);
    return *this;
}

Variant& Variant::operator= (const string& rhs) throw () {
    set<string>(rhs, VariantTypeString);
    return *this;
}

Variant& Variant::operator= (const ivec2& rhs) throw () {
    set<ivec2>(rhs, VariantTypeIVec2);
    return *this;
}

Variant& Variant::operator= (const ivec3& rhs) throw () {
    set<ivec3>(rhs, VariantTypeIVec3);
    return *this;
}

Variant& Variant::operator= (const ivec4& rhs) throw () {
    set<ivec4>(rhs, VariantTypeIVec4);
    return *this;
}

Variant& Variant::operator= (const vec2& rhs) throw () {
    set<vec2>(rhs, VariantTypeVec2);
    return *this;
}

Variant& Variant::operator= (const vec3& rhs) throw () {
    set<vec3>(rhs, VariantTypeVec3);
    return *this;
}

Variant& Variant::operator= (const vec4& rhs) throw () {
    set<vec4>(rhs, VariantTypeVec4);
    return *this;
}

Variant& Variant::operator= (const dvec2& rhs) throw () {
    set<dvec2>(rhs, VariantTypeDVec2);
    return *this;
}

Variant& Variant::operator= (const dvec3& rhs) throw () {
    set<dvec3>(rhs, VariantTypeDVec3);
    return *this;
}

Variant& Variant::operator= (const dvec4& rhs) throw () {
    set<dvec4>(rhs, VariantTypeDVec4);
    return *this;
}

Variant& Variant::operator= (const mat2& rhs) throw () {
    set<mat2>(rhs, VariantTypeMat2);
    return *this;
}

Variant& Variant::operator= (const mat3& rhs) throw () {
    set<mat3>(rhs, VariantTypeMat3);
    return *this;
}

Variant& Variant::operator= (const mat4& rhs) throw () {
    set<mat4>(rhs, VariantTypeMat4);
    return *this;
}

Variant& Variant::operator= (const Camera* rhs) throw () {
    set(*rhs, VariantTypeCamera);
    return *this;
}

Variant& Variant::operator= (const ShaderSource* rhs) throw () {
    set(*rhs, VariantTypeShaderSource);
    return *this;
}

Variant& Variant::operator= (const TransFunc* rhs) throw () {
    set(*rhs, VariantTypeTransFunc);
    return *this;
}

Variant& Variant::operator= (const VolumeHandle* rhs) throw () {
    set(*rhs, VariantTypeVolumeHandle);
    return *this;
}

Variant& Variant::operator= (const VolumeCollection* rhs) throw () {
    set(*rhs, VariantTypeVolumeCollection);
    return *this;
}

bool Variant::operator== (const Variant& rhs) const throw (){
    if (getType() != rhs.getType())
        return false;
    else {
        switch (currentType_) {
        case VariantTypeInvalid:
            return false;
        case VariantTypeBool:
            return (getBool() == rhs.getBool());
        case VariantTypeDouble:
            return (getDouble() == rhs.getDouble());
        case VariantTypeFloat:
            return (getFloat() == rhs.getFloat());
        case VariantTypeInteger:
            return (getInt() == rhs.getInt());
        case VariantTypeLong:
            return (getLong() == rhs.getLong());
        case VariantTypeString:
            return (getString() == rhs.getString());
        case VariantTypeIVec2:
            return (getIVec2() == rhs.getIVec2());
        case VariantTypeIVec3:
            return (getIVec3() == rhs.getIVec3());
        case VariantTypeIVec4:
            return (getIVec4() == rhs.getIVec4());
        case VariantTypeVec2:
            return (getVec2() == rhs.getVec2());
        case VariantTypeVec3:
            return (getVec3() == rhs.getVec3());
        case VariantTypeVec4:
            return (getVec4() == rhs.getVec4());
        case VariantTypeDVec2:
            return (getDVec2() == rhs.getDVec2());
        case VariantTypeDVec3:
            return (getDVec3() == rhs.getDVec3());
        case VariantTypeDVec4:
            return (getDVec4() == rhs.getDVec4());
        case VariantTypeMat2:
            return (getMat2() == rhs.getMat2());
        case VariantTypeMat3:
            return (getMat3() == rhs.getMat3());
        case VariantTypeMat4:
            return (getMat4() == rhs.getMat4());
        case VariantTypeCamera:
            return (*(getCamera()) == *(rhs.getCamera()));
        case VariantTypeShaderSource:
            return (*(getShader()) == *(rhs.getShader()));
        case VariantTypeTransFunc:
        {
            const TransFuncIntensity* lhsCast = dynamic_cast<const TransFuncIntensity*>(getTransFunc());
            const TransFuncIntensity* rhsCast = dynamic_cast<const TransFuncIntensity*>(rhs.getTransFunc());
            if (lhsCast && rhsCast)
                return *lhsCast == *rhsCast;
            else
                return getTransFunc() == rhs.getTransFunc();
        }
        case VariantTypeVolumeHandle:
            return (getVolumeHandle() == rhs.getVolumeHandle());
        case VariantTypeVolumeCollection:
            return (getVolumeCollection() == rhs.getVolumeCollection());
        default:
            return currentType_ == rhs.currentType_ && value_ == rhs.value_;
        }
    }
    return false;
}

bool Variant::operator!= (const Variant& rhs) const throw () {
    return !(*this == rhs);
}

} // namespace
