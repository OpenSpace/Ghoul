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

#include <ghoul/misc/buffer.h>
#include <ghoul/logging/logmanager.h>

#include <lz4/lz4.h>

#include <algorithm>
#include <iostream>
#include <fstream>

namespace {
    const std::string _loggerCat = "Buffer";
}

namespace ghoul {

Buffer::Buffer()
    : _offsetWrite(0)
    , _offsetRead(0)
{}

Buffer::Buffer(size_t capacity)
    : _data(capacity)
    , _offsetWrite(0)
    , _offsetRead(0)
{}

Buffer::Buffer(const std::string& filename)
    : _offsetWrite(0)
    , _offsetRead(0)
{
    read(filename);
}

Buffer::Buffer(const Buffer& other) {
    // copy memory
    _data = other._data;
    _offsetWrite = other._offsetWrite;
    _offsetRead = other._offsetRead;
}
Buffer::Buffer(Buffer&& other) {
    if(this != &other) {
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
    if(this != &rhs) {
        // copy memory
        _data = rhs._data;
        _offsetWrite = rhs._offsetWrite;
        _offsetRead = rhs._offsetRead;
    }
    return *this;
}
Buffer& Buffer::operator=(Buffer&& rhs) {
    if(this != &rhs) {
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

bool Buffer::write(const std::string& filename, bool compress) {
    //BinaryFile file(filename, BinaryFile::OpenMode::Out);
    std::ofstream file(filename, std::ios::binary | std::ios::out);
    if (!file)
        return false;
    
    file.write(reinterpret_cast<const char*>(&compress), sizeof(bool));
    if(compress) {
        value_type* data = new value_type[size()];
        int compressed_size = LZ4_compress(reinterpret_cast<const char*>(_data.data()),
                                           reinterpret_cast<char*>(data),
                                           _offsetWrite);
        if (compressed_size <= 0) {
            delete[] data;
            return false;
        }
        size_t size = compressed_size;
        // orginal size
        file.write(reinterpret_cast<const char*>(&_offsetWrite), sizeof(size_t));
        
        // compressed size
        file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
        file.write(reinterpret_cast<const char*>(data), size); // compressed data
        delete[] data;
    } else {
        file.write(reinterpret_cast<const char*>(&_offsetWrite), sizeof(size_t));
        file.write(reinterpret_cast<const char*>(_data.data()), _offsetWrite);
    }
    return true;
}

bool Buffer::read(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    //BinaryFile file(filename, BinaryFile::OpenMode::In);
    if (!file)
        return false;
    
    _offsetRead = 0;
    size_t size;
    bool compressed;
    file.read(reinterpret_cast<char*>(&compressed), sizeof(bool));
    if(compressed) {
    
        // read original size
        file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
        _data.resize(size);
        
        // read compressed size
        file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
        
        // allocate and read data
        value_type* data = new value_type[size];
        file.read(reinterpret_cast<char*>(data),size);
        
        // decompress
        _offsetWrite = LZ4_decompress_safe(reinterpret_cast<const char*>(data),
                                           reinterpret_cast<char*>(_data.data()),
                                           size,
                                           _data.size());
        delete[] data;
    } else {
        file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
        _data.resize(size);
        file.read(reinterpret_cast<char*>(_data.data()), size);
        _offsetWrite = size;
    }
    return true;
}

void Buffer::serialize(const char* s) {
    serialize(std::string(s));
}

void Buffer::serialize(const value_type* data, size_t size) {
    _data.resize(_data.capacity() + size);
    std::memcpy(_data.data() + _offsetWrite, &data, size);
    _offsetWrite += size;
}

void Buffer::deserialize(value_type* data, size_t size) {
    std::memcpy(data, &_data + _offsetRead, size);
    _offsetRead += size;
}

template<>
void Buffer::serialize(const std::string& v) {
    const size_t length = v.length();
    const size_t size = length + sizeof(size_t);
    _data.resize(_data.capacity() + size);
    std::memcpy(_data.data() + _offsetWrite, &length, sizeof(size_t));
    _offsetWrite += sizeof(size_t);
    std::memcpy(_data.data() + _offsetWrite, v.c_str(), length);
    _offsetWrite += length;
}

template<>
void Buffer::deserialize(std::string& v) {
    assert(_offsetRead + sizeof(size_t) <= _data.size());
    
    size_t size;
    std::memcpy(&size, _data.data() + _offsetRead, sizeof(size_t));
    _offsetRead += sizeof(size_t);
    
    assert(_offsetRead + size <= _data.size());
    
    v = std::string(reinterpret_cast<char*>(_data.data()+_offsetRead), size);
    _offsetRead += size;
}

template<>
void Buffer::serialize(const std::vector<std::string>& v) {
    const size_t length = v.size();
    size_t size = sizeof(size_t);
    _data.resize(_data.capacity() + size + length);
    
    std::memcpy(_data.data() + _offsetWrite, &length, sizeof(size_t));
    _offsetWrite += sizeof(size_t);
    for(auto e: v) {
        serialize(e);
    }
}

template<>
void Buffer::deserialize(std::vector<std::string>& v) {
    assert(_offsetRead + sizeof(size_t) <= _data.size());
    size_t n;
    std::memcpy(&n, _data.data() + _offsetRead, sizeof(size_t));
    _offsetRead += sizeof(size_t);
    
    v.reserve(n);
    for(size_t i = 0; i < n; ++i) {
        std::string t;
        deserialize(t);
        v.emplace_back(t);
    }
}



} // namespace owl