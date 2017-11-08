/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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
 * This class is heavily based on the boost::any class written by Kevlin Henny with      *
 * contibutation from Antony Polukhin, Ed Brey, Mark Rodgers, Peter Dimov, and James     *
 * Curran.                                                                               *
 * Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.                       *
 * Original file ddstributed under the Boost Software License, Version 1.0. (See copy at *
 * http://www.boost.org/LICENSE_1_0.txt)                                                 *
 ****************************************************************************************/

#ifndef __GHOUL___ANY___H__
#define __GHOUL___ANY___H__

#include <algorithm>
#include <memory>
#include <type_traits>
#include <typeinfo>

namespace ghoul {

/**
 * An object of type !any is able to represent any object. When accessing the stored value
 * using !any_cast%, the correct type must be requested or an exception is thrown.
 */
class any {
public:
    /// Constructor creating an empty any object
    any() noexcept;

    /**
     * Constructor creating an !any object from the \p value.
     * \param value The value that should be stored in the any object
     */ 
    template<typename ValueType>
    any(const ValueType& value);

    /**
     * Constructor duplicating the contents of another any object. The specific type is
     * not needed for this function
     */
    any(const any& other);

    /**
     * Move constructor that initializes this !any object's contents with the passed
     * object, leaving it uninitialized in the process.
     * \param other The object used to create this !any object
     */
    any(any&& other) noexcept;

    /**
     * If the \p value is of a correct type, we can use perfect forwarding for efficiency
     * reasons when move-constructing an !any object.
     * \param value The value from which to create this !any object
     */
    template <typename ValueType>
    any(ValueType&& value,
        typename std::enable_if_t<!std::is_same<any&, ValueType>::value>* = nullptr,
        typename std::enable_if_t<!std::is_const<ValueType>::value>* = nullptr);

    /// Default destructor 
    ~any() noexcept;

    /**
     * Swaps the contents of \p rhs and <code>this</code> object.
     * \param rhs The !any object which which <code>this</code> will swap its contents
     */
    any& swap(any& rhs) noexcept;

    /**
     * Default assignment operator.
     * \param rhs The value that should be assigned to <code>this</code>
     */
    any& operator=(const any& rhs);

    /**
     * Standard move operator that leaves <code>this</code> !any object in an emtpy state.
     * \param rhs The source from which to move-assign <code>this</code> !any object
     */
    any& operator=(any&& rhs) noexcept;

    /**
     * Assigns the values of \p rhs to <code>this</code> !any object by perfectly
     * forwarding the contents.
     * \param rhs The value used to move-assign to <code>this</code> any object
     */
    template <typename ValueType>
    any& operator=(ValueType&& rhs);

    /**
     * Returns <code>true</code> if this !any object is unassigned, deleting any stored
     * content in the process.
     * \return <code>true</code> if this !any object is unassigned, <code>false</code>
     * otherwise
     */
    bool empty() const noexcept;

    /**
     * Clears this !any object and makes leaves it empty.
     */
    void clear() noexcept;

    /**
     * Returns the std::type_info of the stored value.
     * \return The std::type_info of the stored value
     */
    const std::type_info& type() const noexcept;

    /**
     * This abstract internal class is a baseclass to !holder, which stores the contents
     * for an !any object.
    */
    class placeholder {
    public:
        /// Default virtual destructor
        virtual ~placeholder();

        /**
         * This method returns the std::type_info of the stored value
         * \returns The std::type_info of the stored value
         */
        virtual const std::type_info& type() const noexcept = 0;

        /**
         * Creates a copy of the stored value by calling its copy constructor
         * \return A copy of the stored value by calling its copy constructor
         */
        virtual placeholder* clone() const = 0;
    };

    /**
     * The concrete implementation of a placeholder using a specific type of value held.
     * The <code>ValueType</code> must be copy constructable and move constructable.
     */
    template <typename ValueType>
    class holder : public placeholder {
    public:
        /**
         * Constructor that will take a copy of the \p value and stores it internally.
         * \param value The value to be stored in the holder
         */
        holder(const ValueType& value);

        /**
         * Move constructor that will store the contents of \p value inside and leave the
         * \p value in an undefined state.
         * \param value The value that is used to initialize this holder
         */
        holder(ValueType&& value);

        /**
         * Returns the std::type_info struct that contains the type information for the
         * stored value type. This call is equivalent to calling
         * <code>typeid(ValueType)</code>.
         * \return The std::type_info structure describing the stored value's type
         */
        const std::type_info& type() const noexcept override;

        /**
         * Clones the stored value by invoking the copy constructor of
         * <code>ValueType</code>.
         * \return A copy of the stored value
         */
        placeholder* clone() const override;

        /// The object that is contained in this !holder
        ValueType held;

    private:
        holder& operator=(const holder&) = delete;

    };

    std::unique_ptr<placeholder> content;
};

/**
 * And exception of this class is thrown if an any_cast fails due to mismatching types.
 */
class bad_any_cast : public std::bad_cast {
public:
    const char* what() const noexcept override; 
};

/**
 * This method casts an !any object \p operand into a type <code>ValueType</code> and
 * returns it.
 * \param operand The !any object that is to be converted into the type
 * <code>ValueType</code>
 * \return The converted \p operand
 * \throw bad_any_cast If the \p operand does not contain a value of type
 * <code>ValueType</code>
 * \pre \p operand must not be <code>nullptr</code>
 * \post The return value is not <code>nullptr</code>
 */
template <typename ValueType>
ValueType* any_cast(any* operand);

/**
* This method casts an !any object \p operand into a type <code>ValueType</code> and
* returns it.
* \param operand The !any object that is to be converted into the type
* <code>ValueType</code>
* \return The converted \p operand
* \throw bad_any_cast If the \p operand does not contain a value of type
* <code>ValueType</code>
* \pre \p operand must not be <code>nullptr</code>
* \post The return value is not <code>nullptr</code>
*/
template<typename ValueType>
inline const ValueType* any_cast(const any* operand);

/**
* This method casts an !any object \p operand into a type <code>ValueType</code> and
* returns it.
* \param operand The !any object that is to be converted into the type
* <code>ValueType</code>
* \return The converted \p operand
* \throw bad_any_cast If the \p operand does not contain a value of type
* <code>ValueType</code>
*/
template<typename ValueType>
ValueType any_cast(any& operand);

/**
* This method casts an !any object \p operand into a type <code>ValueType</code> and
* returns it.
* \param operand The !any object that is to be converted into the type
* <code>ValueType</code>
* \return The converted \p operand
* \throw bad_any_cast If the \p operand does not contain a value of type
* <code>ValueType</code>
*/
template<typename ValueType>
inline ValueType any_cast(const any& operand);

/**
* This method casts an !any object \p operand into a type <code>ValueType</code> and
* returns it.
* \param operand The !any object that is to be converted into the type
* <code>ValueType</code>
* \return The converted \p operand
* \throw bad_any_cast If the \p operand does not contain a value of type
* <code>ValueType</code>
*/
template<typename ValueType>
inline ValueType any_cast(any&& operand);

} // namespace

#include "any.inl"

#endif // __GHOUL___ANY___H__
