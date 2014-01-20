/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#include "gtest\gtest.h"
#include <ghoul/misc/configurationmanager.h>
#include <glm/glm.hpp>
#include <random>

namespace {
    // A non-existing configuration file
    const std::string _configuration0 = "${TEST_DIR}/configurationmanager/test0.cfg";

    // The configuration1 test configuration has one key "t" = 1
    const std::string _configuration1 = "${TEST_DIR}/configurationmanager/test1.cfg";

    // The configuration1 test configuration has two keys "t" and "s"
    const std::string _configuration2 = "${TEST_DIR}/configurationmanager/test2.cfg";

    // More complicated configuration file with nested tables
    const std::string _configuration3 = "${TEST_DIR}/configurationmanager/test3.cfg";
}

/*
Test checklist:
+++ loadConfiguration: existing file
+++ loadConfiguration: non-existing file
+++ getValue: key does not exist
+++ getValue: subtable does not exist
+++ getValue: overriding previous configuration
+++ getValue: function does not change passed value on error
+++ getValue: nested keys
--- getValue: deep nesting of keys
--- getValue: correct values returned for each type
--- getValue: all basic types implemented
--- getValue: glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4 implemented
--- setValue: all types implemented
--- setValue: create subtables on the way
--- setValue: value gets set correctly for each type
--- setValue: value overwrites setting in configuration file
--- setValue: deep nesting of keys
--- setValue: nested keys
--- setValue: glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4 implemented
--- hasKeys: deep nesting of keys
--- hasKeys: subtables on the way do not exist
--- hasKeys: correct values for all types
+++ hasKeys: nestedKeys
*/

class ConfigurationManagerTest : public testing::Test {
protected:
    ConfigurationManagerTest() {
        _m = new ghoul::ConfigurationManager;
        _m->initialize();
    }

    ~ConfigurationManagerTest() {
        delete _m;
        _m = nullptr;
    }

    ghoul::ConfigurationManager* _m;
};

TEST_F(ConfigurationManagerTest, DeinitDeath) {
    // Accessing the ConfigurationManager after it has been deinitialized gets an assert
    _m->deinitialize();
    EXPECT_DEATH(_m->keys();, "");
}

TEST_F(ConfigurationManagerTest, LoadConfigurationTest) {
    const bool success0 = _m->loadConfiguration(_configuration0);
    ASSERT_EQ(success0, false) << "Loading a non-existing file should fail gracefully";
    const bool success1 = _m->loadConfiguration(_configuration1);
    ASSERT_EQ(success1, true) << "Loading of configuration file 'test1.cfg'";
    const bool success2 = _m->loadConfiguration(_configuration2);
    ASSERT_EQ(success2, true) << "Loading of configuration file 'test2.cfg'";
    const bool success3 = _m->loadConfiguration(_configuration3);
    ASSERT_EQ(success3, true) << "Loading of configuration file 'test3.cfg'";
}

TEST_F(ConfigurationManagerTest, KeysFunction) {
    // The empty configuration should not have any keys
    size_t nKeys = _m->keys().size();
    EXPECT_EQ(nKeys, 0) << "The empty configuration should not have any keys";

    _m->loadConfiguration(_configuration1);
    nKeys = _m->keys().size();
    EXPECT_EQ(nKeys, 1) << "test1";

    _m->loadConfiguration(_configuration3);
    nKeys = _m->keys().size();
    EXPECT_EQ(nKeys, 2) << "base: test1 + test3";

    nKeys = _m->keys("s").size();
    EXPECT_EQ(nKeys, 3) << "s: test1 + test3";

    nKeys = _m->keys("s.3").size();
    EXPECT_EQ(nKeys, 2) << "s.3: test1 + test3";
}

TEST_F(ConfigurationManagerTest, GetValueFunction) {
    bool test;
    bool success = _m->getValue("key", test);
    EXPECT_EQ(success, false) << "Empty configuration";

    success = _m->getValue("key.key", test);
    EXPECT_EQ(success, false) << "Empty configuration recursive";

    _m->loadConfiguration(_configuration1);
    _m->loadConfiguration(_configuration3);
    int testInt;
    success = _m->getValue("t", testInt);
    EXPECT_EQ(success, true) << "test1+test3 (t)";
    EXPECT_EQ(testInt, 1) << "test1+test3 (t)";

    success = _m->getValue("s.a", test);
    EXPECT_EQ(success, false) << "test1+test3 (s.a)";

    success = _m->getValue("s.1", test);
    EXPECT_EQ(success, true) << "test1+test3 (s.1)";

    success = _m->getValue("s.1.a", test);
    EXPECT_EQ(success, false) << "test1+test3 (s.1.a)";

    success = _m->getValue("s.3.a", test);
    EXPECT_EQ(success, true) << "test1+test3 (s.3.a)";

    success = _m->getValue("s.1", testInt);
    EXPECT_EQ(success, true) << "test1+test3 (s.1)";
    EXPECT_EQ(testInt, 1) << "test1+test3 (s.1)";

    success = _m->getValue("s.2", testInt);
    EXPECT_EQ(success, true) << "test1+test3 (s.2)";
    EXPECT_EQ(testInt, 2) << "test1+test3 (s.2)";

    std::vector<int> testVec;
    success = _m->getValue("key", testVec);
    EXPECT_EQ(success, false) << "test1+test3: Vector access";

}

TEST_F(ConfigurationManagerTest, InvalidKeyAccessInvariant) {
    // Accessing an invalid key should not change the tested argument
    std::mt19937 rd;
    {
        std::uniform_int_distribution<int> dist;
        for (int i = 0; i < 10; ++i) {
            const int testValue = dist(rd);
            int test = testValue;
            _m->getValue("key", test);
            ASSERT_EQ(test, testValue) << "invariant int";
        }
    }

    {
        std::uniform_real_distribution<float> dist;
        for (int i = 0; i < 10; ++i) {
            const float testValue = dist(rd);
            float test = testValue;
            _m->getValue("key", test);
            ASSERT_EQ(test, testValue) << "invariant float";
        }
    }
}

TEST_F(ConfigurationManagerTest, HasKeyFunction) {
    bool success = _m->hasKey("key");
    EXPECT_EQ(success, false) << "empty configuration";

    _m->loadConfiguration(_configuration1);
    success = _m->hasKey("t");
    EXPECT_EQ(success, true) << "test1 (t)";

    success = _m->hasKey("s");
    EXPECT_EQ(success, false) << "test1 (s)";

    _m->loadConfiguration(_configuration2);
    success = _m->hasKey("s.a");
    EXPECT_EQ(success, true) << "test1+test2 (s.a)";

    success = _m->hasKey("s.c");
    EXPECT_EQ(success, false) << "test1+test2 (s.c)";
}


TEST_F(ConfigurationManagerTest, MultipleKeyLoadOverwrite) {
    _m->loadConfiguration(_configuration1);
    int value;
    _m->getValue("t", value);
    EXPECT_EQ(value, 1);

    _m->loadConfiguration(_configuration2);

    // configuration2 should overwrite the value t in configuration1
    _m->getValue("t", value);
    EXPECT_EQ(value, 2);
}
