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

#include <cassert>
#include <string>
#include <vector>
#include <type_traits>
#include <initializer_list>

namespace ghoul {

/**
 * This class is a buffer container for serialized objects. The serialize
 * functions copies the memory of the provided object to the end of the 
 * internal array. The deserialize functions copies the memory from the 
 * end of the array into the provided object. Serialize and deserialize 
 * functions can be used interleaved since there are one read and one 
 * write pointer. The write and read functions write and read the internal
 * array to a binary file, LZ4 compression is supported.
 */
class Buffer {
public:
    typedef unsigned char value_type;
    typedef std::vector<value_type>::size_type size_type;

    /**
     * Default Buffer object constructor. The size of the internal 
     * array is 0.
     */
    Buffer();
    
    /**
     * Constructor with requested initial capacity of the internal array.
     * \param capacity The initial capacity for the internal array
     */
    Buffer(size_t capacity);
    
    /**
     * Constructs a Buffer object from file.
     * \param filename The filename of the binary Buffer file.
     */
    Buffer(const std::string& filename);
    
    /**
     * Constructs a Buffer by copying another Buffer object.
     */
    Buffer(const Buffer& other);
    
    /**
     * Moves the original object into this one. The original 
     * object must not be used after the move since it is in 
     * an undefined state.
     */
    Buffer(Buffer&& other);
    
    /**
     * Constructs a Buffer by copying another Buffer object.
     */
    Buffer& operator=(const Buffer& rhs);
    
    /**
     * Moves the original object into this one. The original
     * object must not be used after the move since it is in
     * an undefined state.
     */
    Buffer& operator=(Buffer&& rhs);
    
    /**
     * Default destructor is sufficient since no objects are 
     * allocated internally
     */
    ~Buffer() = default;
    
    /**
     * Sets the read and write offsets to 0
     */
    void reset();
    
    /**
     * Pointer to the const raw data pointer
     *  \return Const pointer to the raw data
     */
    const value_type* data() const;
    
    /**
     * Pointer to the raw data pointer
     * \return Pointer to the raw data
     */
    value_type* data();
    
    /**
     * Returns the capacity of the internal array
     * \return The current capacity of the internal array
     */
    size_type capacity() const;
    
    /**
     * Returns the size of the internal array. This is the same as
     * the amount of bytes currently serialized.
     * \return The current size of the internal array
     */
    size_type size() const;
    
    /**
     * Writes the current Buffer to a file. This file will be bigger than 
     * the current Buffer size because it also writes metadata to the file.
     * \param filename The filename to be written to
     * \param compress Flag that specifies if the current Buffer should
     * be compressed when written to file
     * \return <code>true</code> if successfull and <code>false</vode> if unsuccessfull
     */
    bool write(const std::string& filename, bool compress = false);
    
    /**
     * Reads the Buffer from a Buffer file. 
     * \param filename The path to the file to read
     * \return <code>true</code> if successfull and <code>false</vode> if unsuccessfull
     */
    bool read(const std::string& filename);
    
    /**
     * Serializes a const char* string to a std::string
     * \param s The string to be serialized
     */
    void serialize(const char* s);
    
    /**
     * Serializes raw data
     * \param data Pointer to the raw data to serialize
     * \param size The size of the data to serialize in bytes
     */
    void serialize(const value_type* data, size_t size);
    
    /**
     * Seralizes an object
     */
    template<class T> void serialize(const T& v);
    
    /**
     * Serializes a vector of objects
     */
    template<class T, template <typename, typename = std::allocator<T>> class C> void serialize(const C<T>& v);
    
    /**
     * Deserialize raw data
     * \param data Pointer to a datablock to copy data into
     * \param size The size number of bytes of data to copy
     */
    void deserialize(value_type* data, size_t size);
    
    /**
     * Deserializes an object
     */
    template<class T> void deserialize(T& value);
    
    /**
     * Deserializes a vector of objects
     */
    template<class T, template <typename, typename = std::allocator<T>> class C>  void deserialize(C<T>& v);
    
private:

    std::vector<value_type> _data;
    size_t _offsetWrite;
    size_t _offsetRead;
    
    
}; // class Buffer

// Specializations for std::string
template<>
void Buffer::serialize(const std::string& v);
template<>
void Buffer::deserialize(std::string& v);
template<>
void Buffer::serialize(const std::vector<std::string>& v);
template<>
void Buffer::deserialize(std::vector<std::string>& v);

} // namespace ghoul

#include "buffer.inl"

#endif // __BUFFER_H__