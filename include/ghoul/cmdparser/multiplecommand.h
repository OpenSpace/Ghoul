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

#ifndef __MULTIPLECOMMAND_H__
#define __MULTIPLECOMMAND_H__

#include <ghoul/cmdparser/commandlinecommand.h>

#include <vector>

namespace ghoul {
namespace cmdparser {

/**
 * This class represents a command that can called multiple times in a given commandline
 * and can have up to 4 arguments of respective types <code>T</code>, <code>U</code>,
 * <code>V</code>, and <code>U</code>. Each time the command is called, the converted
 * value is appended to a vector that has been passed in the constructor. The concrete
 * amount of variables is determined by the constructor used. The command tries to convert
 * the parameters to the appropriate types and stores them. The template classes
 * <code>T</code>, <code>U</code>, <code>V</code>, and <code>U</code> must be convertable
 * using an <code>std::stringstream</code>.
 * \tparam T The typename of the first argument type
 * \tparam U The typename of the second argument type, defaulting to <code>T</code>
 * \tparam V The typename of the third argument type, defaulting to <code>U</code>
 * \tparam W The typename of the fourth argument type, defaulting to <code>V>/code>
 * \sa MultipleCommandZeroArguments
 */
template<class T, class U = T, class V = U, class W = V>
class MultipleCommand : public CommandlineCommand {
public:
    MultipleCommand(std::vector<T>* ptr1, std::string name,
                    std::string shortName = "", std::string infoText = "",
                    std::string parameterList = "")
        : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText), std::move(parameterList), 1, true)
        , _ptr1(ptr1)
        , _ptr2(nullptr)
        , _ptr3(nullptr)
        , _ptr4(nullptr)
    {}

    MultipleCommand(std::vector<T>* ptr1, std::vector<U>* ptr2,
                    std::string name, std::string shortName = "",
                    std::string infoText = "",
                    std::string parameterList = "")
        : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText), std::move(parameterList), 2, true)
        , _ptr1(ptr1)
        , _ptr2(ptr2)
        , _ptr3(nullptr)
        , _ptr4(nullptr)
    {}

    MultipleCommand(std::vector<T>* ptr1, std::vector<U>* ptr2, std::vector<V>* ptr3,
        std::string name, std::string shortName = "",
                    std::string infoText = "",
                    std::string parameterList = "")
        : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText), std::move(parameterList), 3, true)
        , _ptr1(ptr1)
        , _ptr2(ptr2)
        , _ptr3(ptr3)
        , _ptr4(nullptr)
    {}

    MultipleCommand(std::vector<T>* ptr1, std::vector<U>* ptr2, std::vector<V>* ptr3,
                    std::vector<W>* ptr4, std::string name,
                    std::string shortName = "", std::string infoText = "",
                    std::string parameterList = "")
        : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText), std::move(parameterList), 4, true)
        , _ptr1(ptr1)
        , _ptr2(ptr2)
        , _ptr3(ptr3)
        , _ptr4(ptr4)
    {}

    bool execute(const std::vector<std::string>& parameters) {
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

    bool checkParameters(const std::vector<std::string>& parameters) {
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

protected:
    std::vector<T>* _ptr1;
    std::vector<U>* _ptr2;
    std::vector<V>* _ptr3;
    std::vector<W>* _ptr4;
};

/**
 * This class represents a command with zero arguments which can be called multiple times
 * in a given commandline. The <code>int</code> pointer will contain the number of how
 * often the command was present in the command-line.
 * \sa MultipleCommand
 */
class MultipleCommandZeroArguments : public CommandlineCommand {
public:
    MultipleCommandZeroArguments(int* ptr, std::string name, std::string shortName = "",
        std::string infoText = "")
        : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText), "", 0, true)
        , _ptr(ptr)
    {
        *_ptr = 0;
    }

    bool execute(const std::vector<std::string>& /*parameters*/) {
        ++(*_ptr);
        return true;
    }

protected:
    int* _ptr;
};

} // namespace cmdparser
} // namespace ghoul

#endif // __MULTIPLECOMMAND_H__
