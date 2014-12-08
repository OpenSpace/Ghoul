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

namespace ghoul {
namespace cmdparser {

template<class T, class U = T, class V = U, class W = V>
MultipleCommand<T,U,V,W>::MultipleCommand(std::vector<T>* ptr1, std::string name,
                std::string shortName, std::string infoText, std::string parameterList)
    : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText),
		std::move(parameterList), 1, true)
    , _ptr1(ptr1)
    , _ptr2(nullptr)
    , _ptr3(nullptr)
    , _ptr4(nullptr)
{}

template<class T, class U = T, class V = U, class W = V>
MultipleCommand<T,U,V,W>::MultipleCommand(std::vector<T>* ptr1, std::vector<U>* ptr2,
                std::string name, std::string shortName, std::string infoText,
                std::string parameterList)
    : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText),
		std::move(parameterList), 2, true)
    , _ptr1(ptr1)
    , _ptr2(ptr2)
    , _ptr3(nullptr)
    , _ptr4(nullptr)
{}

template<class T, class U = T, class V = U, class W = V>
MultipleCommand<T,U,V,W>::MultipleCommand(std::vector<T>* ptr1, std::vector<U>* ptr2,
				std::vector<V>* ptr3, std::string name, std::string shortName,
                std::string infoText, std::string parameterList)
    : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText), std::move(parameterList), 3, true)
    , _ptr1(ptr1)
    , _ptr2(ptr2)
    , _ptr3(ptr3)
    , _ptr4(nullptr)
{}

template<class T, class U = T, class V = U, class W = V>
MultipleCommand<T,U,V,W>::MultipleCommand(std::vector<T>* ptr1, std::vector<U>* ptr2, std::vector<V>* ptr3,
                std::vector<W>* ptr4, std::string name, std::string shortName,
				std::string infoText, std::string parameterList)
    : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText),
		std::move(parameterList), 4, true)
    , _ptr1(ptr1)
    , _ptr2(ptr2)
    , _ptr3(ptr3)
    , _ptr4(ptr4)
{}

template<class T, class U = T, class V = U, class W = V>
bool MultipleCommand<T,U,V,W>::execute(const std::vector<std::string>& parameters) {
    T v1;
    cast(parameters[0], v1);
    _ptr1->push_back(v1);
    if (_ptr2 != nullptr) {
        U v2;
        cast<U>(parameters[1], v2);
        _ptr2->push_back(v2);
    }
    if (_ptr3 != nullptr) {
        V v3;
        cast<V>(parameters[2], v3);
        _ptr3->push_back(v3);
    }
    if (_ptr4 != nullptr) {
        W v4;
        cast<W>(parameters[3], v4);
        _ptr4->push_back(v4);
    }

    return true;
}

template<class T, class U = T, class V = U, class W = V>
bool MultipleCommand<T,U,V,W>::checkParameters(const std::vector<std::string>& parameters) {
    bool result = parameters.size() == static_cast<size_t>(_argumentNum);

    result &= is<T>(parameters[0]);
    if (_ptr2 != nullptr)
        result &= is<U>(parameters[1]);
    if (_ptr3 != nullptr)
        result &= is<V>(parameters[2]);
    if (_ptr4 != nullptr)
        result &= is<W>(parameters[3]);

    return result;
}

} // namespace cmdparser
} // namespace ghoul
