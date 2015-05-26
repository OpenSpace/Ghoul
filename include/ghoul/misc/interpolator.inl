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
 ****************************************************************************************/

#include <array>

namespace ghoul{

template<typename U, typename... UArgs>
U ghoul::Interpolator<ghoul::Interpolators::Linear>::interpolate(double fact, U p0, U p1, UArgs && ... arguments){
        #ifdef GHL_DEBUG
        //no error to report here, any number of extra arguments may be given but we know that at least
        //the two required are there.
        //static_assert(sizeof...(arguments) == 0, "Interpolation requires at least two values");
        #else
        #warning "Linear only wants 2, duh"
        #endif
        std::array<U, sizeof...(arguments)> list = {arguments...};
        return fact * p1 + (1.0 - fact) * p0;
    }

template<typename U, typename... UArgs>
U ghoul::Interpolator<ghoul::Interpolators::CatmullRom>::interpolate(double fact, U p0, U p1, UArgs && ... arguments){
    #ifdef GHL_DEBUG
    static_assert(sizeof...(arguments) < 2, "Catmull-Rom interpolation requires at least four control points");
    #else
    #warning "Catmull-Rom interpolation requires at least four control points"
    #endif
    std::array<U, sizeof...(arguments)> list = {arguments...};
    
    //the additional two control points contained in the arguments variable
    //indexing by [] is safe up until index 1 since we have already checked that the size is at least two.
    U p2, p3;
    p2 = list[0];
    p3 = list[1];
    
    double t = fact;
    double t2 = t*t;
    double t3 = t2*t;
    
    return  0.5 * ( 2.0 * p1 + t * (p2 - p0) + t2 * (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) + t3 * (3.0 * p1 - p0  - 3.0 * p2 + p3) );
    }
}
