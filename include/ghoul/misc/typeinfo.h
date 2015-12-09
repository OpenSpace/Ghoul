/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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

#ifndef __TYPEINFO_H__
#define __TYPEINFO_H__

#include <typeinfo>
#include <string>
#include <vector>

namespace ghoul {

/**
 * This class is a wrapper class for returning human readable names
 * for the most commonly used types in Ghoul.
 */
class TypeInfo {
public:
    /**
     * Returns the name of the provided type. If the class is not registered, the
     * <code>typeid(T).name</code> will be used. This will make the names look much worse
     * and const types will be treated as the same as non-const types.
     * \return The name of the provided type
     */
    template<class T>
    static std::string name();
    
    /**
     * Returns the name of the type of the provided object. If the class is not
     * registered, the <code>typeid(T).name</code> will be used. This will make the names
     * look much worse and const types will be treated as the same as non-const types.
     * \param obj The object of the type name requested
     * \return The name of the type of the provided object
     */
    template<class T>
    static std::string name(const T& obj);
    
};

}  // ghoul

#endif
