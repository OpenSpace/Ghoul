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

#ifndef __GHOUL___DICTIONARY___H__
#define __GHOUL___DICTIONARY___H__

#include <ghoul/glm.h>
#include <ghoul/misc/exception.h>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace ghoul {

/**
 * The Dictionary is a class that represents a mapping from a string to a fixed selection
 * of types. It has the ability to store and retrieve these items by unique string keys.
 * The only automatic conversion that is currently performed is to convert glm vector or
 * matrix types into std::vector types and reconstructing them on access such that:
 * <code>
 * Dictionary d;
 * d.setValue("a", glm::dvec4(1.0, 2.0, 3.0, 4.0);
 * std::vector<double> v = d.value<std::vector<double>>("a");
 * assert(v.size() == 4 && v[0] == 1.0 && v[1] == 2.0 && v[2] == 3.0 && v[3] == 4.0);
 * 
 * Dictionary e;
 * e.setValue("a", std::vector<double>{5.0, 6.0, 7.0, 8.0});
 * glm::dvec4 vv = e.value<glm::dvec4>("a");
 * assert(vv.x == 5.0 && vv.y == 6.0 && vv.z == 7.0 && vv.w == 8.0);
 * </code>
 * are legal
 */
class Dictionary : public std::map<std::string, std::variant<
    bool, int, double, std::string, Dictionary, std::vector<int>, std::vector<double>
>, std::less<>> {
public:
    /// Exception that is thrown if the Dictionary does not contain a provided key
    struct KeyError : public ghoul::RuntimeError {
        explicit KeyError(std::string msg);
    };

    /// Exception thrown if there was an error with a value, either trying to access the
    /// wrong type for a known key or if trying to access a vector/matrix based type and
    /// the underlying std::vector did contain the wrong number of values
    struct ValueError : public ghoul::RuntimeError {
        explicit ValueError(std::string key, std::string msg);
    };

    /**
     * Store the value \c at the specified \c key, overwriting any existing value.
     */    
    void setValue(std::string key, bool value);
    void setValue(std::string key, double value);
    void setValue(std::string key, int value);
    void setValue(std::string key, std::string value);
    void setValue(std::string key, Dictionary value);
    void setValue(std::string key, std::vector<int> value);
    void setValue(std::string key, std::vector<double> value);
    void setValue(std::string key, glm::ivec2 value);
    void setValue(std::string key, glm::ivec3 value);
    void setValue(std::string key, glm::ivec4 value);
    void setValue(std::string key, glm::dvec2 value);
    void setValue(std::string key, glm::dvec3 value);
    void setValue(std::string key, glm::dvec4 value);
    void setValue(std::string key, glm::dmat2x2 value);
    void setValue(std::string key, glm::dmat2x3 value);
    void setValue(std::string key, glm::dmat2x4 value);
    void setValue(std::string key, glm::dmat3x2 value);
    void setValue(std::string key, glm::dmat3x3 value);
    void setValue(std::string key, glm::dmat3x4 value);
    void setValue(std::string key, glm::dmat4x2 value);
    void setValue(std::string key, glm::dmat4x3 value);
    void setValue(std::string key, glm::dmat4x4 value);


    //template<typename U>
    //std::enable_if_t < std::holds_alternative<U>(type{}) >
    //    f() {
    //    // that's ok
    //}
    template <typename T>
    T value(std::string_view key) const;


    //template<typename U>
    //std::enable_if_t < std::holds_alternative<U>(type{}) >
    //    f() {
    //    // that's ok
    //}
    template <typename T>
    bool hasValue(std::string_view key) const;

    // This function should go away
    std::vector<std::string_view> keys() const;

    bool hasKey(std::string_view key) const;
};

} // namespace ghoul

#endif // __GHOUL___DICTIONARY___H__
