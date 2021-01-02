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

#include <ghoul/misc/dictionary.h>

#include <ghoul/misc/assert.h>

namespace ghoul {

Dictionary::KeyError::KeyError(std::string msg)
    : ghoul::RuntimeError(std::move(msg), "Dictionary")
{}

Dictionary::ValueError::ValueError(std::string key, std::string msg)
    : ghoul::RuntimeError(
        fmt::format("Key '{}': {}", std::move(key), std::move(msg)),
        "Dictionary"
    )
{}

template <typename T, std::enable_if_t<Dictionary::IsAllowed<T>{}, int>>
void Dictionary::setValue(std::string key, T value) {
    if constexpr (
        std::is_same_v<T, bool> || std::is_same_v<T, double> ||
        std::is_same_v<T, int> || std::is_same_v<T, std::string> ||
        std::is_same_v<T, Dictionary> || std::is_same_v<T, std::vector<int>> ||
        std::is_same_v<T, std::vector<double>>)
    {
        _storage.insert_or_assign(std::move(key), std::move(value));
    }
    else if constexpr (
        std::is_same_v<T, glm::ivec2> || std::is_same_v<T, glm::ivec3> ||
        std::is_same_v<T, glm::ivec4> || std::is_same_v<T, glm::dvec2> ||
        std::is_same_v<T, glm::dvec3> || std::is_same_v<T, glm::dvec4>)
    {
        T::value_type *ptr = glm::value_ptr(value);
        std::vector<T::value_type> vec(ptr, ptr + T::length());
        _storage.insert_or_assign(std::move(key), std::move(vec));
    }
    else if constexpr (
        std::is_same_v<T, glm::dmat2x2> || std::is_same_v<T, glm::dmat2x3> ||
        std::is_same_v<T, glm::dmat2x4> || std::is_same_v<T, glm::dmat3x2> ||
        std::is_same_v<T, glm::dmat3x3> || std::is_same_v<T, glm::dmat3x4> ||
        std::is_same_v<T, glm::dmat4x2> || std::is_same_v<T, glm::dmat4x3> ||
        std::is_same_v<T, glm::dmat4x4>)
    {
        T::value_type* ptr = glm::value_ptr(value);
        std::vector<T::value_type> vec(
            ptr,
            ptr + T::col_type::length() * T::row_type::length()
        );
        _storage.insert_or_assign(std::move(key), std::move(vec));
    }
    else {
        static_assert(false, "Unsupported type");
    }
}

template <typename T, std::enable_if_t<Dictionary::IsAllowed<T>{}, int>>
T Dictionary::value(std::string_view key) const {
    const size_t dotPos = key.find('.');
    if (dotPos != std::string_view::npos) {
        std::string_view before = key.substr(0, dotPos);
        std::string_view after = key.substr(dotPos + 1);

        ghoul::Dictionary d = value<ghoul::Dictionary>(before);
        return d.value<T>(after);
    }
    else {
        auto it = _storage.find(key);
        if (it == _storage.end()) {
            throw KeyError(fmt::format("Could not find key '{}'", key));
        }

        if constexpr (
            std::is_same_v<T, bool>       || std::is_same_v<T, int> ||
            std::is_same_v<T, double>     || std::is_same_v<T, std::string> ||
            std::is_same_v<T, Dictionary> || std::is_same_v<T, std::vector<int>> ||
            std::is_same_v<T, std::vector<double>>)
        {
            return std::get<T>(it->second);
        }
        else if constexpr (
            std::is_same_v<T, glm::ivec2>   || std::is_same_v<T, glm::ivec3> ||
            std::is_same_v<T, glm::ivec4>   || std::is_same_v<T, glm::dvec2> ||
            std::is_same_v<T, glm::dvec3>   || std::is_same_v<T, glm::dvec4> ||
            std::is_same_v<T, glm::dmat2x2> || std::is_same_v<T, glm::dmat2x3> ||
            std::is_same_v<T, glm::dmat2x4> || std::is_same_v<T, glm::dmat3x2> ||
            std::is_same_v<T, glm::dmat3x3> || std::is_same_v<T, glm::dmat3x4> ||
            std::is_same_v<T, glm::dmat4x2> || std::is_same_v<T, glm::dmat4x3> ||
            std::is_same_v<T, glm::dmat4x4>)
        {
            using VT = std::vector<T::value_type>;
            VT vec;
            if (std::holds_alternative<VT>(it->second)) {
                vec = std::get<VT>(it->second);
            }
            else if (std::holds_alternative<ghoul::Dictionary>(it->second)) {
                ghoul::Dictionary d = std::get<ghoul::Dictionary>(it->second);
                vec.resize(d._storage.size());
                for (const auto& kv : d._storage) {
                    // Lua is 1-based index, the rest of the world is 0-based
                    int k = std::stoi(kv.first) - 1;
                    vec[k] = std::get<T::value_type>(kv.second);
                }
            }
            else {
                throw ValueError(
                    std::string(key),
                    fmt::format(
                        "Requested {} but did not contain {} or {}",
                        typeid(T).name(), typeid(T).name(),
                        typeid(std::vector<T::value_type>).name()
                    )
                );
            }

            size_t expectedSize;
            if constexpr (
                std::is_same_v<T, glm::ivec2> || std::is_same_v<T, glm::ivec3> ||
                std::is_same_v<T, glm::ivec4> || std::is_same_v<T, glm::dvec2> ||
                std::is_same_v<T, glm::dvec3> || std::is_same_v<T, glm::dvec4>)
            {
                expectedSize = T::length();
            }
            else {
                expectedSize = T::col_type::length() * T::row_type::length();
            }

            if (vec.size() != expectedSize) {
                throw ValueError(
                    std::string(key),
                    fmt::format(
                        "Contained wrong number of values. Expected {} got {}",
                        expectedSize, vec.size()
                    )
                );
            }

            T res;
            std::memcpy(glm::value_ptr(res), vec.data(), sizeof(T));
            return res;
        }
        else {
            static_assert(false, "Unsupported type");
        }
    }
}

template <typename T, std::enable_if_t<Dictionary::IsAllowed<T>{}, int>>
bool Dictionary::hasValue(std::string_view key) const {
    const size_t dotPos = key.find('.');
    if (dotPos != std::string_view::npos) {
        std::string_view before = key.substr(0, dotPos);
        std::string_view after = key.substr(dotPos + 1);

        ghoul::Dictionary d = value<ghoul::Dictionary>(before);
        return d.hasValue<T>(after);
    }
    else {
        auto it = _storage.find(key);
        if (it == _storage.end()) {
            return false;
        }
        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int> ||
            std::is_same_v<T, double> || std::is_same_v<T, std::string> ||
            std::is_same_v<T, Dictionary> ||
            std::is_same_v<T, std::vector<int>> ||
            std::is_same_v<T, std::vector<double>>)
        {
            return std::holds_alternative<T>(it->second);
        }
        else if constexpr (std::is_same_v<T, glm::ivec2> ||
            std::is_same_v<T, glm::ivec3> ||
            std::is_same_v<T, glm::ivec4> ||
            std::is_same_v<T, glm::dvec2> ||
            std::is_same_v<T, glm::dvec3> ||
            std::is_same_v<T, glm::dvec4> ||
            std::is_same_v<T, glm::dmat2x2> ||
            std::is_same_v<T, glm::dmat2x3> ||
            std::is_same_v<T, glm::dmat2x4> ||
            std::is_same_v<T, glm::dmat3x2> ||
            std::is_same_v<T, glm::dmat3x3> ||
            std::is_same_v<T, glm::dmat3x4> ||
            std::is_same_v<T, glm::dmat4x2> ||
            std::is_same_v<T, glm::dmat4x3> ||
            std::is_same_v<T, glm::dmat4x4>)
        {
            if (std::holds_alternative<ghoul::Dictionary>(it->second)) {
                ghoul::Dictionary d = std::get<ghoul::Dictionary>(it->second);
                for (const auto& kv : d._storage) {
                    if (!std::holds_alternative<T::value_type>(kv.second)) {
                        return false;
                    }
                }
                // We should check whether the keys are sorted, too
                return true;
            }
            else {
                using VT = std::vector<T::value_type>;
                return std::holds_alternative<VT>(it->second) &&
                    std::get<VT>(it->second).size() == T::length();
            }
        }
        else {
            static_assert(false, "Unknown type T");
        }
    }
}

// This function should go away
std::vector<std::string_view> Dictionary::keys() const {
    std::vector<std::string_view> keys;
    keys.reserve(_storage.size());
    for (const auto& kv : _storage) {
        keys.push_back(kv.first);
    }
    return keys;
}

template void Dictionary::setValue(std::string, Dictionary value);
template void Dictionary::setValue(std::string, bool value);
template void Dictionary::setValue(std::string, double);
template void Dictionary::setValue(std::string, int);
template void Dictionary::setValue(std::string, std::string);
template void Dictionary::setValue(std::string, std::vector<int>);
template void Dictionary::setValue(std::string, std::vector<double>);
template void Dictionary::setValue(std::string, glm::ivec2);
template void Dictionary::setValue(std::string, glm::ivec3);
template void Dictionary::setValue(std::string, glm::ivec4);
template void Dictionary::setValue(std::string, glm::dvec2);
template void Dictionary::setValue(std::string, glm::dvec3);
template void Dictionary::setValue(std::string, glm::dvec4);
template void Dictionary::setValue(std::string, glm::dmat2x2);
template void Dictionary::setValue(std::string, glm::dmat2x3);
template void Dictionary::setValue(std::string, glm::dmat2x4);
template void Dictionary::setValue(std::string, glm::dmat3x2);
template void Dictionary::setValue(std::string, glm::dmat3x3);
template void Dictionary::setValue(std::string, glm::dmat3x4);
template void Dictionary::setValue(std::string, glm::dmat4x2);
template void Dictionary::setValue(std::string, glm::dmat4x3);
template void Dictionary::setValue(std::string, glm::dmat4x4);


template Dictionary Dictionary::value(std::string_view) const;
template bool Dictionary::value(std::string_view) const;
template double Dictionary::value(std::string_view) const;
template int Dictionary::value(std::string_view) const;
template std::string Dictionary::value(std::string_view) const;
template std::vector<int> Dictionary::value(std::string_view) const;
template std::vector<double> Dictionary::value(std::string_view) const;
template glm::ivec2 Dictionary::value(std::string_view) const;
template glm::ivec3 Dictionary::value(std::string_view) const;
template glm::ivec4 Dictionary::value(std::string_view) const;
template glm::dvec2 Dictionary::value(std::string_view) const;
template glm::dvec3 Dictionary::value(std::string_view) const;
template glm::dvec4 Dictionary::value(std::string_view) const;
template glm::dmat2x2 Dictionary::value(std::string_view) const;
template glm::dmat2x3 Dictionary::value(std::string_view) const;
template glm::dmat2x4 Dictionary::value(std::string_view) const;
template glm::dmat3x2 Dictionary::value(std::string_view) const;
template glm::dmat3x3 Dictionary::value(std::string_view) const;
template glm::dmat3x4 Dictionary::value(std::string_view) const;
template glm::dmat4x2 Dictionary::value(std::string_view) const;
template glm::dmat4x3 Dictionary::value(std::string_view) const;
template glm::dmat4x4 Dictionary::value(std::string_view) const;

template bool Dictionary::hasValue<Dictionary>(std::string_view) const;
template bool Dictionary::hasValue<bool>(std::string_view) const;
template bool Dictionary::hasValue<double>(std::string_view) const;
template bool Dictionary::hasValue<int>(std::string_view) const;
template bool Dictionary::hasValue<std::string>(std::string_view) const;
template bool Dictionary::hasValue<std::vector<int>>(std::string_view) const;
template bool Dictionary::hasValue<std::vector<double>>(std::string_view) const;
template bool Dictionary::hasValue<glm::ivec2>(std::string_view) const;
template bool Dictionary::hasValue<glm::ivec3>(std::string_view) const;
template bool Dictionary::hasValue<glm::ivec4>(std::string_view) const;
template bool Dictionary::hasValue<glm::dvec2>(std::string_view) const;
template bool Dictionary::hasValue<glm::dvec3>(std::string_view) const;
template bool Dictionary::hasValue<glm::dvec4>(std::string_view) const;
template bool Dictionary::hasValue<glm::dmat2x2>(std::string_view) const;
template bool Dictionary::hasValue<glm::dmat2x3>(std::string_view) const;
template bool Dictionary::hasValue<glm::dmat2x4>(std::string_view) const;
template bool Dictionary::hasValue<glm::dmat3x2>(std::string_view) const;
template bool Dictionary::hasValue<glm::dmat3x3>(std::string_view) const;
template bool Dictionary::hasValue<glm::dmat3x4>(std::string_view) const;
template bool Dictionary::hasValue<glm::dmat4x2>(std::string_view) const;
template bool Dictionary::hasValue<glm::dmat4x3>(std::string_view) const;
template bool Dictionary::hasValue<glm::dmat4x4>(std::string_view) const;

bool Dictionary::hasKey(std::string_view key) const {
    const size_t dotPos = key.find('.');
    if (dotPos != std::string_view::npos) {
        std::string_view before = key.substr(0, dotPos);
        std::string_view after = key.substr(dotPos + 1);

        if (hasKey(before)) {
            ghoul::Dictionary d = value<ghoul::Dictionary>(before);
            return d.hasKey(after);
        }
        else {
            return false;
        }
    }
    else {
        auto it = _storage.find(key);
        return it != _storage.end();
    }
}

bool Dictionary::isEmpty() const {
    return _storage.empty();
}

size_t Dictionary::size() const {
    return _storage.size();
}

} // namespace ghoul
