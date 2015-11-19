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

namespace ghoul {
namespace cmdparser {

template<typename T, typename U, typename V, typename W>
SingleCommand<T, U, V, W>::SingleCommand(T* ptr1, std::string name,
                             std::string shortName,
                             std::string infoText,
                             std::string parameterList)
    : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText), 
        std::move(parameterList), 1, false)
	, _ptr1(ptr1)
	, _ptr2(nullptr)
	, _ptr3(nullptr)
	, _ptr4(nullptr)
{}

template<typename T, typename U, typename V, typename W>
SingleCommand<T, U, V, W>::SingleCommand(T* ptr1, U* ptr2, std::string name,
                             std::string shortName,
                             std::string infoText,
                             std::string parameterList)
    : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText),
        std::move(parameterList), 2, false)
	, _ptr1(ptr1)
	, _ptr2(ptr2)
	, _ptr3(nullptr)
	, _ptr4(nullptr)
{}

template<typename T, typename U, typename V, typename W>
SingleCommand<T, U, V, W>::SingleCommand(T* ptr1, U* ptr2, V* ptr3,
							 std::string name,
                             std::string shortName,
                             std::string infoText,
                             std::string parameterList)
    : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText),
        std::move(parameterList), 3, false)
	, _ptr1(ptr1)
	, _ptr2(ptr2)
	, _ptr3(ptr3)
	, _ptr4(nullptr)
{}

template<typename T, typename U, typename V, typename W>
SingleCommand<T, U, V, W>::SingleCommand(T* ptr1, U* ptr2, V* ptr3, W* ptr4,
							 std::string name,
							 std::string shortName,
							 std::string infoText,
							 std::string parameterList)
    : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText),
        std::move(parameterList), 4, false)
	, _ptr1(ptr1)
	, _ptr2(ptr2)
	, _ptr3(ptr3)
	, _ptr4(ptr4)
{}

template<typename T, typename U, typename V, typename W>
bool SingleCommand<T, U, V, W>::execute(const std::vector<std::string>& parameters) {
    cast(parameters[0], *_ptr1);
    if (_ptr2 != nullptr)
        cast(parameters[1], *_ptr2);
    if (_ptr3 != nullptr)
        cast(parameters[2], *_ptr3);
    if (_ptr4 != nullptr)
        cast(parameters[3], *_ptr4);

    return true;
}

template<typename T, typename U, typename V, typename W>
bool SingleCommand<T, U, V, W>::checkParameters(
										const std::vector<std::string>& parameters)
{
    std::ostringstream errorStr;

    bool result = parameters.size() == static_cast<size_t>(_argumentNum);
    if (!result) {
        errorStr << "Invalid number of parameters: " << parameters.size();
        errorStr << ", expected: " << _argumentNum;
        _errorMsg = errorStr.str();
        return false;
    }

    result &= is<T>(parameters[0]);
    if (!result)
        errorStr << "First parameter invalid";

    if (result && (_ptr2 != nullptr)) {
        result &= is<U>(parameters[1]);
        if (!result)
            errorStr << "Second parameter invalid";
    }

    if (result && (_ptr3 != nullptr)) {
        result &= is<V>(parameters[2]);
        if (!result)
            errorStr << "Third parameter invalid";
    }

    if (result && (_ptr4 != nullptr)) {
        result &= is<W>(parameters[3]);
        if (!result)
            errorStr << "Fourth parameter invalid";
    }

    if (!result)
        _errorMsg = errorStr.str();

    return result;
}

} // namespace cmdparser
} // namespace ghoul
