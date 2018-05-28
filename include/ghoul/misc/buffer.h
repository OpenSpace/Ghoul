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

#ifndef __GHOUL___BUFFER___H__
#define __GHOUL___BUFFER___H__

#include <ghoul/misc/boolean.h>
#include <ghoul/misc/exception.h>
#include <string>
#include <vector>

namespace ghoul {

/**
 * This class is a buffer container for serialized objects. The serialize functions copy
 * the memory of the provided object to the end of the internal array. The deserialize
 * functions copies the memory from the end of the array into the provided object.
 * Serialize and deserialize functions can be used interleaved since there are one read
 * and one write pointer. The write and read functions write and read the internal
 * array to a binary file; LZ4 compression is supported.
 */
class Buffer {
public:
    BooleanType(Compress);

    using value_type = unsigned char;
    using size_type = std::vector<value_type>::size_type;

    /**
     * Default Buffer object constructor. The size of the internal array is 0.
     */
    Buffer() = default;

    /**
     * Constructor with requested initial capacity of the internal array.
     *
     * \param capacity The initial capacity for the internal array
     */
    Buffer(size_t capacity);

    /**
     * Constructs a Buffer object from file.
     *
     * \param filename The filename of the binary Buffer file.
     *
     * \throw std::ios_base::failure If the buffer could not be read from \p filename
     * \pre \p filename must not be empty
     */
    Buffer(const std::string& filename);

    /**
     * Constructs a Buffer by copying another Buffer object.
     *
     * \param other The Buffer to copy the data from
     */
    Buffer(const Buffer& other) = default;

    /**
     * Moves the original object into this one. The original object must not be used after
     * the move since it is in an undefined state.
     *
     * \param other The Buffer from which the data is moved out of
     */
    Buffer(Buffer&& other);

    /**
     * Constructs a Buffer by copying another Buffer object.
     *
     * \param rhs The Buffer from which the data is copied
     * \return The object this operator was called on
     */
    Buffer& operator=(const Buffer& rhs);

    /**
     * Moves the original object into this one. The original object must not be used after
     * the move since it is in an undefined state.
     *
     * \param rhs The Buffer out of which the data is moved
     * \return The object this operator was called on
     */
    Buffer& operator=(Buffer&& rhs);

    /**
     * Default destructor is sufficient since no objects are allocated internally.
     */
    ~Buffer() = default;

    /**
     * Sets the read and write offsets to 0.
     */
    void reset();

    /**
     * Pointer to the const raw data pointer.
     *
     * \return Const pointer to the raw data
     */
    const value_type* data() const;

    /**
     * Pointer to the raw data pointer.
     *
     * \return Pointer to the raw data
     */
    value_type* data();

    /**
     * Returns the capacity of the internal array.
     *
     * \return The current capacity of the internal array
     */
    size_type capacity() const;

    /**
     * Returns the size of the internal array. This is the same as the amount of bytes
     * currently serialized.
     *
     * \return The current size of the internal array
     */
    size_type size() const;

    /**
     * Writes the current Buffer to a file. This file will be bigger than the current
     * Buffer size because it also writes metadata to the file.
     *
     * \param filename The filename to be written to
     * \param compress Flag that specifies if the current Buffer should be compressed when
     *        written to file
     *
     * \throw std::ios_base::failure If there was an error writing the file
     * \throw RuntimeError if there was an error compressing the data
     * \pre \p filename must not be empty
     */
    void write(const std::string& filename, Compress compress = Compress::No);

    /**
     * Reads the Buffer from a Buffer file.
     *
     * \param filename The path to the file to read
     *
     * \throw std::ios_base::failure If there was an error reading the file
     * \pre \p filename must not be empty
     */
    void read(const std::string& filename);

    /**
     * Serializes a const char* string to a std::string.
     *
     * \param s The string to be serialized
     *
     * \pre \p s must not be <code>nullptr</code>
     */
    void serialize(const char* s);

    /**
     * Serializes raw data.
     *
     * \param data Pointer to the raw data to serialize
     * \param size The size of the data to serialize in bytes
     *
     * \pre \p data must not be <code>nullptr</code>
     */
    void serialize(const value_type* data, size_t size);

    /**
     * Seralizes a general object.
     *
     * \tparam T The type of the object
     * \param v The object to be serialized
     *
     * \pre \p T must be a POD type
     */
    template <class T>
    void serialize(const T& v);

    /**
     * Serializes a vector of general objects.
     *
     * \tparam T The type of each object
     * \param v The vector of objects to serialize
     *
     * \pre \p T must be a POD type
     */
    template <typename T>
    void serialize(const std::vector<T>& v);

    /**
     * Serializes the elements [begin, end) to the Buffer.
     *
     * \tparam Iter Forward-iterator
     * \param begin Inclusive iterator to the front of the set of serialized elements
     * \param end Exclusive iterator to the end of the set of serialized elements
     *
     * \pre The type pointed to by \p Iter must be a POD
     */
    template <typename Iter>
    void serialize(Iter begin, Iter end);

    /**
     * Deserialize raw data.
     *
     * \param data Pointer to a datablock to copy data into
     * \param size The size number of bytes of data to copy
     *
     * \pre \p data must not be <code>nullptr</code>
     */
    void deserialize(value_type* data, size_t size);

    /**
     * Deserializes a general object.
     *
     * \tparam T The type of the object to deserialize
     * \param value The object to deserialize
     *
     * \pre \p T must be a POD type
     */
    template <class T>
    void deserialize(T& value);

    /**
     * Deserializes a vector of general objects.
     *
     * \tparam T The type of each object
     * \param v The vector of objects to deserialize
     *
     * \pre \p T must be a POD type
     */
    template <typename T>
    void deserialize(std::vector<T>& v);

    /**
     * Deserializes the Buffer into the elements [begin, end).
     *
     * \tparam Iter Forward-iterator
     * \param begin Inclusive iterator to the front of the set of deserialized elements
     * \param end Exclusive iterator to the end of the set of deserialized elements
     *
     * \pre The type pointed to by \p Iter must be a POD
     * \pre The number of elements deserialized must be equal to the distance between
     *      \p begin and \p end
     */
    template <typename Iter>
    void deserialize(Iter begin, Iter end);

private:
    /// The buffer storage
    std::vector<value_type> _data;

    /// Pointer to the current writing position
    size_t _offsetWrite = 0;

    /// Pointer to the current reading position
    size_t _offsetRead = 0;
};

// Specializations for std::string
template <>
void Buffer::serialize(const std::string& v);

template <>
void Buffer::deserialize(std::string& v);

template <>
void Buffer::serialize(const std::vector<std::string>& v);

template <>
void Buffer::deserialize(std::vector<std::string>& v);

} // namespace ghoul

#include "buffer.inl"

#endif // __GHOUL___BUFFER___H__
