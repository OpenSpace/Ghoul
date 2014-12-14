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

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>

#include <string>
#include <vector>
#include <bitset>
#include <type_traits>
#include <initializer_list>

namespace ghoul {

class Buffer {
public:
    typedef unsigned char value_type;
    typedef std::vector<value_type>::size_type size_type;

    /**
     * Default Buffer object
     */
    Buffer();
    Buffer(size_t capacity);
    Buffer(const std::string& filename, bool compressed = false);
    
    /**
     * Copy and move constructors
     */
    Buffer(const Buffer& other);
    Buffer(Buffer&& other);
    Buffer& operator=(const Buffer& rhs);
    Buffer& operator=(Buffer&& rhs);
    
    /**
     * Default destructor is sufficient since no objects are allocated internally
     */
    ~Buffer() = default;
    
    /**
     * Sets the read- and write offsets to 0
     */
    void reset();
    
    /**
     * Pointer to the const raw data pointer
     */
    const value_type* data() const;
    
    /**
     * Pointer to the raw data pointer
     */
    value_type* data();
    
    /**
     * Sets the read- and write offsets to 0
     */
    size_type capacity() const;
    size_type size() const;
    
    /**
     * Writes the current Buffer to a file.
     * \param filename The filename to be written to
     * \param compress Flag that specifies if the current Buffer should
     * be compressed when written to file
     * \return <code>true</code> if successfull and <code>false</vode>
     */
    bool write(const std::string& filename, bool compress = false);
    bool read(const std::string& filename, bool compressed = false);
    
    void serialize(const char* data);
    void serialize(const char* data, size_t size);
    template<class T> void serialize(const T& v);
    template<class T, template <typename, typename = std::allocator<T>> class C> void serialize(const C<T>& v);
    
    void deserialize(char* data, size_t size);
    template<class T> void deserialize(T& value);
    template<class T, template <typename, typename = std::allocator<T>> class C>  void deserialize(C<T>& v);
    
private:

    std::vector<value_type> _data;
    size_t _offsetWrite;
    size_t _offsetRead;
    
    
}; // class Buffer


template<>
void Buffer::serialize(const std::string& v);
template<>
void Buffer::deserialize(std::string& v);
template<>
void Buffer::serialize(const std::vector<std::string>& v);
template<>
void Buffer::deserialize(std::vector<std::string>& v);


template<class T>
void Buffer::serialize(const T& v) {
    static_assert(std::is_pod<T>::value, "T has to be a POD for general serialize");
    const size_t size = sizeof(T);
    _data.resize(_data.capacity() + size);
    std::memcpy(_data.data() + _offsetWrite, &v, size);
    _offsetWrite += size;
}

template<class T>
void Buffer::deserialize(T& value) {
    static_assert(std::is_pod<T>::value, "T has to be a POD for general deserialize");
    const size_t size = sizeof(T);
    assert(_offsetRead + size <= _data.size());
    
    std::memcpy(&value, _data.data() + _offsetRead, size);
    _offsetRead += size;
}

template<class T, template <typename, typename = std::allocator<T>> class C>
void Buffer::serialize(const C<T>& v) {
    static_assert(std::is_pod<T>::value, "T has to be a POD for general serialize");
    
    const size_t length = v.size();
    const size_t size = sizeof(T)*length+sizeof(size_t);
    _data.resize(_data.capacity() + size);
    
    std::memcpy(_data.data() + _offsetWrite, &length, sizeof(size_t));
    _offsetWrite += sizeof(size_t);
    std::memcpy(_data.data() + _offsetWrite, v.data(), sizeof(T)*length);
    _offsetWrite += sizeof(T)*length;
}


template<class T, template <typename, typename = std::allocator<T>> class C>
void Buffer::deserialize(C<T>& v) {
    static_assert(std::is_pod<T>::value, "T has to be a POD for general deserialize");
    
    assert(_offsetRead + sizeof(size_t) <= _data.size());
    size_t n;
    std::memcpy(&n, _data.data() + _offsetRead, sizeof(size_t));
    _offsetRead += sizeof(size_t);
    assert(_offsetRead + sizeof(T)*n <= _data.size());
    v.resize(n);
    std::memcpy(v.data(),_data.data() + _offsetRead, sizeof(T)*n);
    _offsetRead += sizeof(T)*n;
}

    
} // namespace ghoul

#endif // __BUFFER_H__