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
 *****************************************************************************************
 * The original of this file is found in the owl library maintained by Jonas Strandstedt *
 ****************************************************************************************/

#include <ghoul/misc/buffer.h>

#include <ghoul/logging/logmanager.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <lz4/lz4.h>

namespace ghoul {

Buffer::Buffer(size_t capacity)
    : _data(capacity)
{}

Buffer::Buffer(const std::string& filename) {
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    read(filename);
}

Buffer::Buffer(Buffer&& other) {
    if (this != &other) {
        // move memory
        _data = std::move(other._data);
        _offsetWrite = other._offsetWrite;
        _offsetRead = other._offsetRead;

        // invalidate rhs memory
        other._offsetWrite = 0;
        other._offsetRead = 0;
    }
}

Buffer& Buffer::operator=(const Buffer& rhs) {
    if (this != &rhs) {
        // copy memory
        _data = rhs._data;
        _offsetWrite = rhs._offsetWrite;
        _offsetRead = rhs._offsetRead;
    }
    return *this;
}

Buffer& Buffer::operator=(Buffer&& rhs) {
    if (this != &rhs) {
        // move memory
        _data = std::move(rhs._data);
        _offsetWrite = rhs._offsetWrite;
        _offsetRead = rhs._offsetRead;

        // invalidate rhs memory
        rhs._offsetWrite = 0;
        rhs._offsetRead = 0;
    }
    return *this;
}

void Buffer::reset() {
    _offsetWrite = 0;
    _offsetRead = 0;
}

const Buffer::value_type* Buffer::data() const {
    return _data.data();
}

Buffer::value_type* Buffer::data() {
    return _data.data();
}

Buffer::size_type Buffer::capacity() const {
    return _data.capacity();
}

Buffer::size_type Buffer::size() const {
    return _offsetWrite;
}

void Buffer::write(const std::string& filename, Compress compress) {
    std::ofstream file;
    file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    file.open(filename, std::ios::binary | std::ios::out);

    bool c = compress == Compress::Yes;
    file.write(reinterpret_cast<const char*>(&c), sizeof(bool));
    if (compress == Compress::Yes) {
        std::vector<value_type> buffer(size());
        int compressed_size = LZ4_compress(
            reinterpret_cast<const char*>(_data.data()),
            reinterpret_cast<char*>(buffer.data()),
            static_cast<int>(_offsetWrite)
        );
        if (compressed_size <= 0) {
            throw RuntimeError("Error compressing Buffer using LZ4", "Buffer");
        }

        std::streamsize size(compressed_size);
        // orginal size
        file.write(reinterpret_cast<const char*>(&_offsetWrite), sizeof(size_t));

        // compressed size
        file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
        file.write(reinterpret_cast<const char*>(buffer.data()), size); // compressed data
    } else {
        file.write(reinterpret_cast<const char*>(&_offsetWrite), sizeof(size_t));
        file.write(
            reinterpret_cast<const char*>(_data.data()),
            static_cast<std::streamsize>(_offsetWrite)
        );
    }
}

void Buffer::read(const std::string& filename) {
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    std::ifstream file;
    file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    file.open(filename, std::ios::binary | std::ios::in);

    _offsetRead = 0;
    bool compressed;
    file.read(reinterpret_cast<char*>(&compressed), sizeof(bool));
    if (compressed) {
        // read original size
        size_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
        _data.resize(size);

        // read compressed size
        file.read(reinterpret_cast<char*>(&size), sizeof(size_t));

        // allocate and read data
        std::vector<value_type> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()),size);

        // decompress
        _offsetWrite = LZ4_decompress_safe(
            reinterpret_cast<const char*>(buffer.data()),
            reinterpret_cast<char*>(_data.data()),
            static_cast<int>(size),
            static_cast<int>(_data.size())
        );
    } else {
        size_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
        _data.resize(size);
        file.read(
            reinterpret_cast<char*>(_data.data()),
            size
        );
        _offsetWrite = size;
    }
}

void Buffer::serialize(const char* s) {
    ghoul_assert(s, "s must not be nullptr");
    serialize(std::string(s));
}

void Buffer::serialize(const value_type* data, size_t size) {
    ghoul_assert(data, "Data must not be nullptr");

    _data.resize(_data.capacity() + size);
    std::memcpy(_data.data() + _offsetWrite, &data, size);
    _offsetWrite += size;
}

void Buffer::deserialize(value_type* data, size_t size) {
    ghoul_assert(data, "Data must not be nullptr");

    std::memcpy(data, &_data + _offsetRead, size);
    _offsetRead += size;
}

template <>
void Buffer::serialize(const std::string& v) {
    size_t length = v.length();
    size_t size = length + sizeof(size_t);
    _data.resize(_data.capacity() + size);

    std::memcpy(_data.data() + _offsetWrite, &length, sizeof(size_t));
    _offsetWrite += sizeof(size_t);

    std::memcpy(_data.data() + _offsetWrite, v.c_str(), length);
    _offsetWrite += length;
}

template <>
void Buffer::deserialize(std::string& value) {
    ghoul_assert(
        _offsetRead + sizeof(size_t) <= _data.size(), "Insufficient buffer size"
    );

    size_t size;
    std::memcpy(&size, _data.data() + _offsetRead, sizeof(size_t));
    _offsetRead += sizeof(size_t);

    ghoul_assert(_offsetRead + size <= _data.size(), "Insufficient buffer size");

    value = std::string(reinterpret_cast<char*>(_data.data()+_offsetRead), size);
    _offsetRead += size;
}

template <>
void Buffer::serialize(const std::vector<std::string>& v) {
    size_t length = v.size();
    size_t size = sizeof(size_t);
    _data.resize(_data.capacity() + size + length);

    std::memcpy(_data.data() + _offsetWrite, &length, sizeof(size_t));
    _offsetWrite += sizeof(size_t);
    for (const auto& e : v) {
        serialize(e);
    }
}

template <>
void Buffer::deserialize(std::vector<std::string>& v) {
    ghoul_assert(
        _offsetRead + sizeof(size_t) <= _data.size(), "Insufficient buffer size"
    );
    size_t n;
    std::memcpy(&n, _data.data() + _offsetRead, sizeof(size_t));
    _offsetRead += sizeof(size_t);

    v.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        std::string t;
        deserialize(t);
        v.emplace_back(t);
    }
}

} // namespace ghoul
