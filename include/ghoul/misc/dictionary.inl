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

#include <ghoul/fmt.h>
#include <ghoul/misc/assert.h>
#include <algorithm>

namespace ghoul {

template <typename T>
bool isConvertible(const Dictionary& dict) {
    using StorageType = internal::StorageTypeConverter<T>;

    const bool correctSize = dict.size() == StorageType::size;
    if (!correctSize) {
        return false;
    }

    const std::vector<std::string>& keys = dict.keys();
    for (size_t i = 0; i < StorageType::size; ++i) {
        const std::string& key = keys[i];
        #ifdef WIN32
        #pragma warning(push)
        // Suppress the warning C2684 (Redundant test) that occurs if
        // StorageTypeConverter<TargetType>::type == TargetType
        #pragma warning(suppress: 6287)
        #endif // WIN32
        const bool correctType = dict.hasValue<typename StorageType::type>(key) ||
                                 dict.hasValue<T>(key);
        #ifdef WIN32
        #pragma warning(pop)
        #endif // WIN32
        if (!correctType) {
            return false;
        }
    }
    return true;
}

///////////
// setValue
///////////

template <typename T>
void Dictionary::setValueHelper(std::string key, T value,
                                CreateIntermediate createIntermediate)
{
    std::string first;
    std::string rest;
    const bool hasRestPath = splitKey(key, first, rest);
    if (!hasRestPath) {
        // if no rest exists, key == first and we can just insert the value
        (*this)[std::move(key)] = std::move(value);
        return;
    }

    // if we get to this point, the 'key' did contain a nested key
    // so we have to find the correct Dictionary (or create it if it doesn't exist)
    auto keyIt = find(first);
    if (keyIt == cend()) {
        // didn't find the Dictionary
        if (createIntermediate) {
            (*this)[first] = ghoul::Dictionary();
            keyIt = find(first);
        }
        else {
            throw KeyError(fmt::format(
                "Intermediate key '{}' was not found in dictionary", first
            ));
        }
    }

    // See if it is actually a Dictionary at this location
    Dictionary* dict = ghoul::any_cast<Dictionary>(&(keyIt->second));
    if (dict == nullptr) {
        throw ConversionError(fmt::format(
            "error converting key '{}' from type '{}' to type 'Dictionary'",
            first,
            keyIt->second.type().name()
        ));
    }
    // Proper tail-recursion
    dict->setValue(std::move(rest), std::move(value), createIntermediate);
}

template <typename T>
void Dictionary::setValueInternal(std::string key, T value,
                                  CreateIntermediate createIntermediate,
                                  IsStandardScalarType<T>*)
{
    setValueHelper(
        std::move(key),
        static_cast<typename internal::StorageTypeConverter<T>::type&&>(std::move(value)),
        createIntermediate
    );
}

template <typename T>
void Dictionary::setValueInternal(std::string key, T value,
                                  CreateIntermediate createIntermediate,
                                  IsStandardVectorType<T>*)
{
    using StorageType = internal::StorageTypeConverter<T>;

    std::array<typename StorageType::type, StorageType::size> v;
    typename T::value_type const * ptr = glm::value_ptr(value);
    for (size_t i = 0; i < StorageType::size; ++i) {
        v[i] = static_cast<typename StorageType::type>(ptr[i]);
    }
    setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <typename T>
void Dictionary::setValueInternal(std::string key, T value,
                                  CreateIntermediate createIntermediate,
                                  IsNonStandardType<T>*)
{
    setValueHelper(std::move(key), std::move(value), createIntermediate);
}

template <typename T>
void Dictionary::setValue(std::string key, T value,
    CreateIntermediate createIntermediate)
{
    ghoul_assert(!key.empty(), "Key must not be empty");
    setValueInternal(std::move(key), std::move(value), createIntermediate);
}

///////////
// getValue
///////////

template <typename T>
void ghoul::Dictionary::getValueHelper(const std::string& key, T& value) const {
    // If we can find the key directly, we can return it immediately
    auto it = find(key);
    if (it != cend()) {
        const T* v = ghoul::any_cast<T>(&(it->second));
        // See if it has the correct type
        if (!v) {
            throw ConversionError(fmt::format(
                "Wrong type for key '{}': Expected '{}' got '{}'",
                key,
                typeid(T).name(),
                it->second.type().name()
            ));
        }
        value = *v;
        return;
    }

    // if we get to this point, the 'key' did contain a nested key
    // so we have to find the correct Dictionary (or create it if it doesn't exist)
    std::string first;
    std::string rest;
    splitKey(key, first, rest);

    auto keyIt = find(first);
    if (keyIt == cend()) {
        throw KeyError(fmt::format("Could not find key '{}' in Dictionary", first));
    }

    const Dictionary* dict = ghoul::any_cast<Dictionary>(&(keyIt->second));
    // See if it is actually a Dictionary at this location
    if (!dict) {
        throw ConversionError(fmt::format(
            "Error converting key '{}' from type '{}' to type 'Dictionary'",
            first,
            keyIt->second.type().name()
        ));
    }
    // Proper tail-recursion
    dict->getValue<T>(rest, value);
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#endif // _MSC_VER
template <typename T>
void Dictionary::getValueInternal(const std::string& key, T& value,
                                                           IsStandardScalarType<T>*) const
{
    const bool keyExists = hasKey(key);
    if (!keyExists) {
        throw KeyError(fmt::format("Key '{}' did not exist in Dictionary", key));
    }
    bool success = hasValueHelper<typename internal::StorageTypeConverter<T>::type>(key);
    if (success) {
        typename internal::StorageTypeConverter<T>::type v;
        getValueHelper(key, v);
        value = static_cast<T>(v);
        return;
    }
    else {
        bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<T>(dict);
            if (canConvert) {
                const std::vector<std::string>& keys = dict.keys();
                for (size_t i = 0; i < internal::StorageTypeConverter<T>::size; ++i) {
                    const std::string& k = keys[i];
                    dict.getValue(k, value);
                    return;
                }
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(T).name()
    ));
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tvec2<T, P>& value) const {
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tvec2<T, P>>::type,
        internal::StorageTypeConverter<glm::tvec2<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value.x = static_cast<T>(v[0]);
        value.y = static_cast<T>(v[1]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tvec2<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tvec2<T, P>>::size;
                     ++i)
                {
                    const std::string& k = keys[i];
                    T v;
                    dict.getValue(k, v);
                    value[static_cast<typename glm::tvec2<T, P>::length_type>(i)] =
                        std::move(v);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tvec2<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tvec3<T, P>& value) const {
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tvec3<T, P>>::type,
        internal::StorageTypeConverter<glm::tvec3<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value.x = static_cast<T>(v[0]);
        value.y = static_cast<T>(v[1]);
        value.z = static_cast<T>(v[2]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tvec3<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tvec3<T, P>>::size;
                     ++i)
                {
                    const std::string& k = keys[i];
                    T v;
                    dict.getValue(k, v);
                    value[static_cast<typename glm::tvec3<T, P>::length_type>(i)] =
                        std::move(v);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tvec3<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tvec4<T, P>& value) const {
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tvec4<T, P>>::type,
        internal::StorageTypeConverter<glm::tvec4<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value.x = static_cast<T>(v[0]);
        value.y = static_cast<T>(v[1]);
        value.z = static_cast<T>(v[2]);
        value.w = static_cast<T>(v[3]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tvec4<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tvec4<T, P>>::size;
                     ++i) {
                    const std::string& k = keys[i];
                    T v;
                    dict.getValue(k, v);
                    value[static_cast<typename glm::tvec4<T, P>::length_type>(i)] =
                        std::move(v);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tvec4<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tmat2x2<T, P>& value) const
{
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tmat2x2<T, P>>::type,
        internal::StorageTypeConverter<glm::tmat2x2<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value[0][0] = static_cast<T>(v[0]);
        value[0][1] = static_cast<T>(v[1]);
        value[1][0] = static_cast<T>(v[2]);
        value[1][1] = static_cast<T>(v[3]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tmat2x2<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tmat2x2<T, P>>::size;
                     ++i) {
                    const std::string& k = keys[i];
                    dict.getValue(k, glm::value_ptr(value)[i]);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tmat2x2<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tmat2x3<T, P>& value) const
{
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tmat2x3<T, P>>::type,
        internal::StorageTypeConverter<glm::tmat2x3<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value[0][0] = static_cast<T>(v[0]);
        value[0][1] = static_cast<T>(v[1]);
        value[0][2] = static_cast<T>(v[2]);
        value[1][0] = static_cast<T>(v[3]);
        value[1][1] = static_cast<T>(v[4]);
        value[1][2] = static_cast<T>(v[5]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tmat2x3<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tmat2x3<T, P>>::size;
                     ++i) {
                    const std::string& k = keys[i];
                    dict.getValue(k, glm::value_ptr(value)[i]);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tmat2x3<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tmat2x4<T, P>& value) const
{
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tmat2x4<T, P>>::type,
        internal::StorageTypeConverter<glm::tmat2x4<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value[0][0] = static_cast<T>(v[0]);
        value[0][1] = static_cast<T>(v[1]);
        value[0][2] = static_cast<T>(v[2]);
        value[0][3] = static_cast<T>(v[3]);
        value[1][0] = static_cast<T>(v[4]);
        value[1][1] = static_cast<T>(v[5]);
        value[1][2] = static_cast<T>(v[6]);
        value[1][3] = static_cast<T>(v[7]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tmat2x4<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tmat2x4<T, P>>::size;
                     ++i) {
                    const std::string& k = keys[i];
                    dict.getValue(k, glm::value_ptr(value)[i]);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tmat2x4<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tmat3x2<T, P>& value) const
{
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tmat3x2<T, P>>::type,
        internal::StorageTypeConverter<glm::tmat3x2<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value[0][0] = static_cast<T>(v[0]);
        value[0][1] = static_cast<T>(v[1]);
        value[1][0] = static_cast<T>(v[2]);
        value[1][1] = static_cast<T>(v[3]);
        value[2][0] = static_cast<T>(v[4]);
        value[2][1] = static_cast<T>(v[5]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tmat3x2<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tmat3x2<T, P>>::size;
                     ++i) {
                    const std::string& k = keys[i];
                    dict.getValue(k, glm::value_ptr(value)[i]);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tmat3x2<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tmat3x3<T, P>& value) const
{
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tmat3x3<T, P>>::type,
        internal::StorageTypeConverter<glm::tmat3x3<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value[0][0] = static_cast<T>(v[0]);
        value[0][1] = static_cast<T>(v[1]);
        value[0][2] = static_cast<T>(v[2]);
        value[1][0] = static_cast<T>(v[3]);
        value[1][1] = static_cast<T>(v[4]);
        value[1][2] = static_cast<T>(v[5]);
        value[2][0] = static_cast<T>(v[6]);
        value[2][1] = static_cast<T>(v[7]);
        value[2][2] = static_cast<T>(v[8]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tmat3x3<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tmat3x3<T, P>>::size;
                     ++i) {
                    const std::string& k = keys[i];
                    dict.getValue(k, glm::value_ptr(value)[i]);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tmat3x3<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tmat3x4<T, P>& value) const
{
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tmat3x4<T, P>>::type,
        internal::StorageTypeConverter<glm::tmat3x4<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value[0][0] = static_cast<T>(v[0]);
        value[0][1] = static_cast<T>(v[1]);
        value[0][2] = static_cast<T>(v[2]);
        value[0][3] = static_cast<T>(v[3]);
        value[1][0] = static_cast<T>(v[4]);
        value[1][1] = static_cast<T>(v[5]);
        value[1][2] = static_cast<T>(v[6]);
        value[1][3] = static_cast<T>(v[7]);
        value[2][0] = static_cast<T>(v[8]);
        value[2][1] = static_cast<T>(v[9]);
        value[2][2] = static_cast<T>(v[10]);
        value[2][3] = static_cast<T>(v[11]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tmat3x4<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tmat3x4<T, P>>::size;
                     ++i) {
                    const std::string& k = keys[i];
                    dict.getValue(k, glm::value_ptr(value)[i]);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tmat3x4<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tmat4x2<T, P>& value) const
{
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tmat4x2<T, P>>::type,
        internal::StorageTypeConverter<glm::tmat4x2<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value[0][0] = static_cast<T>(v[0]);
        value[0][1] = static_cast<T>(v[1]);
        value[1][0] = static_cast<T>(v[2]);
        value[1][1] = static_cast<T>(v[3]);
        value[2][0] = static_cast<T>(v[4]);
        value[2][1] = static_cast<T>(v[5]);
        value[3][0] = static_cast<T>(v[6]);
        value[3][1] = static_cast<T>(v[7]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tmat4x2<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tmat4x2<T, P>>::size;
                     ++i) {
                    const std::string& k = keys[i];
                    dict.getValue(k, glm::value_ptr(value)[i]);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tmat4x2<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tmat4x3<T, P>& value) const
{
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tmat4x3<T, P>>::type,
        internal::StorageTypeConverter<glm::tmat4x3<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value[0][0] = static_cast<T>(v[0]);
        value[0][1] = static_cast<T>(v[1]);
        value[0][2] = static_cast<T>(v[2]);
        value[1][0] = static_cast<T>(v[3]);
        value[1][1] = static_cast<T>(v[4]);
        value[1][2] = static_cast<T>(v[5]);
        value[2][0] = static_cast<T>(v[6]);
        value[2][1] = static_cast<T>(v[7]);
        value[2][2] = static_cast<T>(v[8]);
        value[3][0] = static_cast<T>(v[9]);
        value[3][1] = static_cast<T>(v[10]);
        value[3][2] = static_cast<T>(v[11]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tmat4x3<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tmat4x3<T, P>>::size;
                     ++i) {
                    const std::string& k = keys[i];
                    dict.getValue(k, glm::value_ptr(value)[i]);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tmat4x3<T, P>).name()
    ));
}

template <typename T, glm::precision P>
void Dictionary::getValueInternal(const std::string& key, glm::tmat4x4<T, P>& value) const
{
    using Array = std::array<
        typename internal::StorageTypeConverter<glm::tmat4x4<T, P>>::type,
        internal::StorageTypeConverter<glm::tmat4x4<T, P>>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        value[0][0] = static_cast<T>(v[0]);
        value[0][1] = static_cast<T>(v[1]);
        value[0][2] = static_cast<T>(v[2]);
        value[0][3] = static_cast<T>(v[3]);
        value[1][0] = static_cast<T>(v[4]);
        value[1][1] = static_cast<T>(v[5]);
        value[1][2] = static_cast<T>(v[6]);
        value[1][3] = static_cast<T>(v[7]);
        value[2][0] = static_cast<T>(v[8]);
        value[2][1] = static_cast<T>(v[9]);
        value[2][2] = static_cast<T>(v[10]);
        value[2][3] = static_cast<T>(v[11]);
        value[3][0] = static_cast<T>(v[12]);
        value[3][1] = static_cast<T>(v[13]);
        value[3][2] = static_cast<T>(v[14]);
        value[3][3] = static_cast<T>(v[15]);

        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::tmat4x4<T, P>>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    }
                    catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0;
                     i < internal::StorageTypeConverter<glm::tmat4x4<T, P>>::size;
                     ++i) {
                    const std::string& k = keys[i];
                    dict.getValue(k, glm::value_ptr(value)[i]);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(glm::tmat4x4<T, P>).name()
    ));
}

template <typename T>
void Dictionary::getValueInternal(const std::string& key, T& value,
                                                           IsStandardVectorType<T>*) const
{
    using Array = std::array<
        typename internal::StorageTypeConverter<T>::type,
        internal::StorageTypeConverter<T>::size
    >;

    const bool success = hasValueHelper<Array>(key);
    if (success) {
        Array v;
        getValueHelper(key, v);

        memcpy(glm::value_ptr(value), v.data(), sizeof(T));
        return;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<T>(dict);
            if (canConvert) {
                std::vector<std::string> keys = dict.keys();
                // sort numerically rather than by ASCII value
                std::sort(keys.begin(), keys.end(), [](const auto& k1, const auto& k2) {
                    try {
                        return std::stoi(k1) < std::stoi(k2);
                    } catch (const std::invalid_argument&) {
                        return k1 < k2;
                    }
                });
                for (size_t i = 0; i < internal::StorageTypeConverter<T>::size; ++i) {
                    const std::string& key = keys[i];
                    dict.getValue(key, glm::value_ptr(value)[i]);
                }
                return;
            }
        }
    }
    throw ConversionError(fmt::format(
        "Error converting key '{}' from type '{}' to type '{}'",
        key,
        find(key)->second.type().name(),
        typeid(T).name()
    ));
}

template <typename T>
void Dictionary::getValueInternal(const std::string& key, T& value,
                                                              IsNonStandardType<T>*) const
{
    getValueHelper(key, value);
}

template <typename T>
bool Dictionary::getValue(const std::string& key, T& value) const {
    ghoul_assert(!key.empty(), "Key must not be empty");

    try {
        const bool keyExists = hasKey(key);
        if (!keyExists) {
            return false;
        }
        else {
            getValueInternal(key, value);
            return true;
        }
    }
    catch (const DictionaryError&) {
        return false;
    }
}

template <typename T>
T ghoul::Dictionary::value(const std::string& key) const {
    ghoul_assert(!key.empty(), "Key must not be empty");

    T value;
    getValueInternal(key, value);
    return value;
}

///////////
// hasValue
///////////

template <typename T>
bool ghoul::Dictionary::hasValueHelper(const std::string& key) const {
    auto it = find(key);
    if (it != cend()) {
        // If we can find the key directly, we can check the types and return
        bool typeCorrect = (it->second.type() == typeid(T));
        return typeCorrect;
    }

    std::string first;
    std::string rest;
    splitKey(key, first, rest);

    auto keyIt = find(first);
    if (keyIt == cend()) {
        // If we can't find the first part of nested key, there is no need to continue
        return false;
    }

    const Dictionary* dict = ghoul::any_cast<Dictionary>(&(keyIt->second));
    if (!dict) {
        // If it is not a Dictionary, the value can't be found and no recursion necessary
        return false;
    }

    // Proper tail-recursion
    return dict->hasValue<T>(rest);
}

template <typename T>
bool Dictionary::hasValueInternal(const std::string& key, IsStandardScalarType<T>*) const
{
    bool val = hasValueHelper<typename internal::StorageTypeConverter<T>::type>(key);
    if (val) {
        return true;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<T>(dict);
            if (canConvert) {
                return true;
            }
        }
        return false;
    }
}

template <typename T>
bool Dictionary::hasValueInternal(const std::string& key, IsStandardVectorType<T>*) const
{
    const bool val = hasValueHelper<std::array<
        typename internal::StorageTypeConverter<T>::type,
        internal::StorageTypeConverter<T>::size
    >>(key);
    if (val) {
        return true;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<T>(dict);
            if (canConvert) {
                return true;
            }
        }
        return false;
    }
}

template <typename T>
bool Dictionary::hasValueInternal(const std::string& key, IsNonStandardType<T>*) const {
    return hasValueHelper<T>(key);
}

template <typename T>
bool Dictionary::hasValue(const std::string& key) const {
    ghoul_assert(!key.empty(), "Key must not be empty");

    return hasValueInternal<T>(key);
}

template <typename T>
bool Dictionary::hasKeyAndValue(const std::string& key) const {
    ghoul_assert(!key.empty(), "Key must not be empty");

    // Short-circuit evaluation is used to guard the 'hasValue' function from non-existing
    // keys
    return (hasKey(key) && hasValue<T>(key));
}

// Extern define template declaration such that the compiler won't try to instantiate each
// member function individually whenever it is encountered. The definitions are located
// in the dictionary.cpp compilation unit

#define EXTERN_TEMPLATE_DECLARATION(__TYPE__) \
extern template void Dictionary::setValue<__TYPE__>(std::string, __TYPE__,               \
    CreateIntermediate);                                                                 \
extern template bool Dictionary::getValue<__TYPE__>(const std::string&, __TYPE__&) const;\
extern template __TYPE__ Dictionary::value<__TYPE__>(const std::string&) const;          \
extern template bool Dictionary::hasValue<__TYPE__>(const std::string&) const;           \
extern template bool isConvertible<__TYPE__>(const Dictionary&);                         \
extern template void Dictionary::setValueHelper<__TYPE__>(std::string, __TYPE__,         \
    CreateIntermediate);                                                                 \
extern template void Dictionary::getValueHelper<__TYPE__>(const std::string&,            \
                                                                       __TYPE__&) const; \
extern template bool Dictionary::hasValueHelper<__TYPE__>(const std::string&) const;

EXTERN_TEMPLATE_DECLARATION(char);
EXTERN_TEMPLATE_DECLARATION(signed char);
EXTERN_TEMPLATE_DECLARATION(unsigned char);
EXTERN_TEMPLATE_DECLARATION(wchar_t);
EXTERN_TEMPLATE_DECLARATION(short);
EXTERN_TEMPLATE_DECLARATION(unsigned short);
EXTERN_TEMPLATE_DECLARATION(int);
EXTERN_TEMPLATE_DECLARATION(unsigned int);
EXTERN_TEMPLATE_DECLARATION(long long);
EXTERN_TEMPLATE_DECLARATION(unsigned long long);
EXTERN_TEMPLATE_DECLARATION(float);
EXTERN_TEMPLATE_DECLARATION(double);
EXTERN_TEMPLATE_DECLARATION(glm::vec2);
EXTERN_TEMPLATE_DECLARATION(glm::dvec2);
EXTERN_TEMPLATE_DECLARATION(glm::ivec2);
EXTERN_TEMPLATE_DECLARATION(glm::uvec2);
EXTERN_TEMPLATE_DECLARATION(glm::bvec2);
EXTERN_TEMPLATE_DECLARATION(glm::vec3);
EXTERN_TEMPLATE_DECLARATION(glm::dvec3);
EXTERN_TEMPLATE_DECLARATION(glm::ivec3);
EXTERN_TEMPLATE_DECLARATION(glm::uvec3);
EXTERN_TEMPLATE_DECLARATION(glm::bvec3);
EXTERN_TEMPLATE_DECLARATION(glm::vec4);
EXTERN_TEMPLATE_DECLARATION(glm::dvec4);
EXTERN_TEMPLATE_DECLARATION(glm::ivec4);
EXTERN_TEMPLATE_DECLARATION(glm::uvec4);
EXTERN_TEMPLATE_DECLARATION(glm::bvec4);
EXTERN_TEMPLATE_DECLARATION(glm::mat2x2);
EXTERN_TEMPLATE_DECLARATION(glm::mat2x3);
EXTERN_TEMPLATE_DECLARATION(glm::mat2x4);
EXTERN_TEMPLATE_DECLARATION(glm::mat3x2);
EXTERN_TEMPLATE_DECLARATION(glm::mat3x3);
EXTERN_TEMPLATE_DECLARATION(glm::mat3x4);
EXTERN_TEMPLATE_DECLARATION(glm::mat4x2);
EXTERN_TEMPLATE_DECLARATION(glm::mat4x3);
EXTERN_TEMPLATE_DECLARATION(glm::mat4x4);
EXTERN_TEMPLATE_DECLARATION(glm::dmat2x2);
EXTERN_TEMPLATE_DECLARATION(glm::dmat2x3);
EXTERN_TEMPLATE_DECLARATION(glm::dmat2x4);
EXTERN_TEMPLATE_DECLARATION(glm::dmat3x2);
EXTERN_TEMPLATE_DECLARATION(glm::dmat3x3);
EXTERN_TEMPLATE_DECLARATION(glm::dmat3x4);
EXTERN_TEMPLATE_DECLARATION(glm::dmat4x2);
EXTERN_TEMPLATE_DECLARATION(glm::dmat4x3);
EXTERN_TEMPLATE_DECLARATION(glm::dmat4x4);

#undef EXTERN_TEMPLATE_DECLARATION

template <>
bool Dictionary::getValue<Dictionary>(const std::string& key, Dictionary& value) const;

template <>
bool Dictionary::getValue<std::string>(const std::string& key, std::string& value) const;

template <>
std::string Dictionary::value<std::string>(const std::string& key) const;

} // namespace ghoul
