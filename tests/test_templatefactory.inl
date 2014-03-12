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

#include "gtest/gtest.h"
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/templatefactory.h>

/*
Test checklist:
 +++ Correctness for direct subclass
 +++ Non-interference
 +++ Deep inheritance
 +++ Default constructor
 +++ Default constructor does not exist
 +++ Dictionary constructor
 +++ Dictionary constructor does not exist
 +++ Default + Dictionary ctor
 +++ Class does not exist
 +++ Correctness for 'hasClass'
 +++ Custom factory function pointer
 +++ Custom factory std::function
*/

namespace {

struct BaseClass {
public:
    BaseClass() : value1(-1), value2(-2) {};
    virtual ~BaseClass() {} // virtual method necessary for RTTI

    int value1;
    int value2;
};

struct SubClassDefault : public BaseClass {
public:
    SubClassDefault() 
        : BaseClass()
    {
        value1 = 1;
        value2 = 2;
    };
};

struct SubClassDefault2 : public BaseClass {
public:
    SubClassDefault2()
        : BaseClass()
    {
        value1 = 21;
        value2 = 22;
    }
};

struct SubClassDictionary : public BaseClass {
public:
    SubClassDictionary(const ghoul::Dictionary& dict)
        : BaseClass()
    {
        dict.getValue("value1", value1);
        dict.getValue("value2", value2);
    };
};

struct SubClassDefaultDictionary : public BaseClass {
public:
    SubClassDefaultDictionary() 
        : BaseClass()
    {
        value1 = 31;
        value2 = 32;
    };
    SubClassDefaultDictionary(const ghoul::Dictionary& dict) 
        : BaseClass()
    {
        dict.getValue("value1", value1);
        dict.getValue("value2", value2);
    };
};

struct SubClassMultipleLayers : public SubClassDefault {
    SubClassMultipleLayers() {};
};

struct FunctionPointerClass : public BaseClass {};
struct StdFunctionClass : public BaseClass {};

BaseClass* createFunctionPointerClass(bool useDictionary, const ghoul::Dictionary&) {
    if (useDictionary)
        return new FunctionPointerClass;
    else
        return nullptr;
}

}

class TemplateFactoryTest : public testing::Test {
protected:
    TemplateFactoryTest() {
        factory = new ghoul::TemplateFactory<BaseClass>;
    }

    ~TemplateFactoryTest() {
        delete factory;
    }

    void reset() {
        delete factory;
        factory = new ghoul::TemplateFactory<BaseClass>;
    }

    ghoul::TemplateFactory<BaseClass>* factory;
};

TEST_F(TemplateFactoryTest, CorrectnessDirectSublass) {
    factory->registerClass<SubClassDefault>("SubClassDefault");
    
    BaseClass* obj = factory->create("SubClassDefault");
    ASSERT_NE(nullptr, obj) << "Creation of SubClassDefault failed";
    
    SubClassDefault* derived = dynamic_cast<SubClassDefault*>(obj);
    EXPECT_NE(nullptr, derived) << "Cast to SubClassDefault failed";
}

TEST_F(TemplateFactoryTest, CorrectnessDeepSubClass) {
    factory->registerClass<SubClassMultipleLayers>("SubClassMultipleLayers");
    
    BaseClass* obj = factory->create("SubClassMultipleLayers");
    ASSERT_NE(nullptr, obj) << "Creation of SubClassMultipleLayers failed";
    
    SubClassMultipleLayers* derived = dynamic_cast<SubClassMultipleLayers*>(obj);
    EXPECT_NE(nullptr, derived) << "Cast to SubClassMultipleLayers failed";
}

TEST_F(TemplateFactoryTest, NonInterference) {
    factory->registerClass<SubClassDefault>("SubClassDefault");
    factory->registerClass<SubClassDefault2>("SubClassDefault2");

    BaseClass* obj = factory->create("SubClassDefault");
    ASSERT_NE(nullptr, obj) << "Creation of SubClassDefault failed";
    BaseClass* obj2 = factory->create("SubClassDefault2");
    ASSERT_NE(nullptr, obj2) << "Creation of SubClassDefault2 failed";
    ASSERT_NE(obj, obj2) << "Pointer addresses were equal";

    SubClassDefault* derived = dynamic_cast<SubClassDefault*>(obj);
    EXPECT_NE(nullptr, derived) << "Cast to SubClassDefault failed";

    SubClassDefault2* derived2 = dynamic_cast<SubClassDefault2*>(obj2);
    EXPECT_NE(nullptr, derived2) << "Cast to SubClassDefault2 failed";
}

TEST_F(TemplateFactoryTest, DefaultConstructor) {
    factory->registerClass<SubClassDefault>("SubClassDefault");

    BaseClass* obj = factory->create("SubClassDefault");
    ASSERT_NE(nullptr, obj) << "Creation of SubClassDefault failed";

    EXPECT_EQ(1, obj->value1);
    EXPECT_EQ(2, obj->value2);
}

TEST_F(TemplateFactoryTest, NoDefaultConstructorExists) {
    factory->registerClass<SubClassDictionary>("SubClassDictionary");

    BaseClass* obj = factory->create("SubClassDictionary");
    ASSERT_NE(nullptr, obj) << "Creation of SubClassDictionary failed";

    EXPECT_EQ(-1, obj->value1) << "Value1 was modified";
    EXPECT_EQ(-2, obj->value2) << "Value2 was modified";
}

TEST_F(TemplateFactoryTest, DictionaryConstructor) {
    factory->registerClass<SubClassDefault>("SubClassDefault");

    ghoul::Dictionary dict = { { "value1", 100 }, { "value2", 200 } };
    BaseClass* obj = factory->create("SubClassDefault", dict);
    ASSERT_NE(nullptr, obj) << "Creation of SubClassDefault failed";

    EXPECT_EQ(1, obj->value1) << "Value1 was modified";
    EXPECT_EQ(2, obj->value2) << "Value2 was modified";
}

TEST_F(TemplateFactoryTest, NoDictionaryConstructorExists) {
    factory->registerClass<SubClassDefault>("SubClassDefault");

    ghoul::Dictionary dict = { { "value1", 100 }, { "value2", 200 } };
    BaseClass* obj = factory->create("SubClassDefault", dict);
    ASSERT_NE(nullptr, obj) << "Creation of SubClassDefault failed";

    EXPECT_EQ(1, obj->value1) << "Value1 was not modified";
    EXPECT_EQ(2, obj->value2) << "Value2 was not modified";
}

TEST_F(TemplateFactoryTest, ClassDoesNotExist) {
    factory->registerClass<SubClassDictionary>("SubClassDictionary");

    BaseClass* obj = factory->create("SubClassDictionary");
    ASSERT_NE(nullptr, obj) << "Creation of SubClassDictionary failed";

    BaseClass* obj2 = factory->create("DoesNotExist");
    EXPECT_EQ(nullptr, obj2);
}

TEST_F(TemplateFactoryTest, DefaultDictionaryConstructor) {
    //SubClassDefaultDictionary 31 32
    factory->registerClass<SubClassDefaultDictionary>("class");

    BaseClass* obj = factory->create("class");
    ASSERT_NE(nullptr, obj) << "Creation of SubClassDefaultDictionary failed";
    EXPECT_EQ(31, obj->value1);
    EXPECT_EQ(32, obj->value2);

    ghoul::Dictionary dict = { { "value1", 41 }, { "value2", 42 } };
    BaseClass* obj2 = factory->create("class", dict);
    ASSERT_NE(nullptr, obj2) << "Creation of SubClassDefaultDictionary failed";
    EXPECT_EQ(41, obj2->value1);
    EXPECT_EQ(42, obj2->value2);
}

TEST_F(TemplateFactoryTest, CorrectnessForHasClass) {
    factory->registerClass<SubClassDictionary>("SubClassDictionary");

    const bool subClassDirectoryExists = factory->hasClass("SubClassDictionary");
    const bool doesNotExistExists = factory->hasClass("DoesNotExist");

    EXPECT_EQ(subClassDirectoryExists, true);
    EXPECT_EQ(doesNotExistExists, false);
}

TEST_F(TemplateFactoryTest, FunctionPointerConstruction) {
    factory->registerClass("ptr", &createFunctionPointerClass);

    BaseClass* obj = factory->create("ptr");
    EXPECT_EQ(nullptr, obj) << "'useDictionary' was passed wrongly";

    BaseClass* obj2 = factory->create("ptr", {});
    EXPECT_NE(nullptr, obj2) << "'useDictionary' was passed wrongly";
}

TEST_F(TemplateFactoryTest, StdFunctionConstruction) {
    std::function<BaseClass*(bool, const ghoul::Dictionary&)> function =
        [](bool use, const ghoul::Dictionary&) {
        if (use)
            return new StdFunctionClass;
        else
            return reinterpret_cast<StdFunctionClass*>(nullptr);
    };
    factory->registerClass("ptr", function);

    BaseClass* obj = factory->create("ptr");
    EXPECT_EQ(nullptr, obj) << "'useDictionary' was passed wrongly";

    BaseClass* obj2 = factory->create("ptr", {});
    EXPECT_NE(nullptr, obj2) << "'useDictionary' was passed wrongly";
}
