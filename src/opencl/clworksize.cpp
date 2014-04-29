/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
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

#include <ghoul/opencl/clworksize.h>

#include <ghoul/logging/logmanager.h>

#include <cassert>

namespace {
    std::string _loggerCat = "CLWorkSize";
}


namespace ghoul {
namespace opencl {

CLWorkSize::CLWorkSize(std::initializer_list<size_t> global, std::initializer_list<size_t> local): _dimensions(0), _global(0), _local(0) {
    assert(global.size() == local.size());
    _dimensions = global.size();
    assert(_dimensions >= 1 && _dimensions <= 3);
    
    _global = new size_t[_dimensions];
    _local = new size_t[_dimensions];
    
    for (unsigned int i = 0; i < _dimensions; ++i) {
        _global[i] = *(global.begin() + i);
        _local[i] = *(local.begin() + i);
    }
}

CLWorkSize::~CLWorkSize() {
    delete _global;
    delete _local;
}

unsigned int CLWorkSize::dimensions() const {
    return _dimensions;
}
size_t* CLWorkSize::globalOffset() const {
    return NULL;
}
size_t* CLWorkSize::global() const {
    return _global;
}
size_t* CLWorkSize::local() const {
    return _local;
}

}
}