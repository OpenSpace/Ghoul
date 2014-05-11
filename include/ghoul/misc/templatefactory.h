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
#include <functional>

#include <ghoul/misc/dictionary.h>

namespace ghoul {

class TemplateFactoryBase {
public:
    virtual const std::type_info& baseClassType() const = 0;
    virtual ~TemplateFactoryBase();
};

/**
 * This class implements a generic Factory pattern that can be used for any class as long
 * as a default constructor and/or constructor taking a #Dictionary is available for the
 * subclass. The usage of this TemplateFactory is as follows: The <code>BaseClass</code>
 * template parameter is the base class of all classes that can be created using this
 * factory. Only subclasses of <code>BaseClass> can be added to the factory, or a
 * compile-time error will be generated. Subclasses of the <code>BaseClass</code> (or the
 * <code>BaseClass</code> itself) can be registered using the #registerClass method, which
 * takes the subclass as a template parameter. All registered classes can then be created
 * using the #create method, providing the same <code>className</code> with which the
 * class was registered. If the subclass that was registered provides both an empty
 * default constructor as well as a constructor taking a single
 * <code>const ghoul::Dictionary&</code> parameter, the second #create method can be used
 * which takes the <code>className</code> as well as the #Dictionary with which to
 * instantiate the class. If this method is used and the class does not provide a
 * #Dictionary constructor, an error is logged (if the source was compiled with
 * <code>GHL_DEBUG</code>). In any case, the #Dictionary is silently ignored. Likewise, if
 * the registered class does not provide a default constructor and is called with the
 * first #create method, a similar error is logged (and the object is constructed using an
 * empty #Dictionary instead).  #hasClass tests if a specific <code>className</code> was
 * registered previously. For example:
 * \verbatim
class A {};
class B : public A {};
class C {};
class D : public A {
    D(const ghoul::Dictionary&);
};

TemplateFactory<A> factory;
factory.registerClass<B>("B");
factory.registerClass<C>("C"); // compile error as C is not a subclass of A
factory.registerClass<D>("D");
A* i = factory.create("B"); // creates an instance of B with the default constructor
i = factory.create("D"); // creates an instance of D with the default constructor
i = factory.create("D", { }); // creates an instance of D with the Dictionary constructor
i = factory.create("B", { }); // will log an error as B does not have a Dictionary ctor
\endverbatim
 * \tparam BaseClass The base class of all classes that can be registered and created
 * using this factory
 */
template <typename BaseClass>
class TemplateFactory : public TemplateFactoryBase {
public:
    /**
    * This is a function pointer that is called when a new subclass is to be created and
    * must return the new-allocated class. The function pointer is stored in the
    * #TemplateFactory.
    * \param useDictionary <code>true</code> if the class was called with a provided
    * #Dictionary and the Dictionary should be used.
    * \param dict The Dictionary that should be used to initialize the subclass. If
    * <code>useDictionary</code> is <code>false</code>, this is the empty Dictionary.
    * \return The initialized subclass of type <code>BaseClass</code>
    */
    typedef BaseClass* (*FactoryFuncPtr)(bool useDictionary, const Dictionary& dict);

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
    * Creates an instance of the class which was registered under the provided
    * <code>className</code>. This creation uses the constructor of the class and passes
    * the <code>dictionary</code> to the constructor. For this method to work, the class
    * which was registered with <code>className</code> has to have a constructor with a
    * single #Dictionary as parameter. If <code>className</code> does not name a
    * registered class, a <code>nullptr</code> is returned and an error is logged. Classes
    * can be registered with the #registerClass method.
    * \param className The class name of the instance that should be created
    * \param dictionary The dictionary that will be passed to the constructor of the class
    * \return A fully initialized instance of the registered class, or
    * <code>nullptr</code> if <code>className</code> did not name a valid type
    */
    BaseClass* create(const std::string& className, const Dictionary& dictionary) const;

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
    void registerClass(std::string className);

    /**
     * Registers a class with the provided <code>className</code> and the user-defined
     * #FactoryFuncPtr. The <code>factoryFunction</code> must return a valid subclass of
     * <code>BaseClass</code> when is it called in the #create methods of
     * #TemplateFactory. The function pointer is stored inside an <b>no</b> closure will
     * be constructed. This means that it is the callers responsibility that the factory
     * function returns a valid type for each call of #create.
     * \param className The class name, which will be registered with the provided
     * factory function
     * \param factoryFunction The function pointer that will be called if the
     * TemplateFactory is asked to create a class of type <code>className</code>. The
     * first argument of the function pointer is <code>true</code> if the #create method
     * was called with a #Dictionary as an additional parameter, implying that the
     * subclass should be constructed using the #Dictionary provided as the second
     * argument. The factory function is free to ignore this request.
     */
    void registerClass(std::string className, FactoryFuncPtr factoryFunction);

    /**
    * Registers a class with the provided <code>className</code> and the user-defined
    * #FactoryFuncPtr. The <code>factoryFunction</code> must return a valid subclass of
    * <code>BaseClass</code> when is it called in the #create methods of
    * #TemplateFactory. The <code>std::function</code> object is stored inside. 
    * \param className The class name, which will be registered with the provided
    * factory function
    * \param factoryFunction The <code>std::function</code> that will be called if the
    * TemplateFactory is asked to create a class of type <code>className</code>. The
    * first argument of the function pointer is <code>true</code> if the #create method
    * was called with a #Dictionary as an additional parameter, implying that the
    * subclass should be constructed using the #Dictionary provided as the second
    * argument. The factory function is free to ignore this request.
    */
    void registerClass(std::string className, 
        std::function<BaseClass*(bool, const ghoul::Dictionary&)> factoryFunction);

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

    const std::type_info& baseClassType() const override;

private:
    typedef std::function<BaseClass*(bool, const ghoul::Dictionary&)> FactoryFunction;

    /// The map storing all the associations from <code>className</code> to classes
    std::map<std::string, FactoryFunction> _map;
};

} // namespace ghoul

#include "ghoul/misc/templatefactory.inl"

#endif // __TEMPLATEFACTORY_H__
