/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2021                                                               *
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

namespace {
    // List of types that are stored as-is in the Dictionary
    using DirectTypes = std::variant<bool, double, int, std::string, Dictionary,
        std::vector<int>, std::vector<double>>;
    template <typename T> using isDirectType = internal::is_one_of<T, DirectTypes>;

    // Vector types that are converted into std::vector for storage purposes
    using GLMTypes = std::variant<glm::ivec2, glm::ivec3, glm::ivec4, glm::dvec2,
    glm::dvec3, glm::dvec4, glm::dmat2x2, glm::dmat2x3, glm::dmat2x4, glm::dmat3x2,
        glm::dmat3x3, glm::dmat3x4, glm::dmat4x2, glm::dmat4x3, glm::dmat4x4>;
    template <typename T> using isGLMType = internal::is_one_of<T, GLMTypes>;
} // namespace

Dictionary::KeyError::KeyError(std::string msg)
    : RuntimeError(std::move(msg), "Dictionary")
{}

Dictionary::ValueError::ValueError(std::string k, std::string m)
    : RuntimeError(fmt::format("Key '{}': {}", std::move(k), std::move(m)), "Dictionary")
{}

bool Dictionary::operator==(const Dictionary& rhs) const noexcept {
    return _storage == rhs._storage;
}

bool Dictionary::operator!=(const Dictionary& rhs) const noexcept {
    return _storage != rhs._storage;
}

template <typename T, std::enable_if_t<Dictionary::IsAllowedType<T>{}, int>>
void Dictionary::setValue(std::string key, T value) {
    ghoul_assert(!key.empty(), "Key must not be empty");
    if constexpr (isDirectType<T>::value) {
        _storage.insert_or_assign(std::move(key), std::move(value));
    }
    else if constexpr (isGLMType<T>::value) {
        typename T::value_type* p = glm::value_ptr(value);
        std::vector<typename T::value_type> vec(p, p + ghoul::glm_components<T>::value);
        _storage.insert_or_assign(std::move(key), std::move(vec));
    }
    else {
        static_assert(sizeof(T) == 0, "Unsupported type");
    }
}

template <typename T, std::enable_if_t<Dictionary::IsAllowedType<T>{}, int>>
T Dictionary::value(std::string_view key) const {
    ghoul_assert(!key.empty(), "Key must not be empty");
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

        if constexpr (isDirectType<T>::value) {
            if (std::holds_alternative<T>(it->second)) {
                return std::get<T>(it->second);
            }
            else {
                throw ValueError(
                    std::string(key),
                    fmt::format(
                        "Error accessing value, wanted type '{}' has '{}'",
                        typeid(T).name(), it->second.index()
                    )
                );
            }
        }
        else if constexpr (isGLMType<T>::value) {
            using VT = std::vector<typename T::value_type>;
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
                    if (k < 0 || k >= static_cast<int>(d._storage.size())) {
                        throw ValueError(
                            std::string(key),
                            fmt::format(
                                "Invalid key {} outside range [0,{}]",
                                k, d._storage.size()
                            )
                        );
                    }
                    vec[k] = std::get<typename T::value_type>(kv.second);
                }
            }
            else {
                throw ValueError(
                    std::string(key),
                    fmt::format(
                        "Requested {} but did not contain {} or {}",
                        typeid(T).name(), typeid(T).name(),
                        typeid(std::vector<typename T::value_type>).name()
                    )
                );
            }

            if (vec.size() != ghoul::glm_components<T>::value) {
                throw ValueError(
                    std::string(key),
                    fmt::format(
                        "Contained wrong number of values. Expected {} got {}",
                        ghoul::glm_components<T>::value, vec.size()
                    )
                );
            }

            T res;
            std::memcpy(glm::value_ptr(res), vec.data(), sizeof(T));
            return res;
        }
        else {
            static_assert(sizeof(T) == 0, "Unsupported type");
        }
    }
}

template <typename T, std::enable_if_t<Dictionary::IsAllowedType<T>{}, int>>
bool Dictionary::hasValue(std::string_view key) const {
    ghoul_assert(!key.empty(), "Key must not be empty");
    const size_t dotPos = key.find('.');
    if (dotPos != std::string_view::npos) {
        std::string_view before = key.substr(0, dotPos);
        std::string_view after = key.substr(dotPos + 1);

        if (hasValue<ghoul::Dictionary>(before)) {
            ghoul::Dictionary d = value<ghoul::Dictionary>(before);
            return d.hasValue<T>(after);
        }
        else {
            return false;
        }
    }
    else {
        auto it = _storage.find(key);
        if (it == _storage.end()) {
            return false;
        }
        if constexpr (isDirectType<T>::value) {
            return std::holds_alternative<T>(it->second);
        }
        else if constexpr (isGLMType<T>::value) {
            if (std::holds_alternative<ghoul::Dictionary>(it->second)) {
                ghoul::Dictionary d = std::get<ghoul::Dictionary>(it->second);

                if (d.size() != ghoul::glm_components<T>::value) {
                    return false;
                }

                // Check whether we have all keys and they are of the correct type
                for (int i = 1; i <= ghoul::glm_components<T>::value; ++i) {
                    if (!d.hasValue<typename T::value_type>(std::to_string(i))) {
                        return false;
                    }
                }

                // We should check whether the keys are sorted, too
                return true;
            }
            else {
                using VT = std::vector<typename T::value_type>;
                return std::holds_alternative<VT>(it->second) &&
                    std::get<VT>(it->second).size() == ghoul::glm_components<T>::value;
            }
        }
        else {
            static_assert(sizeof(T) == 0, "Unknown type T");
        }
    }
}

bool Dictionary::hasKey(std::string_view key) const {
    ghoul_assert(!key.empty(), "Key must not be empty");
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

std::vector<std::string_view> Dictionary::keys() const {
    std::vector<std::string_view> keys;
    keys.reserve(_storage.size());
    for (const auto& kv : _storage) {
        keys.push_back(kv.first);
    }
    return keys;
}

void Dictionary::removeValue(std::string_view key) {
    if (const auto it = _storage.find(key);  it != _storage.end()) {
        _storage.erase(it);
    }
}

bool Dictionary::isEmpty() const {
    return _storage.empty();
}

size_t Dictionary::size() const {
    return _storage.size();
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

} // namespace ghoul
