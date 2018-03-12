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

#include <ghoul/misc/easing.h>

#include <array>

namespace {

const std::array<const char*, 28> EaseFuncNames = {
    "Linear",
    "QuadraticEaseIn", "QuadraticEaseOut", "QuadraticEaseInOut",
    "CubicEaseIn", "CubicEaseOut", "CubicEaseInOut",
    "QuarticEaseIn", "QuarticEaseOut", "QuarticEaseInOut",
    "QuinticEaseIn", "QuinticEaseOut", "QuinticEaseInOut",
    "SineEaseIn", "SineEaseOut", "SineEaseInOut",
    "CircularEaseIn", "CircularEaseOut", "CircularEaseInOut",
    "ExponentialEaseIn", "ExponentialEaseOut", "ExponentialEaseInOut",
    "ElasticEaseIn", "ElasticEaseOut", "ElasticEaseInOut",
    "BounceEaseIn", "BounceEaseOut", "BounceEaseInOut"
};

} // namespace

namespace ghoul {

const char* nameForEasingFunction(EasingFunction func) {
    return EaseFuncNames[static_cast<std::underlying_type_t<EasingFunction>>(func)];
}

EasingFunction easingFunctionFromName(const char* name) {
    for (size_t i = 0; i < EaseFuncNames.size(); ++i) {
        if (std::string(EaseFuncNames[i]) == std::string(name)) {
            return EasingFunction(i);
        }
    }
    throw std::invalid_argument(
        "Could not find easing function with name " + std::string(name)
    );
}

bool isValidEasingFunctionName(const char* name) {
    for (size_t i = 0; i < EaseFuncNames.size(); ++i) {
        if (std::string(EaseFuncNames[i]) == std::string(name)) {
            return true;
        }
    }
    return false;
}

// template linear<float>;
// template quadraticEaseIn<float>;
// template quadraticEaseOut<float>;
// template quadraticEaseInOut<float>;
// template cubicEaseIn<float>;
// template cubicEaseOut<float>;
// template cubicEaseInOut<float>;
// template quarticEaseIn<float>;
// template quarticEaseOut<float>;
// template quarticEaseInOut<float>;
// template quinticEaseIn<float>;
// template quinticEaseOut<float>;
// template quinticEaseInOut<float>;
// template quinticEaseIn<float>;
// template quinticEaseOut<float>;
// template quinticEaseInOut<float>;
// template sineEaseIn<float>;
// template sineEaseOut<float>;
// template sineEaseInOut<float>;
// template circularEaseIn<float>;
// template circularEaseOut<float>;
// template circularEaseInOut<float>;
// template exponentialEaseIn<float>;
// template exponentialEaseOut<float>;
// template exponentialEaseInOut<float>;
// template elasticEaseIn<float>;
// template elasticEaseInOut<float>;
// template elasticEaseOut<float>;
// template bounceEaseIn<float>;
// template bounceEaseOut<float>;
// template bounceEaseInOut<float>;

// template linear<double>;
// template quadraticEaseIn<double>;
// template quadraticEaseOut<double>;
// template quadraticEaseInOut<double>;
// template cubicEaseIn<double>;
// template cubicEaseOut<double>;
// template cubicEaseInOut<double>;
// template quarticEaseIn<double>;
// template quarticEaseOut<double>;
// template quarticEaseInOut<double>;
// template quinticEaseIn<double>;
// template quinticEaseOut<double>;
// template quinticEaseInOut<double>;
// template quinticEaseIn<double>;
// template quinticEaseOut<double>;
// template quinticEaseInOut<double>;
// template sineEaseIn<double>;
// template sineEaseOut<double>;
// template sineEaseInOut<double>;
// template circularEaseIn<double>;
// template circularEaseOut<double>;
// template circularEaseInOut<double>;
// template exponentialEaseIn<double>;
// template exponentialEaseOut<double>;
// template exponentialEaseInOut<double>;
// template elasticEaseIn<double>;
// template elasticEaseInOut<double>;
// template elasticEaseOut<double>;
// template bounceEaseIn<double>;
// template bounceEaseOut<double>;
// template bounceEaseInOut<double>;





} // namespace ghoul
