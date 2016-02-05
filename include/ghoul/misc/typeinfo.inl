/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2016                                                               *
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
 *****************************************************************************************
 * The original of this file is found in the owl library maintained by Jonas Strandstedt *
 ****************************************************************************************/

namespace ghoul {

// Forward declare the ghoul classes. Definition not necessary
class Buffer;
class Dictionary;
namespace filesystem {
    class CacheManager;
    class Directory;
    class File;
}
namespace opengl {
    class FramebufferObject;
    class ProgramObject;
    class ShaderObject;
    class Texture;
}

#define TYPEINFO_NAME_DECLARATION(__CLASS__) \
template<> std::string TypeInfo::name<__CLASS__>();                                      \
template<> std::string TypeInfo::name<__CLASS__*>();                                     \
template<> std::string TypeInfo::name<const __CLASS__>();                                \
template<> std::string TypeInfo::name<const __CLASS__*>();                               \
template<> std::string TypeInfo::name<std::vector<__CLASS__>>();                         \
template<> std::string TypeInfo::name<std::vector<__CLASS__*>>();                        \
template<> std::string TypeInfo::name<std::vector<__CLASS__>*>();                        \
template<> std::string TypeInfo::name<std::vector<__CLASS__*>*>();                       \
template<> std::string TypeInfo::name<std::vector<const __CLASS__>>();                   \
template<> std::string TypeInfo::name<std::vector<const __CLASS__*>>();                  \
template<> std::string TypeInfo::name<std::vector<const __CLASS__>*>();                  \
template<> std::string TypeInfo::name<std::vector<const __CLASS__*>*>();                 \
template<> std::string TypeInfo::name<const std::vector<__CLASS__>>();                   \
template<> std::string TypeInfo::name<const std::vector<__CLASS__*>>();                  \
template<> std::string TypeInfo::name<const std::vector<__CLASS__>*>();                  \
template<> std::string TypeInfo::name<const std::vector<__CLASS__*>*>();                 \
template<> std::string TypeInfo::name<const std::vector<const __CLASS__>>();             \
template<> std::string TypeInfo::name<const std::vector<const __CLASS__*>>();            \
template<> std::string TypeInfo::name<const std::vector<const __CLASS__>*>();            \
template<> std::string TypeInfo::name<const std::vector<const __CLASS__*>*>();

// Fundamental types
TYPEINFO_NAME_DECLARATION(std::nullptr_t)
TYPEINFO_NAME_DECLARATION(char)
TYPEINFO_NAME_DECLARATION(bool)
TYPEINFO_NAME_DECLARATION(short)
TYPEINFO_NAME_DECLARATION(int)
TYPEINFO_NAME_DECLARATION(long)
TYPEINFO_NAME_DECLARATION(long long)
TYPEINFO_NAME_DECLARATION(float)
TYPEINFO_NAME_DECLARATION(double)
TYPEINFO_NAME_DECLARATION(long double)
TYPEINFO_NAME_DECLARATION(unsigned char)
TYPEINFO_NAME_DECLARATION(unsigned short)
TYPEINFO_NAME_DECLARATION(unsigned int)
TYPEINFO_NAME_DECLARATION(unsigned long)
TYPEINFO_NAME_DECLARATION(unsigned long long)
TYPEINFO_NAME_DECLARATION(signed char)
TYPEINFO_NAME_DECLARATION(std::string)

// ghoul classes
TYPEINFO_NAME_DECLARATION(Buffer)
TYPEINFO_NAME_DECLARATION(Dictionary)
TYPEINFO_NAME_DECLARATION(filesystem::CacheManager)
TYPEINFO_NAME_DECLARATION(filesystem::Directory)
TYPEINFO_NAME_DECLARATION(filesystem::File)
TYPEINFO_NAME_DECLARATION(opengl::FramebufferObject)
TYPEINFO_NAME_DECLARATION(opengl::ProgramObject)
TYPEINFO_NAME_DECLARATION(opengl::ShaderObject)
TYPEINFO_NAME_DECLARATION(opengl::Texture)

// unknown type
template<class T>
std::string TypeInfo::name() {
    return std::string(typeid(T).name());
}

// unknown type
template<class T>
std::string TypeInfo::name(const T&) {
    return name<T>();
}

}  // ghoul
