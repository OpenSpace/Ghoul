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
 ****************************************************************************************/

#ifndef __GHOUL___INTERPOLATOR___H__
#define __GHOUL___INTERPOLATOR___H__

namespace ghoul {

/**
 * Returns an interpolated value based on the factor supplied in \p t. In addition to the
 * interpolation factor \p t two values which to interpolate between must be supplied.
 * For <code>t = 0.0</code>, \p p0 is returned, for <code>t = 1.0</code>, \p p1 is
 * returned.
 * \param t The interpolation factor to use in interpolation
 * \param p0 The first control point of the interpolation
 * \param p1 The second control point of the interpolation
 * \return The interpolated value for the given control points based on \p t
 */
template <typename T>
T interpolateLinear(double t, const T& p0, const T& p1);

/**
 * Returns an interpolated value using the Catmull-Rom spline interpolation scheme. Four
 * control points are used in the interpolation; \p p0, \p p1, \p p2, and \p p3. The
 * interpolation is calculated based on the interpolation factor \p t.
 * \param t The interpolation factor to use in interpolation
 * \param p0 The first control point of the interpolation
 * \param p1 The second control point of the interpolation
 * \param p2 The third control point of the interpolation
 * \param p3 The fourth control point of the interpolation
 * \return The interpolated value for the given control points calculated using
 * Catmull-Rom spline interpolation based on the interpolation factor \p t
 */
template <typename T>
T interpolateCatmullRom(double t, const T& p0, const T& p1, const T& p2, const T& p3);

} // namespace ghoul

#include "interpolator.inl"

#endif // __GHOUL___INTERPOLATOR___H__
