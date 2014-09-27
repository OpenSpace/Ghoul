/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
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

#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <boost/any.hpp>
#include <map>
#include <string>
#include <vector>

namespace ghoul {

/**
 * The Dictionary is a class to generically store arbitrary items associated with and
 * accessible using an <code>std::string</code>%s. It has the abilitiy to store and
 * retrieve these items by unique <code>std::string</code>-typed keys. The items that can
 * be stored have to be compatible with the <code>boost::any</code> type. For a select
 * list of types an automatic conversion is performed, so that simple type conversions are
 * possible. The following table is a complete listing of all types that are automatically
 * converted. Each type has a <code>StorageType</code> that is used to store the type
 * internally. Each type with the same number of values and the same
 * <code>StorageType</code> will be converted automatically. For example a value can be
 * stored as an <code>int</code> and can be retrieved as a <code>short</code>. The table
 * of all types is:
 * | <code>Type</code>               | <code>StorageType</code>  | <code>#Values</code> |
 * |:-------------------------------:|:-------------------------:|:--------------------:|
 * | <code>bool</code>               | <code>IntegralType</code>         |  1           |
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
 *  exception to this is the #setValue method, which has an additional parameter that
 * controls if each individual level of the Dictionary is created on-the-fly or not. None
 * of the method will throw an exception of their own, but might pass exceptions from
 * the underlying systems, for example <code>boost::any</code>.
 */
class Dictionary : private std::map<std::string, boost::any> {
public:
    /**
     * Creates an empty Dictionary
     */
    Dictionary() = default;

    /**
     * Creates a Dictionary out of the provided <code>std::initializer_list</code>. This
     * initializer list can be, for example, of the format
     * <code>{ { "a", 1 }, { "b", 2 } }</code> and it will add all of the
     * <code>std::pair</code>%s provided to the Dictionary.
	 * \param l The <code>std::initializer_list</code> that contains all of the values
	 * that should be added to the Dictionary
     */
    Dictionary(std::initializer_list<std::pair<std::string, boost::any>> l);

    /**
     * Returns all of the keys that are stored in the dictionary at a given
     * <code>location</code>. This location specifier can be recursive to inspect the keys
     * at deeper levels.
     * \param location The location for which all keys should be returned
     * \return A list of all keys that are stored in the Dictionary for the provided
     * location
     */
    std::vector<std::string> keys(const std::string& location = "") const;

    /**
     * Returns <code>true</code> if there is a specific key in the Dictionary, regardless
     * of its type. <code>key</code> can be a nested location to search for keys at deeper
     * levels.
     * \param key The key that should be checked for existence
     * \return <code>true</code> if the provided key exists in the Dictionary,
     * <code>false</code> otherwise
     */
    bool hasKey(const std::string& key) const;

    /**
     * Adds the <code>value</code> for a given location at <code>key</code>. If a value
     * already exists at that key, the old value is overwritten, regardless of its
     * previous type and without any feedback. The <code>key</code> parameter can be
     * specified as a nested location to add values to deeper levels of the Dictionary.
     * If <code>createIntermediate</code> is <code>true</code> all intermediate
     * levels in the Dictionary are created automatically; if it is <code>false</code>,
     * this function will fail and return <code>false</code> if a non-existing level is
     * encountered.
     * \param key The key at which the <code>value</code> will be entered. This can be a
     * nested location to insert value are deeper levels. Depending on the value of the
     * <code>createIntermediate</code> parameter, all intermediate levels already have to
     * exist in the Dictionary or will be created on-the-fly.
     * \param value The value that will be added into the Dictionary. If the type supports
     * it, the value will only be moved rather than copied.
     * \param createIntermediate If <code>true</code> all intermediate levels in the
     * Dictionary will be automatically created along the way, if the provided
     * <code>key</code> contains a nested location. If <code>false</code> the method will
     * fail and return <code>false</code> if a missing intermediate level is encountered.
     * \return <code>true</code> if the value was stored successfully, <code>false</code>
     * otherwise
     */
    template <typename T>
    bool setValue(std::string key, T value, bool createIntermediate = false);

    /**
     * Returns the value stored at location with a given <code>key</code>. This key can be
     * nested and will automatically be decomposed by the method to traverse to deeper
     * levels of the Dictionary. If the Dictionary has a value at the provided key and the
     * type agrees with the template parameter, the value will be copied into the provided
     * reference <code>value</code> and the method returns <code>true</code>. If either
     * the key does not exist or the type does not agree, <code>false</code> is returned
     * and the <code>value</code> is unchanged. Only if the type of the key and the
	 * requested type <code>T</code> disagree, an error is logged. If the key does not
	 * exist in the dictionary, the error is silently ignored if the code was compiled
	 * with GHL_DEBUG.
     * \tparam The type of the value that should be tested. The <code>typeid</code> of
     * this type has to be equal to the typeid of the value that is to be retrieved
     * \param key The, potentially nested, key for which the stored value should be
     * returned
     * \param value A reference to the value where the value will be copied to, if it
     * could be found and the types agree
     * \return <code>true</code> if the value was retrieved successfully,
     * <code>false</code> otherwise
     */
    template <typename T>
    bool getValue(const std::string& key, T& value) const;

    /**
     * Returns <code>true</code> if the Dictionary stores a value at the provided
     * <code>key</code> and the stored type agrees with the provided template parameter.
     * The key can be nested and refer to deeper levels of the Dictionary. If any of the
     * intermediate levels does not exists, <code>false</code> is returned.
     * \tparam The type of the value that should be tested. The <code>typeid</code> of
     * this type has to be equal to the typeid of the value that is to be checked
     * \param key The, potentially nested, key which should be checked for existence
     * \return <code>true</code> if the Dictionary contains a value at the specified
     * <code>key</code> with the correct type <code>T</code>. Will return
     * <code>false</code> otherwise
     */
    template <typename T>
    bool hasValue(const std::string& key) const;
    
    /**
     * Returns <code>true</code> if the Dictionary contains a value for the specified
     * <code>key</code> and the value that is stored is of the type <code>T</code>. This
     * method calls the Dictionary::hasKey and Dictionary::hasValue methods.
     * \tparam T The type of the value that should be tested
     * \param key The key that should be tested
     * \return <code>true</code> if the Dictionary contains a value for the
     * <code>key</code> and the value is of type <code>T</code>
     */
    template <typename T>
    bool hasKeyAndValue(const std::string& key) const;

    /**
     * Returns the total number of keys stored in this Dictionary. This method will not
     * recurse into sub-Dictionaries, but will only return the top-level keys for the
     * Dictionary it is called on. This value will always be equal to the size of the
     * <code>std::vector</code> returned by #keys.
     * \return The number of keys stored in this Dictionary
     */
    size_t size() const;

    /**
     * Clears the Dictionary and leaves it in the same state as if it would just have been
     * created.
     */
    void clear();

	/**
	 * Removes key and value pair stored under the, potentially nested, <code>key</code>.
	 * If the <code>key</code> was found and successfully removed, <code>true</code> is
	 * returned, otherwise the method returns <code>false</code>. Under no circumstances
	 * an error is logged.
	 * \param key The, potentially nested, key pointing to the object that should be
	 * deleted
	 * \return Returns <code>true</code> if the key was successfully found and removed,
	 * <code>false</code> otherwise
	 */
	bool removeKey(const std::string& key);

protected:
    /**
     * Splits the provided <code>key</code> into a <code>first</code> part and the
     * <code>rest</code>. Provided a key <code>a.b.c</code>, <code>first</code> will be
     * assigned <code>a</code> and <code>rest</code> will be assigned <code>b.c</code>
     * after calling this method.
     * \param key The key that should be split
     * \param first The first part of the key
     * \param rest The rest of the key
     * \return <code>true</code> if there was a rest of the key, <code>false</code> if no
     * separator could be found
     */
    bool splitKey(const std::string& key, std::string& first, std::string& rest) const;

    /**
     * A helper function that is used by the <code>std::initializer_list</code>
     * constructor. Will determine the type in the <code>boost::any</code> and call the
     * correct version of #setValue%, which will do the conversion to the unified storage
     * format. Any type that is not explicitly handled here, is just passed on to the
     * #setValue function unchanged.
     * \param key The key under which the <code>value</code> is stored
     * \param value The value that should be stored under the provided key
     */
    void setValueAnyHelper(std::string key, boost::any value);

    template <typename T>
    bool setValueHelper(std::string key, T value, bool createIntermediate);

    template <typename T>
    bool getValueHelper(const std::string& key, T& value) const;

    template <typename T>
    bool hasValueHelper(const std::string& key) const;
};

}  // namespace ghoul

#include "dictionary.inl"

#endif
