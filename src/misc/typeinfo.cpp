/*****************************************************************************************
 *                                                                                       *
 * owl                                                                                   *
 *                                                                                       *
 * Copyright (c) 2014 Jonas Strandstedt                                                  *
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

#include <ghoul/misc/typeinfo.h>

#include <ghoul/filesystem/cachemanager.h>
#include <ghoul/misc/buffer.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/opengl/opengl>

#define TYPEINFO_NAME_DEFINITION(__CLASS__)                                              \
                                                                                         \
template <>                                                                              \
std::string TypeInfo::name<__CLASS__>() {                                                \
    return #__CLASS__;                                                                   \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<__CLASS__*>() {                                               \
    return #__CLASS__"*";                                                                \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<const __CLASS__>() {                                          \
    return "const "#__CLASS__;                                                           \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<const __CLASS__*>() {                                         \
    return "const "#__CLASS__"*";                                                        \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<std::vector<__CLASS__> >() {                                  \
    return "std::vector<"#__CLASS__">";                                                  \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<std::vector<__CLASS__*> >() {                                 \
    return "std::vector<"#__CLASS__"*>";                                                 \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<std::vector<__CLASS__>*>() {                                  \
    return "std::vector<"#__CLASS__">*";                                                 \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<std::vector<__CLASS__*>*>() {                                 \
    return "std::vector<"#__CLASS__"*>*";                                                \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<std::vector<const __CLASS__> >() {                            \
    return "std::vector<const "#__CLASS__">";                                            \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<std::vector<const __CLASS__*> >() {                           \
    return "std::vector<const "#__CLASS__"*>";                                           \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<std::vector<const __CLASS__>*>() {                            \
    return "std::vector<const "#__CLASS__">*";                                           \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<std::vector<const __CLASS__*>*>() {                           \
    return "std::vector<const "#__CLASS__"*>*";                                          \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<const std::vector<__CLASS__> >() {                            \
    return "const std::vector<"#__CLASS__">";                                            \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<const std::vector<__CLASS__*> >() {                           \
    return "const std::vector<"#__CLASS__"*>";                                           \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<const std::vector<__CLASS__>*>() {                            \
    return "const std::vector<"#__CLASS__">*";                                           \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<const std::vector<__CLASS__*>*>() {                           \
    return "const std::vector<"#__CLASS__"*>*";                                          \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<const std::vector<const __CLASS__> >() {                      \
    return "const std::vector<const "#__CLASS__">";                                      \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<const std::vector<const __CLASS__*> >() {                     \
    return "const std::vector<const "#__CLASS__"*>";                                     \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<const std::vector<const __CLASS__>*>() {                      \
    return "const std::vector<const "#__CLASS__">*";                                     \
}                                                                                        \
template <>                                                                              \
std::string TypeInfo::name<const std::vector<const __CLASS__*>*>() {                     \
    return "const std::vector<const "#__CLASS__"*>*";                                    \
}                                                                                        \


namespace ghoul {

// Fundamental types
TYPEINFO_NAME_DEFINITION(std::nullptr_t)
TYPEINFO_NAME_DEFINITION(char)
TYPEINFO_NAME_DEFINITION(bool)
TYPEINFO_NAME_DEFINITION(short)
TYPEINFO_NAME_DEFINITION(int)
TYPEINFO_NAME_DEFINITION(long)
TYPEINFO_NAME_DEFINITION(long long)
TYPEINFO_NAME_DEFINITION(float)
TYPEINFO_NAME_DEFINITION(double)
TYPEINFO_NAME_DEFINITION(long double)
TYPEINFO_NAME_DEFINITION(unsigned char)
TYPEINFO_NAME_DEFINITION(unsigned short)
TYPEINFO_NAME_DEFINITION(unsigned int)
TYPEINFO_NAME_DEFINITION(unsigned long)
TYPEINFO_NAME_DEFINITION(unsigned long long)
TYPEINFO_NAME_DEFINITION(signed char)
TYPEINFO_NAME_DEFINITION(std::string)

// ghoul classes
TYPEINFO_NAME_DEFINITION(ghoul::filesystem::CacheManager)
TYPEINFO_NAME_DEFINITION(ghoul::filesystem::Directory)
TYPEINFO_NAME_DEFINITION(ghoul::filesystem::File)
TYPEINFO_NAME_DEFINITION(ghoul::Buffer)
TYPEINFO_NAME_DEFINITION(ghoul::Dictionary)
TYPEINFO_NAME_DEFINITION(ghoul::opengl::FramebufferObject)
TYPEINFO_NAME_DEFINITION(ghoul::opengl::ProgramObject)
TYPEINFO_NAME_DEFINITION(ghoul::opengl::ShaderObject)
TYPEINFO_NAME_DEFINITION(ghoul::opengl::Texture)

}
