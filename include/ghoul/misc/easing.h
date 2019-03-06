/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2019                                                               *
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
 * File based on 'easing.c' by Auerhaus Development, LLC and used under the Do What The  *
 * Fuck You Want To Public License, Version 2 as published by Sam Hocevar. See           *
 * http://sam.zoy.org/wtfpl/COPYING for more details.                                    *
 ****************************************************************************************/

#ifndef __GHOUL___EASING___H__
#define __GHOUL___EASING___H__

#include <string>

namespace ghoul {

enum class EasingFunction : uint8_t {
    // @FRAGILE(abock):  The cpp expects the values to be in this order and start at 0
    Linear = 0,
    QuadraticEaseIn, QuadraticEaseOut, QuadraticEaseInOut,
    CubicEaseIn, CubicEaseOut, CubicEaseInOut,
    QuarticEaseIn, QuarticEaseOut, QuarticEaseInOut,
    QuinticEaseIn, QuinticEaseOut, QuinticEaseInOut,
    SineEaseIn, SineEaseOut, SineEaseInOut,
    CircularEaseIn, CircularEaseOut, CircularEaseInOut,
    ExponentialEaseIn, ExponentialEaseOut, ExponentialEaseInOut,
    ElasticEaseIn, ElasticEaseOut, ElasticEaseInOut,
    BounceEaseIn, BounceEaseOut, BounceEaseInOut
};

/// Function pointer representing any of the easing functions defined in this file
template <typename T> using EasingFunc = T(*)(T);

/**
 * Returns the easing function that is named by \p func.
 *
 * \param func The enum that defines the easing function that is returned by this function
 * \return A function pointer to the easing function that is to be called
 */
template <typename T>
EasingFunc<T> easingFunction(EasingFunction func);

/**
 * Converts the passed \p func enum into a textual representation, which can be used for
 * the #easingFunctionFromName to recreate the same enum.
 *
 * \param func The easing function for which the name should be retrieved
 * \return The name of the easing function
 */
const char* nameForEasingFunction(EasingFunction func);

/**
 * Returns the enum for the passed \p name of an easing function. If \p name is not a
 * valid easing function name, a ghoul::MissingCaseException is raised.
 *
 * \param name The name for which the easing function enum should be returned
 * \return The enum value for the easing function named \p name
 *
 * \throw std::invalid_argument If \p name is not a valid name for an easing function
 * \pre \p name must not be nullptr
 */
EasingFunction easingFunctionFromName(const char* name);

/**
 * Returns whether \p name is naming a valid easing function. If this function returns
 * \c true and \p name is passed to #easingFunctionFromName, the function is guaranteed to
 * not throw.
 *
 * \param name The name for which should be tested whether it is a valid easing function
 *        name
 * \return \c true if \p name is a valid name for an easing function, \c false otherwise
 *
 * \pre \p name must not be nullptr
 */
bool isValidEasingFunctionName(const char* name);

/**
 * Interpolates the parameter \p p, which has to be in [0,1] using the easing function
 * named by \p func, which is used to a call for #easingFunction to determine which easing
 * function is used.
 *
 * \param p The parameter to be interpolated in [0,1]
 * \param func The =enum that defines the easing function that is used to ease/interpolate
 *        \p p
 * \return The interpolated value using the easing function defined by \p func
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T easing(T p, EasingFunction func);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the line
 * <code>y = x</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T linear(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the parabola
 * <code>y = x^2</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T quadraticEaseIn(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the parabola
 * <code>y = -x^2 + 2x</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T quadraticEaseOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the piecewise
 * quadratic:
 * <code>y = (1/2)((2x)^2)             ; [0, 0.5)</code>
 * <code>y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */

template <typename T>
T quadraticEaseInOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the cubic
 * <code>y = x^3</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T cubicEaseIn(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the cubic
 * <code>y = (x - 1)^3 + 1</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T cubicEaseOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the piecewise
 * cubic:
 * <code<y = (1/2)((2x)^3)       ; [0, 0.5)</code>
 * <code> y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T cubicEaseInOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the quartic
 * <code>y = x^4</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T quarticEaseIn(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the quartic
 * <code>y = 1 - (x - 1)^4</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T quarticEaseOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the piecewise
 * quartic:
 * <code>y = (1/2)((2x)^4)        ; [0, 0.5)</code>
 * <code>y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T quarticEaseInOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the quintic
 * <code>y = x^5</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T quinticEaseIn(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the quintic
 * <code>y = (x - 1)^5 + 1</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T quinticEaseOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the piecewise
 * quintic:
 * <code>y = (1/2)((2x)^5)       ; [0, 0.5)</code>
 * <code>y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T quinticEaseInOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the
 * quarter-cycle of sine wave.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T sineEaseIn(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the
 * quarter-cycle of sine wave (different phase).
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T sineEaseOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the half sine
 * wave.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T sineEaseInOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the shifted
 * quadrant IV of unit circle.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T circularEaseIn(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the shifted
 * quadrant II of unit circle.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T circularEaseOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the piecewise
 * circular function:
 * <code>y = (1/2)(1 - sqrt(1 - 4x^2))           ; [0, 0.5)</code>
 * <code>y = (1/2)(sqrt(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */

template <typename T>
T circularEaseInOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the
 * exponential function: <code>y = 2^(10(x - 1))</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T exponentialEaseIn(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the
 * exponential function: <code>y = -2^(-10x) + 1</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T exponentialEaseOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the
 * piecewise exponential function:
 * <code>y = (1/2)2^(10(2x - 1))         ; [0,0.5)</code>
 * <code>y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */

template <typename T>
T exponentialEaseInOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the damped
 * sine wave: <code>y = sin(13pi/2*x)*pow(2, 10 * (x - 1))</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T elasticEaseIn(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the damped
 * sine wave: <code>y = sin(-13pi/2*(x + 1))*pow(2, -10x) + 1</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T elasticEaseOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the piecewise
 * exponentially-damped sine wave:
 * <code>y = (1/2)*sin(13pi/2*(2*x))*pow(2, 10 * ((2*x) - 1))      ; [0,0.5)</code>
 * <code>y = (1/2)*(sin(-13pi/2*((2x-1)+1))*pow(2,-10(2*x-1)) + 2) ; [0.5, 1]</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T elasticEaseInOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the
 * overshooting cubic: <code>y = x^3-x*sin(x*pi)</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T backEaseIn(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the
 * overshooting cubic: <code>y = 1-((1-x)^3-(1-x)*sin((1-x)*pi))</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T backEaseOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the piecewise
 * overshooting cubic function:
 * <code>y = (1/2)*((2x)^3-(2x)*sin(2*x*pi))           ; [0, 0.5)</code>
 * <code>y = (1/2)*(1-((1-x)^3-(1-x)*sin((1-x)*pi))+1) ; [0.5, 1]</code>
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T backEaseInOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the quadratic
 * equation: <code>ax^2 + bx + c</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T bounceEaseIn(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the quadratic
 * equation: <code>ax^2 + bx + c</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T bounceEaseOut(T p);

/**
 * Interpolates the parameter \p p, which has to be in [0,1]. Modeled after the quadratic
 * equation: <code>ax^2 + bx + c</code>.
 *
 * \tparam T The type of the value to be interpreted. Must be an arithmetic type
 * \param p The parameter to be interpolated in [0,1]
 * \return The interpolated value; in [0,1]
 *
 * \pre \p p must be in [0,1]
 */
template <typename T>
T bounceEaseInOut(T p);

// extern template linear<float>;
// extern template quadraticEaseIn<float>;
// extern template quadraticEaseOut<float>;
// extern template quadraticEaseInOut<float>;
// extern template cubicEaseIn<float>;
// extern template cubicEaseOut<float>;
// extern template cubicEaseInOut<float>;
// extern template quarticEaseIn<float>;
// extern template quarticEaseOut<float>;
// extern template quarticEaseInOut<float>;
// extern template quinticEaseIn<float>;
// extern template quinticEaseOut<float>;
// extern template quinticEaseInOut<float>;
// extern template quinticEaseIn<float>;
// extern template quinticEaseOut<float>;
// extern template quinticEaseInOut<float>;
// extern template sineEaseIn<float>;
// extern template sineEaseOut<float>;
// extern template sineEaseInOut<float>;
// extern template circularEaseIn<float>;
// extern template circularEaseOut<float>;
// extern template circularEaseInOut<float>;
// extern template exponentialEaseIn<float>;
// extern template exponentialEaseOut<float>;
// extern template exponentialEaseInOut<float>;
// extern template elasticEaseIn<float>;
// extern template elasticEaseInOut<float>;
// extern template elasticEaseOut<float>;
// extern template bounceEaseIn<float>;
// extern template bounceEaseOut<float>;
// extern template bounceEaseInOut<float>;

// extern template linear<double>;
// extern template quadraticEaseIn<double>;
// extern template quadraticEaseOut<double>;
// extern template quadraticEaseInOut<double>;
// extern template cubicEaseIn<double>;
// extern template cubicEaseOut<double>;
// extern template cubicEaseInOut<double>;
// extern template quarticEaseIn<double>;
// extern template quarticEaseOut<double>;
// extern template quarticEaseInOut<double>;
// extern template quinticEaseIn<double>;
// extern template quinticEaseOut<double>;
// extern template quinticEaseInOut<double>;
// extern template quinticEaseIn<double>;
// extern template quinticEaseOut<double>;
// extern template quinticEaseInOut<double>;
// extern template sineEaseIn<double>;
// extern template sineEaseOut<double>;
// extern template sineEaseInOut<double>;
// extern template circularEaseIn<double>;
// extern template circularEaseOut<double>;
// extern template circularEaseInOut<double>;
// extern template exponentialEaseIn<double>;
// extern template exponentialEaseOut<double>;
// extern template exponentialEaseInOut<double>;
// extern template elasticEaseIn<double>;
// extern template elasticEaseInOut<double>;
// extern template elasticEaseOut<double>;
// extern template bounceEaseIn<double>;
// extern template bounceEaseOut<double>;
// extern template bounceEaseInOut<double>;

} // namespace ghoul

#include "easing.inl"

#endif // __GHOUL___EASING___H__
