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

#include <ghoul/misc/assert.h>

namespace ghoul {

template<typename ValueType>
any::any(const ValueType& value)
    : content(new holder<typename std::remove_cv_t<typename std::decay_t<const ValueType>>
        >(value))
{}

template <typename ValueType>
any::any(ValueType&& value,
    // disable if value has type `any&`
    typename std::enable_if_t<!std::is_same<any&, ValueType>::value>*,
    // disable if value has type `const ValueType&&`
    typename std::enable_if_t<!std::is_const<ValueType>::value>*)
    : content(new holder<
                typename std::decay_t<ValueType>>(static_cast<ValueType&&>(value)))
{}

template <typename ValueType>
any& any::operator=(ValueType&& rhs) {
    any(static_cast<ValueType&&>(rhs)).swap(*this);
    return *this;
}

template <typename ValueType>
any::holder<ValueType>::holder(const ValueType& value)
    : held(value)
{}

template <typename ValueType>
any::holder<ValueType>::holder(ValueType&& value)
    : held(static_cast<ValueType&&>(value))
{}

template <typename ValueType>
const std::type_info& any::holder<ValueType>::type() const noexcept {
    return typeid(ValueType);
}

template<typename ValueType>
any::placeholder* any::holder<ValueType>::clone() const {
    return new holder(held);
}

template <typename ValueType>
ValueType* any_cast(any* operand) {
    ghoul_assert(operand, "Operand for any_cast must not be nullptr");
    if (operand->type() == typeid(ValueType)) {
        return &static_cast<
            any::holder<typename std::remove_cv_t<ValueType>>*
        >(operand->content.get())->held;
    }
    else {
        return nullptr;
    }
}

template <typename ValueType>
inline const ValueType* any_cast(const any* operand) {
    ghoul_assert(operand, "Operand for any_cast must not be nullptr");
    if (operand->type() == typeid(ValueType)) {
        return &static_cast<
            any::holder<typename std::remove_cv_t<ValueType>>*
        >(operand->content.get())->held;
    }
    else {
        return nullptr;
    }
}

template<typename ValueType>
ValueType any_cast(any& operand) {
    typedef typename std::remove_reference<ValueType>::type nonref;

    nonref* result = any_cast<nonref>(&operand);
    if (!result) {
        throw bad_any_cast();
    }

    // Attempt to avoid construction of a temporary object in cases when
    // `ValueType` is not a reference. Example:
    // `static_cast<std::string>(*result);`
    // which is equal to `std::string(*result);`
    //typedef typename boost::mpl::if_<
    //    boost::is_reference<ValueType>,
    //    ValueType,
    //    BOOST_DEDUCED_TYPENAME boost::add_reference<ValueType>::type
    //>::type ref_type;

    return static_cast<ValueType>(*result);
}

template<typename ValueType>
inline ValueType any_cast(const any& operand) {
    typedef typename std::remove_reference<ValueType>::type nonref;

    nonref* result = any_cast<nonref>(&operand);
    if (!result) {
        throw bad_any_cast();
    }

    // Attempt to avoid construction of a temporary object in cases when
    // `ValueType` is not a reference. Example:
    // `static_cast<std::string>(*result);`
    // which is equal to `std::string(*result);`
    //typedef typename boost::mpl::if_<
    //    boost::is_reference<ValueType>,
    //    ValueType,
    //    BOOST_DEDUCED_TYPENAME boost::add_reference<ValueType>::type
    //>::type ref_type;

    return static_cast<ValueType>(*result);
}

template<typename ValueType>
inline ValueType any_cast(any&& operand) {
    static_assert(
         /*true if ValueType is rvalue or just a value*/
        std::is_rvalue_reference<ValueType&&>::value ||
        std::is_const< typename std::remove_reference<ValueType>::type >::value,
        "ghoul::any_cast cannot be used for nonconst references to temporary objects"
        );
    return any_cast<ValueType>(operand);
}

} // namespace
