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

#ifndef __INTERPOLATOR_H__
#define __INTERPOLATOR_H__

#include <array>

namespace ghoul{
    /**
     * Enum listing the available template specialization of the Interpolator class.
     */
    enum Interpolators {
        Linear,
        CatmullRom,
        Undefined
    };

    /**
     * This is a templated class for Interpolation of arbitrary data types.
     * The particular interpolation method for any given data is defined by
     * which template specialization is instantiated, for available interpolators
     * see the <code>Interpolators</code> enum.
     */
    template <Interpolators T>
    class Interpolator{
    public:
        
        /**
         * Returns an interpolated value based on the factor supplied in <code>fact</code>.
         * In addition to the interpolation factor <code>fact</code> a minimum of two values
         * which to interpolate between need to be supplied, <code>p0</code> and <code>p1</code>.
         * \param fact The interpolation factor to use in interpolation
         * \param p0 The first control point of the interpolation
         * \param p1 The second control point of the interpolation
         * \param arguments An arbitrary number of additional control points to use in interpolation.
         * \return The interpolated value for the given control points based on
         * the interpolation factor <code>fact</code>
         */
        template<typename U, typename... UArgs>
        U interpolate(double fact, U p0, U p1, UArgs && ... arguments);
    };

    /**
     * This the linear interpolation template specialization of the Interpolator class.
     */
    template<>
    class Interpolator<Interpolators::Linear>{
    public:
        /**
         * Returns an linearly interpolated value between the control points <code>p0</code>
         * and <code>p1</code> based on the interpolation factor <code>fact</code>.
         * \param fact The interpolation factor to use in interpolation
         * \param p0 The first control point of the interpolation
         * \param p1 The second control point of the interpolation
         * \param arguments An arbitrary number of additional control points, all of which are
         * ignored in this interpolation method.
         * \return The linearly interpolated value for the given control points based on
         * the interpolation factor <code>fact</code>
         */
        template<typename U, typename... UArgs>
        U interpolate(double fact, U p0, U p1, UArgs && ... arguments);
    };
    
    /**
     * This the Catmull-Rom interpolation template specialization of the Interpolator class.
     */
    template<>
    class Interpolator<Interpolators::CatmullRom>{
    public:
        /**
         * Returns an interpolated value using the Catmull-Rom spline interpolation scheme.
         * Four conrol points are used in the interpolation; <code>p0</code>, <code>p1</code>
         * and two points extracted from <code>arguments</code> and interpolation is calculated
         * based on the interpolation factor <code>fact</code>.
         * \param fact The interpolation factor to use in interpolation
         * \param p0 The first control point of the interpolation
         * \param p1 The second control point of the interpolation
         * \param arguments Two additional control points used in the interpolation.
         * \return The interpolated value for the given control points calculated using Catmull-Rom 
         * spline interpolation based on the interpolation factor <code>fact</code>
         */
        template<typename U, typename... UArgs>
        U interpolate(double fact, U p0, U p1, UArgs && ... arguments);
    };
    
}

#include "interpolator.inl"

#endif
