/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#include <ghoul/misc/assert.h>
#include <cstring>

template<class T>
void ghoul::Buffer::serialize(const T& v) {
    static_assert(std::is_pod<T>::value, "T has to be a POD for general serialize");
    
    size_t size = sizeof(T);
    _data.resize(_data.capacity() + size);
    memcpy(_data.data() + _offsetWrite, &v, size);
    _offsetWrite += size;
}

template<class T>
void ghoul::Buffer::deserialize(T& value) {
    static_assert(std::is_pod<T>::value, "T has to be a POD for general deserialize");
    
    size_t size = sizeof(T);
    ghoul_assert(_offsetRead + size <= _data.size(), "Insufficient buffer size");
    
    memcpy(&value, _data.data() + _offsetRead, size);
    _offsetRead += size;
}

template<typename T>
void ghoul::Buffer::serialize(const std::vector<T>& v) {
    static_assert(std::is_pod<T>::value, "T has to be a POD for general serialize");
    
    size_t length = v.size();
    size_t size = sizeof(T)*length+sizeof(size_t);
    _data.resize(_data.capacity() + size);
    
    std::memcpy(_data.data() + _offsetWrite, &length, sizeof(size_t));
    _offsetWrite += sizeof(size_t);
    std::memcpy(_data.data() + _offsetWrite, v.data(), sizeof(T)*length);
    _offsetWrite += sizeof(T)*length;
}

template <typename Iter>
void ghoul::Buffer::serialize(Iter begin, Iter end) {
    using T = typename std::iterator_traits<Iter>::value_type;
    static_assert(
        std::is_pod<T>::value,
        "Iter must point to a POD for general serialize"
    );
    
    size_t length = std::distance(begin, end);
    size_t size = sizeof(T) * length + sizeof(size_t);
    _data.resize(_data.capacity() + size);
    
    std::memcpy(_data.data() + _offsetWrite, &length, sizeof(size_t));
    _offsetWrite += sizeof(size_t);
    while (begin != end) {
        serialize(*begin);
        begin = std::next(begin);
    }
}


template<typename T>
void ghoul::Buffer::deserialize(std::vector<T>& v) {
    static_assert(std::is_pod<T>::value, "T has to be a POD for general deserialize");
    
    ghoul_assert(
        _offsetRead + sizeof(size_t) <= _data.size(), "Insufficient buffer size"
    );
    size_t n;
    memcpy(&n, _data.data() + _offsetRead, sizeof(size_t));
    _offsetRead += sizeof(size_t);
    ghoul_assert(_offsetRead + sizeof(T)*n <= _data.size(), "Insufficient buffer size");
    v.resize(n);
    memcpy(v.data(),_data.data() + _offsetRead, sizeof(T)*n);
    _offsetRead += sizeof(T)*n;
}

template <typename Iter>
void ghoul::Buffer::deserialize(Iter begin, Iter end) {
    using T = typename std::iterator_traits<Iter>::value_type;
    static_assert(
        std::is_pod<T>::value,
        "Iter must point to a POD for general serialize"
    );
    
    size_t n;
    std::memcpy(&n, _data.data() + _offsetRead, sizeof(size_t));
    _offsetRead += sizeof(size_t);
    
    ghoul_assert(
        std::distance(begin, end) == n, "Requested size differs from stored size"
    );
    
    while (begin != end) {
        deserialize(*begin);
        begin = std::next(begin);
    }
}
