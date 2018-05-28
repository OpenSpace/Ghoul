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

#ifndef __GHOUL___DICTIONARY___H__
#define __GHOUL___DICTIONARY___H__

#include <ghoul/glm.h>
#include <ghoul/misc/boolean.h>
#include <ghoul/misc/exception.h>
#include <ghoul/misc/any.h>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace ghoul {

namespace internal {
    // The storage type for integral values (char, short, int, long long, ...)
    using IntegralType = long long;

    // The storage type for unsigned integral values
    using UnsignedIntegralType = unsigned long long;

    // The storage type for floating point values
    using FloatingType = double;

    // Default struct that defines the type to void
    template <typename InputType>
    struct StorageTypeConverter {
        using type = void;
    };

#define DEF_STORAGE_CONVERTER(T, U, S)                                                   \
    template <>                                                                          \
    struct StorageTypeConverter<T> {                                                     \
        using type = U;                                                                  \
        static const size_t size = S;                                                    \
    };

    DEF_STORAGE_CONVERTER(char, IntegralType, 1);
    DEF_STORAGE_CONVERTER(signed char, IntegralType, 1);
    DEF_STORAGE_CONVERTER(unsigned char, UnsignedIntegralType, 1);
    DEF_STORAGE_CONVERTER(wchar_t, IntegralType, 1);
    DEF_STORAGE_CONVERTER(short, IntegralType, 1);
    DEF_STORAGE_CONVERTER(unsigned short, UnsignedIntegralType, 1);
    DEF_STORAGE_CONVERTER(int, IntegralType, 1);
    DEF_STORAGE_CONVERTER(unsigned int, UnsignedIntegralType, 1);
    DEF_STORAGE_CONVERTER(long long, IntegralType, 1);
    DEF_STORAGE_CONVERTER(unsigned long long, UnsignedIntegralType, 1);
    DEF_STORAGE_CONVERTER(float, FloatingType, 1);
    DEF_STORAGE_CONVERTER(double, FloatingType, 1);
    DEF_STORAGE_CONVERTER(glm::vec2, FloatingType, 2);
    DEF_STORAGE_CONVERTER(glm::dvec2, FloatingType, 2);
    DEF_STORAGE_CONVERTER(glm::ivec2, IntegralType, 2);
    DEF_STORAGE_CONVERTER(glm::uvec2, UnsignedIntegralType, 2);
    DEF_STORAGE_CONVERTER(glm::bvec2, IntegralType, 2);
    DEF_STORAGE_CONVERTER(glm::vec3, FloatingType, 3);
    DEF_STORAGE_CONVERTER(glm::dvec3, FloatingType, 3);
    DEF_STORAGE_CONVERTER(glm::ivec3, IntegralType, 3);
    DEF_STORAGE_CONVERTER(glm::uvec3, UnsignedIntegralType, 3);
    DEF_STORAGE_CONVERTER(glm::bvec3, IntegralType, 3);
    DEF_STORAGE_CONVERTER(glm::vec4, FloatingType, 4);
    DEF_STORAGE_CONVERTER(glm::dvec4, FloatingType, 4);
    DEF_STORAGE_CONVERTER(glm::ivec4, IntegralType, 4);
    DEF_STORAGE_CONVERTER(glm::uvec4, UnsignedIntegralType, 4);
    DEF_STORAGE_CONVERTER(glm::bvec4, IntegralType, 4);
    DEF_STORAGE_CONVERTER(glm::mat2x2, FloatingType, 4);
    DEF_STORAGE_CONVERTER(glm::mat2x3, FloatingType, 6);
    DEF_STORAGE_CONVERTER(glm::mat2x4, FloatingType, 8);
    DEF_STORAGE_CONVERTER(glm::mat3x2, FloatingType, 6);
    DEF_STORAGE_CONVERTER(glm::mat3x3, FloatingType, 9);
    DEF_STORAGE_CONVERTER(glm::mat3x4, FloatingType, 12);
    DEF_STORAGE_CONVERTER(glm::mat4x2, FloatingType, 8);
    DEF_STORAGE_CONVERTER(glm::mat4x3, FloatingType, 12);
    DEF_STORAGE_CONVERTER(glm::mat4x4, FloatingType, 16);
    DEF_STORAGE_CONVERTER(glm::dmat2x2, FloatingType, 4);
    DEF_STORAGE_CONVERTER(glm::dmat2x3, FloatingType, 6);
    DEF_STORAGE_CONVERTER(glm::dmat2x4, FloatingType, 8);
    DEF_STORAGE_CONVERTER(glm::dmat3x2, FloatingType, 6);
    DEF_STORAGE_CONVERTER(glm::dmat3x3, FloatingType, 9);
    DEF_STORAGE_CONVERTER(glm::dmat3x4, FloatingType, 12);
    DEF_STORAGE_CONVERTER(glm::dmat4x2, FloatingType, 8);
    DEF_STORAGE_CONVERTER(glm::dmat4x3, FloatingType, 12);
    DEF_STORAGE_CONVERTER(glm::dmat4x4, FloatingType, 16);

#undef DEF_STORAGE_CONVERTER

// Boolean not operator for static values
template <typename T>
using static_not = std::integral_constant<bool, !T::value>;

// Since we define the basic implementation of StorageTypeConverter's type to void, we can
// check whether a type T has an assigned StorageTypeConverter class by checking the type
// against void
template <typename T>
using has_storage_converter = static_not<
    typename std::is_void<typename internal::StorageTypeConverter<T>::type>>;

}

/**
 * The Dictionary is a class to generically store arbitrary items associated with and
 * accessible using <code>std::string</code>%s. It has the abilitiy to store and retrieve
 * these items by unique <code>std::string</code>-typed keys. The items that can be stored
 * have to be compatible with the ghoul::any type. For a select list of types an automatic
 * conversion is performed, so that simple type conversions are possible. The following
 * table is a complete listing of all types that are automatically converted. Each type
 * has a <code>StorageType</code> that is used to store the type internally. Each type
 * with the same number of values and the same <code>StorageType</code> will be converted
 * automatically. For example a value can be stored as an <code>int</code> and can be
 * retrieved as a <code>short</code>. The table of all types is:
 * | <code>Type</code>               | <code>StorageType</code>  | <code>\#Values</code>|
 * |:-------------------------------:|:-------------------------:|:--------------------:|
 * | <code>char</code>               | <code>IntegralType</code>         |  1           |
 * | <code>signed char</code>        | <code>IntegralType</code>         |  1           |
 * | <code>unsigned char</code>      | <code>UnsignedIntegralType</code> |  1           |
 * | <code>wchar_t</code>            | <code>IntegralType</code>         |  1           |
 * | <code>short</code>              | <code>IntegralType</code>         |  1           |
 * | <code>unsigned short</code>     | <code>UnsignedIntegralType</code> |  1           |
 * | <code>int</code>                | <code>IntegralType</code>         |  1           |
 * | <code>unsigned int</code>       | <code>UnsignedIntegralType</code> |  1           |
 * | <code>long long</code>          | <code>IntegralType</code>         |  1           |
 * | <code>unsigned long long</code> | <code>UnsignedIntegralType</code> |  1           |
 * | <code>float</code>              | <code>FloatingType</code>         |  1           |
 * | <code>double</code>             | <code>FloatingType</code>         |  1           |
 * | <code>glm::vec2</code>          | <code>FloatingType</code>         |  2           |
 * | <code>glm::dvec2</code>         | <code>FloatingType</code>         |  2           |
 * | <code>glm::ivec2</code>         | <code>IntegralType</code>         |  2           |
 * | <code>glm::uvec2</code>         | <code>UnsignedIntegralType</code> |  2           |
 * | <code>glm::bvec2</code>         | <code>IntegralType</code>         |  2           |
 * | <code>glm::vec3</code>          | <code>FloatingType</code>         |  3           |
 * | <code>glm::dvec3</code>         | <code>FloatingType</code>         |  3           |
 * | <code>glm::ivec3</code>         | <code>IntegralType</code>         |  3           |
 * | <code>glm::uvec3</code>         | <code>UnsignedIntegralType</code> |  3           |
 * | <code>glm::bvec3</code>         | <code>IntegralType</code>         |  3           |
 * | <code>glm::vec4</code>          | <code>FloatingType</code>         |  4           |
 * | <code>glm::dvec4</code>         | <code>FloatingType</code>         |  4           |
 * | <code>glm::ivec4</code>         | <code>IntegralType</code>         |  4           |
 * | <code>glm::uvec4</code>         | <code>UnsignedIntegralType</code> |  4           |
 * | <code>glm::bvec4</code>         | <code>IntegralType</code>         |  4           |
 * | <code>glm::mat2x2</code>        | <code>FloatingType</code>         |  4           |
 * | <code>glm::mat2x3</code>        | <code>FloatingType</code>         |  6           |
 * | <code>glm::mat2x4</code>        | <code>FloatingType</code>         |  8           |
 * | <code>glm::mat3x2</code>        | <code>FloatingType</code>         |  6           |
 * | <code>glm::mat3x3</code>        | <code>FloatingType</code>         |  9           |
 * | <code>glm::mat3x4</code>        | <code>FloatingType</code>         | 12           |
 * | <code>glm::mat4x2</code>        | <code>FloatingType</code>         |  8           |
 * | <code>glm::mat4x3</code>        | <code>FloatingType</code>         | 12           |
 * | <code>glm::mat4x4</code>        | <code>FloatingType</code>         | 16           |
 * | <code>glm::dmat2x2</code>       | <code>FloatingType</code>         |  4           |
 * | <code>glm::dmat2x3</code>       | <code>FloatingType</code>         |  6           |
 * | <code>glm::dmat2x4</code>       | <code>FloatingType</code>         |  8           |
 * | <code>glm::dmat3x2</code>       | <code>FloatingType</code>         |  6           |
 * | <code>glm::dmat3x3</code>       | <code>FloatingType</code>         |  9           |
 * | <code>glm::dmat3x4</code>       | <code>FloatingType</code>         | 12           |
 * | <code>glm::dmat4x2</code>       | <code>FloatingType</code>         |  8           |
 * | <code>glm::dmat4x3</code>       | <code>FloatingType</code>         | 12           |
 * | <code>glm::dmat4x4</code>       | <code>FloatingType</code>         | 16           |
 *
 * The explicit type of <code>IntegralType</code>, <code>UnsignedIntegralType</code>, and
 * <code>FloatingType</code> is undefined, but all three are using 8 bytes for storage.
 * This means that even storing a single <code>bool</code> will take up 8 bytes in the
 * Dictionary.
 *
 * Values can be added using the <code>std::initializer_list</code> constructor or can be
 * added using the #setValue method. #hasKey will check if the Dictionary has any kind of
 * value for a provided key, regardless of its type. #hasValue will perform the same check
 * but will only return <code>true</code> if the stored type agrees with the template
 * parameter, including possible conversions. All methods accept recursively defined keys
 * which use <code>.</code> as a separator. The key <code>a.b</code> will first search for
 * a Dictionary entry at the key <code>a</code> and then try to find a key <code>b</code>
 * in this second Dictionary and checks, sets, or gets the corresponding value. The single
 * exception to this is the #setValue method, which has an additional parameter that
 * controls if each individual level of the Dictionary is created on-the-fly or not.
 */
class Dictionary : private std::map<std::string, ghoul::any> {
public:
    BooleanType(CreateIntermediate);

    /// Base class for all Dictionary%-based exceptions
    struct DictionaryError : public RuntimeError {
        explicit DictionaryError(std::string msg);
    };

    /// Exception that is thrown if one of the messages is called with invalid arguments
    struct KeyError : public DictionaryError {
        explicit KeyError(std::string msg);
    };

    /// Exception that is thrown if an error converting from one type into another occurs
    struct ConversionError : public DictionaryError {
        explicit ConversionError(std::string msg);
    };

    /**
     * Creates an empty Dictionary
     */
    Dictionary() = default;

    /**
     * Creates a Dictionary out of the provided <code>std::initializer_list</code>. This
     * initializer list can be, for example, of the format
     * <code>{ { "a", 1 }, { "b", 2 } }</code> and it will add all of the
     * <code>std::pair</code>s provided to the Dictionary.
     *
     * \param l The <code>std::initializer_list</code> that contains all of the values
     *        that should be added to the Dictionary
     */
    Dictionary(std::initializer_list<std::pair<std::string, ghoul::any>> l);

    /**
     * Returns all of the keys that are stored in the dictionary at a given \p location.
     * This location specifier can be nested to inspect the keys at deeper levels.
     *
     * \param location The location for which all keys should be returned
     * \return A list of all keys that are stored in the Dictionary for the provided
     *         location
     *
     * \throw KeyError If the provided \p location did not exist in the Dictionary
     * \throw ConversionError if the provided \p location was nested, but one of the
     *        nested levels did exist but was not a Dictionary
     */
    std::vector<std::string> keys(const std::string& location = "") const;

    /**
     * Returns <code>true</code> if there is a specific key in the Dictionary, regardless
     * of its type. \p key can be a nested location to search for keys at deeper levels.
     *
     * \param key The key that should be checked for existence
     * \return <code>true</code> if the provided key exists, <code>false</code> otherwise
     *
     * \pre \p key must not be empty
     */
    bool hasKey(const std::string& key) const;

    /**
     * Adds the \p value for a given location at \p key. If a value already exists at that
     * key, the old value is overwritten, regardless of its previous type and without any
     * feedback. The \p key parameter can be specified as a nested location to add values
     * to deeper levels of the Dictionary. If \p createIntermediate is <code>true</code>
     * all intermediate levels in the Dictionary are created automatically; if it is
     * <code>false</code>, this function will fail if a non-existing level is encountered.
     *
     * \tparam T The type of the \p value to set
     * \param key The key at which the \p value will be entered. This can be a nested
     *        location to insert value are deeper levels. Depending on the value of the
     *        \p createIntermediate parameter, all intermediate levels already have to
     *        exist in the Dictionary or will be created on-the-fly.
     * \param value The value that will be added into the Dictionary. If the type supports
     *        it, the value will only be moved rather than copied.
     * \param createIntermediate If <code>true</code> all intermediate levels in the
     *        Dictionary will be automatically created along the way, if the provided
     *        \p key contains a nested location. If <code>false</code> the method will
     *        fail if a missing intermediate level is encountered.
     *
     * \throw KeyError If no intermediate keys should be created and an intermediate key
     *        is missing
     * \throw ConversionError If an intermediate key does not name a Dictionary, but any
     *        other type. Example: For <code>a.b.c</code>, this exception is thrown if
     *        either <code>a</code> or <code>b</code> exist but are not Dictionary%s
     * \pre \p key must not be empty
     */
    template <typename T>
    void setValue(std::string key, T value,
        CreateIntermediate createIntermediate = CreateIntermediate::No);

    /**
     * Returns the value stored at location with a given \p key. This key can be nested
     * and will automatically be decomposed by the method to traverse to deeper levels of
     * the Dictionary. If the Dictionary has a value at the provided key and the type
     * agrees with the template parameter, the value will be copied into the provided
     * reference \p value and the method returns <code>true</code>. If either the key does
     * not exist or the type does not agree, <code>false</code> is returned and the
     * <code>value</code> is unchanged. If the key does not exist in the dictionary, the
     * error is silently ignored. See the #value method for a variant of this method that
     * does not ignore errors.
     *
     * \tparam T The type of the value that should be retrieved. The <code>typeid</code>
     *         of this type has to be equal to the typeid of the value that is to be
     *        retrieved
     * \param key The, potentially nested, key for which the stored value should be
     *        returned
     * \param value A reference to the value where the value will be copied to, if it
     *        could be found and the types agree
     * \return <code>true</code> if the value was retrieved successfully,
     *         <code>false</code> otherwise
     *
     * \pre \p key must not be empty
     * \pre \p value must not be the Dictionary this method is called on
     * \post If the value could not be retrieved, the \p value is unchanged
     */
    template <typename T>
    bool getValue(const std::string& key, T& value) const;

    /**
     * Returns the value stored at location with a given \p key. This key can be nested
     * and will automatically be decomposed by the method to traverse to deeper levels of
     * the Dictionary. If the Dictionary has a value at the provided key and the type
     * agrees with the template parameter, the value will be returned. If either the key
     * does not exist or the type does not agree, an exception is thrown.
     *
     * \tparam T The type of the value that should be tested. The <code>typeid</code> of
     *         this type has to be equal to the typeid of the value that is to be
     *         retrieved
     * \param key The, potentially nested, key for which the stored value should be
     *         returned
     * \return value The value stored at the <code>key</code>
     *
     * \throw KeyError If the \p key does not exist in the Dictionary
     * \throw ConversionError If the stored value's type for \p does not agree with
     *        <code>T</code>
     * \pre \p key must not be empty
     */
    template <typename T>
    T value(const std::string& key) const;

    /**
     * Returns <code>true</code> if the Dictionary stores a value at the provided
     * \p key and the stored type agrees with the provided template parameter. The key can
     * be nested and refer to deeper levels of the Dictionary. If any of the intermediate
     * levels does not exists, <code>false</code> is returned.
     *
     * \tparam T The type of the value that should be tested. The <code>typeid</code> of
     *         this type has to be equal to the typeid of the value that is to be checked
     * \param key The, potentially nested, key which should be checked for existence
     * \return <code>true</code> if the Dictionary contains a value at the specified
     *         \p key with the correct type <code>T</code>. Will return <code>false</code>
     *         otherwise
     *
     * \pre \p key must not be empty
     */
    template <typename T>
    bool hasValue(const std::string& key) const;

    /**
     * Returns <code>true</code> if the Dictionary contains a value for the specified
     * \p key and the value that is stored is of the type <code>T</code>. This method
     * calls the Dictionary::hasKey and Dictionary::hasValue methods.
     *
     * \tparam T The type of the value that should be tested
     * \param key The key that should be tested
     * \return <code>true</code> if the Dictionary contains a value for the \p key and the
     *         value is of type <code>T</code>
     *
     * \pre \p key must not be empty
     */
    template <typename T>
    bool hasKeyAndValue(const std::string& key) const;

    /**
     * Returns the total number of keys stored in this Dictionary. This method will not
     * recurse into sub-Dictionaries, but will only return the top-level keys for the
     * Dictionary it is called on. This value will always be equal to the size of the
     * <code>std::vector</code> returned by #keys.
     *
     * \return The number of keys stored in this Dictionary
     */
    size_t size() const;

    /**
     * Clears the Dictionary and leaves it in the same state as if it would just have been
     * created.
     */
    void clear();

    /**
     * Returns <code>true</code> if this Dictionary is empty; <code>false</code>
     * otherwise.
     *
     * \return <code>true</code> if this Dictionary is empty; <code>false</code> otherwise
     */
    bool empty() const;

    /**
     * Removes key and value pair stored under the, potentially nested, \p key. If the
     * \p key was found and successfully removed, <code>true</code> is returned, otherwise
     * the method returns <code>false</code>. Under no circumstances an error is logged.
     * \param key The, potentially nested, key pointing to the object that should be
     * deleted.
     *
     * \return Returns <code>true</code> if the key was successfully found and removed,
     *         <code>false</code> otherwise
     *
     * \pre \p key must not be empty
     */
    bool removeKey(const std::string& key);

private:
    /**
     * Splits the provided \p key into a \p first part and the \p rest. Provided a key
     * <code>a.b.c</code>, \p first will be assigned <code>a</code> and \p rest will be
     * assigned <code>b.c</code> after calling this method.
     *
     * \param key The key that should be split
     * \param first The first part of the key
     * \param rest The rest of the key
     * \return <code>true</code> if there was a rest of the key, <code>false</code> if no
     *         separator could be found
     */
    bool splitKey(const std::string& key, std::string& first, std::string& rest) const;

    /**
     * A helper function that is used by the <code>std::initializer_list</code>
     * constructor. Will determine the type in the <code>boost::any</code> and call the
     * correct version of #setValue%, which will do the conversion to the unified storage
     * format. Any type that is not explicitly handled here, is just passed on to the
     * #setValue function unchanged.
     *
     * \param key The key under which the \p value is stored
     * \param value The value that should be stored under the provided key
     */
    void setValueAnyHelper(std::string key, ghoul::any value);

    /**
     * This type is used in SFINAE evaluation of the internal methods (#setValueInternal,
     * #getValueInternal, and #hasValueInternal) and determines whether the type
     * <code>T</code> is a standard type (i.e., one of the types for which an automatic
     * conversion is defined) and if it is a scalar value (i.e., it only uses one storage
     * value).
     *
     * \tparam T The type to test
     */
    template <typename T>
    using IsStandardScalarType = std::enable_if_t<
        internal::has_storage_converter<T>::value &&
        internal::StorageTypeConverter<T>::size == 1
    >;

    /**
     * This type is used in SFINAE evaluation of the internal methods (#setValueInternal,
     * #getValueInternal, and #hasValueInternal) and determines whether the type
     * <code>T</code> is a standard type (i.e., one of the types for which an automatic
     * conversion is defined) and if it is not a scalar value (i.e., it uses more than one
     * storage value).
     *
     * \tparam T The type to test
     */
    template <typename T>
    using IsStandardVectorType = std::enable_if_t<
        internal::has_storage_converter<T>::value &&
        internal::StorageTypeConverter<T>::size != 1
    >;

    /**
     * This type is used in SFINAE evaluation of the internal methods (#setValueInternal,
     * #getValueInternal, and #hasValueInternal) and determines whether the type
     * <code>T</code> is a non-standard type (i.e., it is not one of the types for which
     * an automatic conversion is defined).
     *
     * \tparam T The type to test
     */
    template <typename T>
    using IsNonStandardType = std::enable_if_t<
        !internal::has_storage_converter<T>::value
    >;

    /**
     * Helper method to add the \p value into the Dictionary at the provided \p key. If
     * \p createIntermediate is <code>true</code>, intermediate keys in the Dictionary are
     * automatically created, otherwise an exception is thrown if a missing key is
     * encountered.
     *
     * \tparam T The type of the \p value that is to be stored
     * \param key The, possibly nested, location at which the \p value is stored
     * \param value The value to be stored in the Dictionary
     * \param createIntermediate If <code>true</code> intermediate levels in the
     *        Dictionary are created, otherwise, they already have to exist
     *
     * \sa setValue
     */
    template <typename T>
    void setValueHelper(std::string key, T value, CreateIntermediate createIntermediate);

    /**
     * Internal setValue implementation for types that are standard, scalar types. The
     * <code>IsStandardScalarType<T></code> parameter is only used for SFINAE evaluation
     * to remove this method from the overload set of unwanted <code>T</code>s.
     *
     * \tparam T The type of the \p value that is to be stored
     * \param key The, possibly nested, location at which the \p value is stored
     * \param value The value to be stored in the Dictionary
     * \param createIntermediate If <code>true</code> intermediate levels in the
     *        Dictionary are created, otherwise, they already have to exist
     *
     * \sa setValue
     */
    template <typename T>
    void setValueInternal(std::string key, T value, CreateIntermediate createIntermediate,
        IsStandardScalarType<T>* = nullptr);

    /**
     * Internal setValue implementation for types that are standard, non-scalar types. The
     * <code>IsStandardVectorType<T></code> parameter is only used for SFINAE evaluation
     * to remove this method from the overload set of unwanted <code>T</code>s.
     *
     * \tparam T The type of the \p value that is to be stored
     * \param key The, possibly nested, location at which the \p value is stored
     * \param value The value to be stored in the Dictionary
     * \param createIntermediate If <code>true</code> intermediate levels in the
     *        Dictionary are created, otherwise, they already have to exist
     *
     * \sa setValue
     */
    template <typename T>
    void setValueInternal(std::string key, T value, CreateIntermediate createIntermediate,
        IsStandardVectorType<T>* = nullptr);

    /**
     * Internal setValue implementation for types that are non-standard. The
     * <code>IsNonStandardType<T></code> parameter is only used for SFINAE evaluation to
     * remove this method from the overload set of unwanted <code>T</code>s.
     *
     * \tparam T The type of the \p value that is to be stored
     * \param key The, possibly nested, location at which the \p value is stored
     * \param value The value to be stored in the Dictionary
     * \param createIntermediate If <code>true</code> intermediate levels in the
     *        Dictionary are created, otherwise, they already have to exist
     *
     * \sa setValue
     */
    template <typename T>
    void setValueInternal(std::string key, T value, CreateIntermediate createIntermediate,
        IsNonStandardType<T>* = nullptr);

    /**
     * Helper method to retrieve the \p value from the provided \p key in the Dictionary.
     *
     * \tparam T The type of the \p value that is to be retrieved
     * \param key The location from which the \p value should be retrived
     * \param value The storage into which the value is written. If an error occurs, this
     *        value is unchanged
     */
    template <typename T>
    void getValueHelper(const std::string& key, T& value) const;

    /**
     * Internal getValue implementation for types that are standard, scalar types. The
     * <code>IsStandardScalarType<T></code> parameter is only used for SFINAE evaluation
     * to remove this method from the overload set of unwanted <code>T</code>s.
     *
     * \tparam T The type of the \p value that is to be retrieved
     * \param key The location from which the \p value should be retrived
     * \param value The storage into which the value is written. If an error occurs, this
     *        value is unchanged
     *
     * \sa getValue
     */
    template <typename T>
    void getValueInternal(const std::string& key, T& value,
        IsStandardScalarType<T>* = nullptr) const;

    /**
     * Internal getValue implementation for types that are standard, non-scalar types. The
     * <code>IsStandardVectorType<T></code> parameter is only used for SFINAE evaluation
     * to remove this method from the overload set of unwanted <code>T</code>s.
     *
     * \tparam T The type of the \p value that is to be retrieved
     * \param key The location from which the \p value should be retrived
     * \param value The storage into which the value is written. If an error occurs, this
     *        value is unchanged
     *
     * \sa getValue
     */
    template <typename T>
    void getValueInternal(const std::string& key, T& value,
        IsStandardVectorType<T>* = nullptr) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tvec2<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tvec3<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tvec4<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tmat2x2<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tmat2x3<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tmat2x4<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tmat3x2<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tmat3x3<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tmat3x4<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tmat4x2<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tmat4x3<T, P>& value) const;

    template <typename T, glm::precision P>
    void getValueInternal(const std::string& key, glm::tmat4x4<T, P>& value) const;

    /**
     * Internal getValue implementation for types that are non-standard. The
     * <code>IsNonStandardType<T></code> parameter is only used for SFINAE evaluation to
     * remove this method from the overload set of unwanted <code>T</code>s.
     *
     * \tparam T The type of the \p value that is to be retrieved
     * \param key The location from which the \p value should be retrived
     * \param value The storage into which the value is written. If an error occurs, this
     *        value is unchanged
     *
     * \sa getValue
     */
    template <typename T>
    void getValueInternal(const std::string& key, T& value,
        IsNonStandardType<T>* = nullptr) const;

    /**
     * Helper method to check whether this Dictionary has a value of a specific type
     * <code>T</code> at the, possibly nested, location \p key.
     *
     * \tparam T The type to be tested
     * \param key The key to be tested
     * \return <code>true</code> if the \p key exists and the stored value has the type
     *         <code>T</code>
     */
    template <typename T>
    bool hasValueHelper(const std::string& key) const;

    /**
     * Internal hasValue implementation for types that are standard, scalar types. The
     * <code>IsStandardScalarType<T></code> parameter is only used for SFINAE evaluation
     * to remove this method from the overload set of unwanted <code>T</code>s.
     *
     * \tparam T The type to be tested
     * \param key The key to be tested
     * \return <code>true</code> if the \p key exists and the stored value has the type
     *         <code>T</code>
     *
     * \sa hasValue
     */
    template <typename T>
    bool hasValueInternal(const std::string& key,
        IsStandardScalarType<T>* = nullptr) const;

    /**
     * Internal hasValue implementation for types that are standard, non-scalar types. The
     * <code>IsStandardVectorType<T></code> parameter is only used for SFINAE evaluation
     * to remove this method from the overload set of unwanted <code>T</code>s.
     *
     * \tparam T The type to be tested
     * \param key The key to be tested
     * \return <code>true</code> if the \p key exists and the stored value has the type
     *         <code>T</code>
     *
     * \sa hasValue
     */
    template <typename T>
    bool hasValueInternal(const std::string& key,
        IsStandardVectorType<T>* = nullptr) const;

    /**
     * Internal hasValue implementation for types that are non-standard. The
     * <code>IsNonStandardType<T></code> parameter is only used for SFINAE evaluation to
     * remove this method from the overload set of unwanted <code>T</code>s.
     *
     * \tparam T The type to be tested
     * \param key The key to be tested
     * \return <code>true</code> if the \p key exists and the stored value has the type
     *         <code>T</code>
     *
     * \sa hasValue
     */
    template <typename T>
    bool hasValueInternal(const std::string& key, IsNonStandardType<T>* = nullptr) const;
};

}  // namespace ghoul

#include "dictionary.inl"

#endif // __GHOUL___DICTIONARY___H__
