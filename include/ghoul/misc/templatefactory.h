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

#ifndef __TEMPLATEFACTORY_H__
#define __TEMPLATEFACTORY_H__

#include <map>
#include <string>

namespace ghoul {

/**
 * This class implements a generic Factory pattern that can be used for any class as long
 * as a default constructor is available for each subclass. The usage of this
 * TemplateFactory is as follows: The <code>BaseClass</code> template parameter is the
 * base class of all classes that can be created using this factory. Only subclasses of
 * <code>BaseClass> can be added to the factory, or a compile-time error will be
 * generated. Subclasses of the <code>BaseClass</code> (or the <code>BaseClass</code>
 * itself) can be registered using the #registerClass method, which takes the subclass as
 * a template parameter. All registered classes can then be created using the #create
 * method, providing the same <code>className</code> with which the class was registered.
 * #hasClass tests if a specific <code>className</code> was registered previously. For
 * example:
 * \verbatim
class A {};
class B : public A {};
class C {};

TemplateFactory<A> factory;
factory.registerClass<B>("B");
A* instance = factory.create("B");
\endverbatim
 * Will create a correct instance of <code>B</code>, while
 * <code>factory.registerClass<C>("C")</code> will not compile, as <code>C</code> is not a
 * subclass of <code>A</code> with which the <code>factory</code> was created.
 * \tparam BaseClass The base class of all classes that can be registered and created
 * using this factory
 */
template <typename BaseClass>
class TemplateFactory {
public:
    /**
     * Creates an instance of the class which was registered under the provided
     * <code>className</code>. This creation uses the parameterless default constructor of
     * the class. If <code>className</code> does not name a registered class, a
     * <code>nullptr</code> is returned and an error is logged. Classes can be registered
     * with the #registerClass method.
     * \param className The class name of the instance that should be created
     * \return A fully initialized instance of the registered class, or
     * <code>nullptr</code> if <code>className</code> did not name a valid type
     */
    BaseClass* create(const std::string& className) const;

    /**
     * Registers a <code>Class</code> with the provided <code>className</code> so that it
     * can later be #create%d using the same <code>className</code>. <code>Class</code>
     * has to be a subclass of <code>BaseClass</code> or a compile-error will occur. If a
     * class already has been registered under the <code>className</code>, the old binding
     * will be silently overwritten.
     * \param className The class name under which the <code>Class</code> is registered.
     * \tparam Class The class that should be registered under the provided
     * <code>className</code>
     */
    template <typename Class>
    void registerClass(const std::string& className);

    /**
     * Checks if any class has been registered under the provided <code>className</code>.
     * As any invalid class will create a compile-time error, if a class has been
     * registered, it is guaranteed that the <code>className</code> can be used to
     * generate a valid instance.
     * \param className The class name that is to be tested
     * \return <code>true</code> if the <code>className</code> was used in #registerClass
     * before; <code>false</code> otherwise
     */
    bool hasClass(const std::string& className) const;

private:
    /**
     * The function pointer that is stored in the map. This pointer points to a templated
     * function, which is templated by the destination type.
     */
    typedef BaseClass* (*FactoryFuncPtr)();

    /// The map storing all the associations from <code>className</code> to classes
    std::map<std::string, FactoryFuncPtr> _map;
};

}

#include "ghoul/misc/templatefactory.inl"

#endif // __TEMPLATEFACTORY_H__
