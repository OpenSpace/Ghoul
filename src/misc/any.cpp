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

#include <ghoul/misc/any.h>

namespace ghoul {

any::any(const any& other)
    : content(other.content ? other.content->clone() : nullptr)
{}

any::any(any&& other) noexcept
    : content(std::move(other.content))
{
    other.content = nullptr;
}

any& any::swap(any& rhs) noexcept {
    std::swap(content, rhs.content);
    return *this;
}

any& any::operator=(const any& rhs) {
    any(rhs).swap(*this);
    return *this;
}

any& any::operator=(any&& rhs) noexcept {
    rhs.swap(*this);
    any().swap(rhs);
    return *this;
}

bool any::empty() const noexcept {
    return !content;
}

void any::clear() noexcept {
    any().swap(*this);
}

const std::type_info& any::type() const noexcept {
    return content ? content->type() : typeid(void);
}

inline void swap(any& lhs, any& rhs) noexcept {
    lhs.swap(rhs);
}

const char* bad_any_cast::what() const noexcept {
    return "ghoul::bad_any_cast: "
        "failed conversion using ghoul::any_cast";
}


} // namespace ghoul
