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

#ifndef __GHOUL___TEMPLATEFACTORY___H__
#define __GHOUL___TEMPLATEFACTORY___H__

#include <ghoul/misc/exception.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ghoul {

class Dictionary;

class TemplateFactoryBase {
public:
    /// Main exception that is thrown by the TemplateFactory in the case of errors
    struct TemplateFactoryError : public RuntimeError {
        explicit TemplateFactoryError(std::string msg);
    };

    /// Exception that is thrown if a requested class has not been registered before
    struct TemplateClassNotFoundError : public TemplateFactoryError {
        explicit TemplateClassNotFoundError(std::string name);
        std::string className;
    };

    /// Exception that is thrown if a registered class is called with a wrong constructor
    struct TemplateConstructionError : public TemplateFactoryError {
        explicit TemplateConstructionError(std::string msg);
    };

    virtual const std::type_info& baseClassType() const = 0;
    virtual ~TemplateFactoryBase() = default;

    virtual bool hasClass(const std::string& className) const = 0;
    virtual std::vector<std::string> registeredClasses() const = 0;
};

/**
 * This class implements a generic Factory pattern that can be used for any class as long
 * as a default constructor and/or constructor taking a Dictionary is available for the
 * subclass. The usage of this TemplateFactory is as follows: The <code>BaseClass</code>
 * template parameter is the base class of all classes that can be created using this
 * factory. Only subclasses of <code>BaseClass</code> can be added to the factory, or a
 * compile-time error will be generated. Subclasses of the <code>BaseClass</code> (or the
 * <code>BaseClass</code> itself) can be registered using the #registerClass method, which
 * takes the subclass as a template parameter. All registered classes can then be created
 * using the #create method, providing the same <code>className</code> with which the
 * class was registered. If the subclass that was registered provides both an empty
 * default constructor as well as a constructor taking a single
 * <code>const ghoul::Dictionary&</code> parameter, the second #create method can be used
 * which takes the <code>className</code> as well as the Dictionary with which to
 * instantiate the class. If this method is used and the class does not provide a
 * Dictionary constructor, an exception is thrown. Likewise, if the registered class does
 * not provide a default constructor and is called with the first #create method, a
 * similar exception is thrown. #hasClass tests if a specific <code>className</code> was
 * registered previously. For example:
 * \verbatim
class A {};
class B : public A {};
class C {};
class D : public A {
public:
    D(const ghoul::Dictionary&);
};

TemplateFactory<A> factory;
factory.registerClass<B>("B");
factory.registerClass<C>("C"); // compile error as C is not a subclass of A
factory.registerClass<D>("D");
A* i = factory.create("B"); // creates an instance of B with the default constructor
i = factory.create("D"); // creates an instance of D with the default constructor
i = factory.create("D", { }); // creates an instance of D with the Dictionary constructor
i = factory.create("B", { }); // throws an exception as B does not have a Dictionary ctor
\endverbatim
 *
 * \tparam BaseClass The base class of all classes that can be registered and created
 * using this factory
 */
template <typename BaseClass>
class TemplateFactory : public TemplateFactoryBase {
public:
    /**
     * This is a function pointer that is called when a new subclass is to be created and
     * must return the new-allocated class. The function pointer is stored in the
     * TemplateFactory.
     *
     * \param useDictionary <code>true</code> if the class was called with a provided
     *        Dictionary and the Dictionary should be used.
     * \param dict The Dictionary that should be used to initialize the subclass. If
     *        \p useDictionary is <code>false</code>, this is the empty Dictionary.
     * \return The initialized subclass of type <code>BaseClass</code>
     *
     * \throw TemplateConstructionError If the class was initialized using the wrong
     *        constructor, for example a class that does not have a Dictionary
     *        constructor, but a Dictionary was used.
     */
    using FactoryFuncPtr = BaseClass* (*)(bool useDictionary, const Dictionary& dict);

    /**
     * Creates an instance of the class which was registered under the provided
     * \p className. This creation uses the parameterless default constructor of the
     * class. If \p className does not name a registered class, an exception is thrown.
     * Classes can be registered with the #registerClass method.
     *
     * \param className The class name of the instance that should be created
     * \return A fully initialized instance of the registered class
     *
     * \throw TemplateClassNotFoundError If the \p className did not name a previously
     *        registered class
     * \throw TemplateConstructionError If the class registered under \p className does
     *        not have a default constructor
     * \pre \p className must not be empty
     */
    std::unique_ptr<BaseClass> create(const std::string& className) const;

    /**
     * Creates an instance of the class which was registered under the provided
     * \p className. This creation uses the constructor of the class and passes the
     * \p dictionary to the constructor. For this method to work, the class which was
     * registered with \p className has to have a constructor with a single Dictionary as
     * parameter. If \p className does not name a registered class, an exception is
     * thrown. Classes can be registered with the #registerClass method.
     *
     * \param className The class name of the instance that should be created
     * \param dictionary The dictionary that will be passed to the constructor of the
     *        class
     * \return A fully initialized instance of the registered class
     *
     * \throw TemplateClassNotFoundError If the \p className did not name a previously
     *        registered class
     * \throw TemplateConstructionError If the class registered under \p className does
     *        not have a constructor using a Dictionary object
     * \pre \p className must not be empty
     */
    std::unique_ptr<BaseClass> create(const std::string& className,
        const Dictionary& dictionary) const;

    /**
     * Registers a <code>Class</code> with the provided \p className so that it can later
     * be #create%d using the same \p className. <code>Class</code> has to be a subclass
     * of <code>BaseClass</code> or a compile-error will occur. If a class already has
     * been registered under the \p className, an exception will be thrown.
     *
     * \tparam Class The class that should be registered under the provided
     *         \p className
     * \param className The class name under which the <code>Class</code> is registered.
     *
     * \throw TemplateFactoryError If the \p className has been registered before
     * \pre <code>Class</code> must be derived from <code>BaseClass</code>
     * \pre <code>Class</code> must have a default constructor or a constructor that takes
     *      a Dictionary
     * \pre \p className must not be empty
     */
    template <typename Class>
    void registerClass(std::string className);

    /**
     * Registers a class with the provided \p className and the user-defined
     * #FactoryFuncPtr. The \p factoryFunction must return a valid subclass of
     * <code>BaseClass</code> when is it called in the #create methods of TemplateFactory.
     * The function pointer is stored inside and <b>no</b> closure will be constructed.
     * This means that it is the callers responsibility that the factory function returns
     * a valid type for each call of #create.
     *
     * \param className The class name, which will be registered with the provided
     *        factory function
     * \param factoryFunction The function pointer that will be called if the
     *        TemplateFactory is asked to create a class of type \p className. The first
     *        argument of the function pointer is <code>true</code> if the #create method
     *        was called with a Dictionary as an additional parameter, implying that the
     *        subclass should be constructed using the Dictionary provided as the second
     *        argument. The factory function is free to ignore this request.
     *
     * \throw TemplateFactoryError If the \p className has been registered before
     * \pre \p className must not be empty
     * \pre \p factoryFunction must not be <code>nullptr</code>
     */
    void registerClass(std::string className, FactoryFuncPtr factoryFunction);

    /**
     * Registers a class with the provided \p className and the user-defined
     * #FactoryFuncPtr. The \p factoryFunction must return a valid subclass of
     * <code>BaseClass</code> when is it called in the #create methods of TemplateFactory.
     * The <code>std::function</code> object is stored inside.
     *
     * \param className The class name, which will be registered with the provided
     *        factory function
     * \param factoryFunction The <code>std::function</code> that will be called if the
     *        TemplateFactory is asked to create a class of type \p className. The first
     *        argument of the function pointer is <code>true</code> if the #create method
     *        was called with a Dictionary as an additional parameter, implying that the
     *        subclass should be constructed using the Dictionary provided as the second
     *        argument. The factory function is free to ignore this request.
     *
     * \throw TemplateFactoryError If the \p className has been registered before
     * \pre \p className must not be empty
     * \pre \p factoryFunction must not be <code>nullptr</code>
     */
    void registerClass(std::string className,
        std::function<BaseClass*(bool, const ghoul::Dictionary&)> factoryFunction);

    /**
     * Checks if any class has been registered under the provided \p className As any
     * invalid class will create a compile-time error, if a class has been registered,
     * it is guaranteed that the \p className can be used to generate a valid instance.
     *
     * \param className The class name that is to be tested
     * \return <code>true</code> if the \p className was used in #registerClass
     *         before; <code>false</code> otherwise
     *
     * \pre \p className must not be empty
     */
    bool hasClass(const std::string& className) const override;

    /**
     * Returns the list of all registered classes. All values in this vector can be used
     * to instantiate a new class sing the #create method.
     *
     * \return The list of all registered classes
     */
    std::vector<std::string> registeredClasses() const override;

    /**
     * Returns the <code>type_info</code> of the baseclass for this factory, i.e., the
     * base class which is the super class for all actual instances that can be created
     * using the #create method.
     *
     * \return The <code>type_info</code> of the baseclass for this factory
     */
    const std::type_info& baseClassType() const override;

private:
    using FactoryFunction = std::function<BaseClass*(bool, const ghoul::Dictionary&)>;

    /// The map storing all the associations from <code>className</code> to classes
    std::map<std::string, FactoryFunction> _map;
};

} // namespace ghoul

#include "ghoul/misc/templatefactory.inl"

#endif // __GHOUL___TEMPLATEFACTORY___H__
