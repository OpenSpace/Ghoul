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

    // Deeply nested configuration file with 12 level
    const std::string _configuration4 = "${TEST_DIR}/configurationmanager/test4.cfg";

    // Testfile with glm::vecX, glm::matX
    const std::string _configuration5 = "${TEST_DIR}/configurationmanager/test5.cfg";
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
+++ getValue: deep nesting of keys
+++ getValue: correct values returned for each type
+++ getValue: are all basic types implemented
+++ getValue: glm::vec2, glm::vec3, glm::vec4 implemented
+++ getValue: valid conversions
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
    const bool success4 = _m->loadConfiguration(_configuration4);
    ASSERT_EQ(success4, true) << "Loading of configuration file 'test4.cfg'";
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
    EXPECT_EQ(nKeys, 3) << "base: test1 + test3";

    nKeys = _m->keys("s").size();
    EXPECT_EQ(nKeys, 3) << "s: test1 + test3";

    nKeys = _m->keys("s.3").size();
    EXPECT_EQ(nKeys, 2) << "s.3: test1 + test3";

    _m->loadConfiguration(_configuration4);

    const char* keys[] = {
        "a", "a.a", "a.a.a", "a.a.a.a", "a.a.a.a.a", "a.a.a.a.a.a", "a.a.a.a.a.a.a",
        "a.a.a.a.a.a.a.a", "a.a.a.a.a.a.a.a.a", "a.a.a.a.a.a.a.a.a.a",
        "a.a.a.a.a.a.a.a.a.a.a", "a.a.a.a.a.a.a.a.a.a.a.a"
    };

    for (int i = 0; i < 12; ++i) {
        nKeys = _m->keys(keys[i]).size();
        EXPECT_EQ(nKeys, 2) << keys[i] <<": test1 + test3";
    }

    //nKeys = _
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

    success = _m->getValue("s[\"1\"]", test);
    EXPECT_EQ(success, true) << "test1+test3 (s.1)";

    success = _m->getValue("s[\"1\"].a", test);
    EXPECT_EQ(success, false) << "test1+test3 (s.1.a)";

    success = _m->getValue("s[\"3\"].a", test);
    EXPECT_EQ(success, true) << "test1+test3 (s.3.a)";

    success = _m->getValue("s[\"1\"]", testInt);
    EXPECT_EQ(success, true) << "test1+test3 (s.1)";
    EXPECT_EQ(testInt, 1) << "test1+test3 (s.1)";

    success = _m->getValue("s[\"2\"]", testInt);
    EXPECT_EQ(success, true) << "test1+test3 (s.2)";
    EXPECT_EQ(testInt, 2) << "test1+test3 (s.2)";

    std::vector<int> testVec;
    success = _m->getValue("key", testVec);
    EXPECT_EQ(success, false) << "test1+test3: Vector access";
}

template <class T>
void correctnessHelperGetValue(ghoul::ConfigurationManager* m, const std::string& key) {
    T value = T(0);
    const bool success = m->getValue(key, value);
    EXPECT_EQ(success, true) << "Type: " << typeid(T).name();
    EXPECT_EQ(value, T(1)) << "Type: " << typeid(T).name();
}

TEST_F(ConfigurationManagerTest, GetValueCorrectness) {
    _m->loadConfiguration(_configuration1);

    correctnessHelperGetValue<bool>(_m, "t");
    correctnessHelperGetValue<char>(_m, "t");
    correctnessHelperGetValue<signed char>(_m, "t");
    correctnessHelperGetValue<unsigned char>(_m, "t");
    correctnessHelperGetValue<wchar_t>(_m, "t");
    correctnessHelperGetValue<short>(_m, "t");
    correctnessHelperGetValue<unsigned short>(_m, "t");
    correctnessHelperGetValue<int>(_m, "t");
    correctnessHelperGetValue<unsigned int>(_m, "t");
    correctnessHelperGetValue<long>(_m, "t");
    correctnessHelperGetValue<unsigned long>(_m, "t");
    correctnessHelperGetValue<long long>(_m, "t");
    correctnessHelperGetValue<unsigned long long>(_m, "t");
    correctnessHelperGetValue<float>(_m, "t");
    correctnessHelperGetValue<double>(_m, "t");
    correctnessHelperGetValue<long double>(_m, "t");

    std::string value;
    const bool success = _m->getValue("t", value);
    EXPECT_EQ(success, true) << "Type: " << typeid(std::string).name();
    EXPECT_STREQ(value.c_str(), "1") << "Type: " << typeid(std::string).name();
}

TEST_F(ConfigurationManagerTest, GetValueConversions) {
    // converting from 1 -> all types is done in GetValueCorrectness
    _m->loadConfiguration(_configuration2);

    correctnessHelperGetValue<bool>(_m, "s.a1");
    correctnessHelperGetValue<char>(_m, "s.a1");
    correctnessHelperGetValue<signed char>(_m, "s.a1");
    correctnessHelperGetValue<unsigned char>(_m, "s.a1");
    correctnessHelperGetValue<wchar_t>(_m, "s.a1");
    correctnessHelperGetValue<short>(_m, "s.a1");
    correctnessHelperGetValue<unsigned short>(_m, "s.a1");
    correctnessHelperGetValue<int>(_m, "s.a1");
    correctnessHelperGetValue<unsigned int>(_m, "s.a1");
    correctnessHelperGetValue<long>(_m, "s.a1");
    correctnessHelperGetValue<unsigned long>(_m, "s.a1");
    correctnessHelperGetValue<long long>(_m, "s.a1");
    correctnessHelperGetValue<unsigned long long>(_m, "s.a1");
    correctnessHelperGetValue<float>(_m, "s.a1");
    correctnessHelperGetValue<double>(_m, "s.a1");
    correctnessHelperGetValue<long double>(_m, "s.a1");

    std::string value;
    const bool success = _m->getValue("s.a1", value);
    EXPECT_EQ(success, true) << "Type: " << typeid(std::string).name();
    EXPECT_STREQ(value.c_str(), "1") << "Type: " << typeid(std::string).name();
}

TEST_F(ConfigurationManagerTest, StringKeyVsIntKey) {
    _m->loadConfiguration(_configuration3);

    int v = 0;
     bool success = _m->getValue("tt[\"1\"]", v);
    ASSERT_EQ(success, true) << "tt.1";
    EXPECT_EQ(v, 2) << "tt.1";

    success = _m->getValue("tt[1]", v);
    ASSERT_EQ(success, true) << "tt[1]";
    EXPECT_EQ(v, 1) << "tt[1]";
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

template <class T>
void vectorClassHelper(ghoul::ConfigurationManager* m, const std::string& key) {
    T value = T(0);
    const bool success = m->getValue(key, value);
    EXPECT_EQ(success, true) << "Type: " << typeid(T).name() << " | Key: " << key;
    EXPECT_EQ(value, T(glm::vec4(5, 6, 7, 8))) << "Type: " << typeid(T).name()  <<
        " | Key: " << key;
}

template <>
void vectorClassHelper<glm::bvec2>(ghoul::ConfigurationManager* m, const std::string& key) {
    glm::bvec2 value = glm::bvec2(false);
    const bool success = m->getValue(key, value);
    EXPECT_EQ(success, true) << "Type: bvec2 | Key: " << key;
    EXPECT_EQ(value.x, true) << "Type: bvec2 | Key: " << key;
    EXPECT_EQ(value.y, true) << "Type: bvec2 | Key: " << key;
}

template <>
void vectorClassHelper<glm::bvec3>(ghoul::ConfigurationManager* m, const std::string& key) {
    glm::bvec3 value = glm::bvec3(false);
    const bool success = m->getValue(key, value);
    EXPECT_EQ(success, true) << "Type: bvec3 | Key: " << key;
    EXPECT_EQ(value.x, true) << "Type: bvec3 | Key: " << key;
    EXPECT_EQ(value.y, true) << "Type: bvec3 | Key: " << key;
    EXPECT_EQ(value.z, true) << "Type: bvec3 | Key: " << key;
}

template <>
void vectorClassHelper<glm::bvec4>(ghoul::ConfigurationManager* m, const std::string& key) {
    glm::bvec4 value = glm::bvec4(false);
    const bool success = m->getValue(key, value);
    EXPECT_EQ(success, true) << "Type: bvec4 | Key: " << key;
    EXPECT_EQ(value.x, true) << "Type: bvec4 | Key: " << key;
    EXPECT_EQ(value.y, true) << "Type: bvec4 | Key: " << key;
    EXPECT_EQ(value.z, true) << "Type: bvec4 | Key: " << key;
    EXPECT_EQ(value.w, true) << "Type: bvec4 | Key: " << key;
}

TEST_F(ConfigurationManagerTest, VectorClasses) {
    _m->loadConfiguration(_configuration5);
    vectorClassHelper<glm::vec2>(_m, "n2");
    vectorClassHelper<glm::vec2>(_m, "num2");
    vectorClassHelper<glm::vec2>(_m, "xy");
    vectorClassHelper<glm::vec2>(_m, "rg");
    vectorClassHelper<glm::vec2>(_m, "st");
    vectorClassHelper<glm::dvec2>(_m, "n2");
    vectorClassHelper<glm::dvec2>(_m, "num2");
    vectorClassHelper<glm::dvec2>(_m, "xy");
    vectorClassHelper<glm::dvec2>(_m, "rg");
    vectorClassHelper<glm::dvec2>(_m, "st");
    vectorClassHelper<glm::ivec2>(_m, "n2");
    vectorClassHelper<glm::ivec2>(_m, "num2");
    vectorClassHelper<glm::ivec2>(_m, "xy");
    vectorClassHelper<glm::ivec2>(_m, "rg");
    vectorClassHelper<glm::ivec2>(_m, "st");
    vectorClassHelper<glm::uvec2>(_m, "n2");
    vectorClassHelper<glm::uvec2>(_m, "num2");
    vectorClassHelper<glm::uvec2>(_m, "xy");
    vectorClassHelper<glm::uvec2>(_m, "rg");
    vectorClassHelper<glm::uvec2>(_m, "st");
    vectorClassHelper<glm::bvec2>(_m, "n2");
    vectorClassHelper<glm::bvec2>(_m, "num2");
    vectorClassHelper<glm::bvec2>(_m, "xy");
    vectorClassHelper<glm::bvec2>(_m, "rg");
    vectorClassHelper<glm::bvec2>(_m, "st");

    vectorClassHelper<glm::vec3>(_m, "n3");
    vectorClassHelper<glm::vec3>(_m, "num3");
    vectorClassHelper<glm::vec3>(_m, "xyz");
    vectorClassHelper<glm::vec3>(_m, "rgb");
    vectorClassHelper<glm::vec3>(_m, "stp");
    vectorClassHelper<glm::dvec3>(_m, "n3");
    vectorClassHelper<glm::dvec3>(_m, "num3");
    vectorClassHelper<glm::dvec3>(_m, "xyz");
    vectorClassHelper<glm::dvec3>(_m, "rgb");
    vectorClassHelper<glm::dvec3>(_m, "stp");
    vectorClassHelper<glm::ivec3>(_m, "n3");
    vectorClassHelper<glm::ivec3>(_m, "num3");
    vectorClassHelper<glm::ivec3>(_m, "xyz");
    vectorClassHelper<glm::ivec3>(_m, "rgb");
    vectorClassHelper<glm::ivec3>(_m, "stp");
    vectorClassHelper<glm::uvec3>(_m, "n3");
    vectorClassHelper<glm::uvec3>(_m, "num3");
    vectorClassHelper<glm::uvec3>(_m, "xyz");
    vectorClassHelper<glm::uvec3>(_m, "rgb");
    vectorClassHelper<glm::uvec3>(_m, "stp");
    vectorClassHelper<glm::bvec3>(_m, "n3");
    vectorClassHelper<glm::bvec3>(_m, "num3");
    vectorClassHelper<glm::bvec3>(_m, "xyz");
    vectorClassHelper<glm::bvec3>(_m, "rgb");
    vectorClassHelper<glm::bvec3>(_m, "stp");

    vectorClassHelper<glm::vec4>(_m, "n4");
    vectorClassHelper<glm::vec4>(_m, "num4");
    vectorClassHelper<glm::vec4>(_m, "xyzw");
    vectorClassHelper<glm::vec4>(_m, "rgba");
    vectorClassHelper<glm::vec4>(_m, "stpq");
    vectorClassHelper<glm::dvec4>(_m, "n4");
    vectorClassHelper<glm::dvec4>(_m, "num4");
    vectorClassHelper<glm::dvec4>(_m, "xyzw");
    vectorClassHelper<glm::dvec4>(_m, "rgba");
    vectorClassHelper<glm::dvec4>(_m, "stpq");
    vectorClassHelper<glm::ivec4>(_m, "num4");
    vectorClassHelper<glm::ivec4>(_m, "n4");
    vectorClassHelper<glm::ivec4>(_m, "xyzw");
    vectorClassHelper<glm::ivec4>(_m, "rgba");
    vectorClassHelper<glm::ivec4>(_m, "stpq");
    vectorClassHelper<glm::uvec4>(_m, "num4");
    vectorClassHelper<glm::uvec4>(_m, "n4");
    vectorClassHelper<glm::uvec4>(_m, "xyzw");
    vectorClassHelper<glm::uvec4>(_m, "rgba");
    vectorClassHelper<glm::uvec4>(_m, "stpq");
    vectorClassHelper<glm::bvec4>(_m, "num4");
    vectorClassHelper<glm::bvec4>(_m, "n4");
    vectorClassHelper<glm::bvec4>(_m, "xyzw");
    vectorClassHelper<glm::bvec4>(_m, "rgba");
    vectorClassHelper<glm::bvec4>(_m, "stpq");

    glm::mat4 f(1.0);
    glm::mat2 g = glm::mat2(f);

    glm::vec3 value = glm::vec3(0.f);
    const bool success = _m->getValue("mix", value);
    EXPECT_EQ(success, false) << "Type: mixed";
    EXPECT_EQ(value, glm::vec3(0.f)) << "Type: mixed";
}

template <class T, typename U>
void matrixClassHelper(ghoul::ConfigurationManager* m, const std::string& key) {
    T value = T(0);
    const bool success = m->getValue(key, value);
    EXPECT_EQ(success, true) << "Type: " << typeid(T).name();

    const glm::detail::tmat4x4<U> res4 = glm::detail::tmat4x4<U>(
        5.0,  6.0, 7.0, 8.0,
        9.0, 10.0,11.0,12.0,
        13.0,14.0,15.0,16.0,
        17.0,18.0,19.0,20.0
        );

    const T res = T(res4);
    
    EXPECT_EQ(value, res) << "Type: " << typeid(T).name();
}

TEST_F(ConfigurationManagerTest, MatrixClasses) {
    _m->loadConfiguration(_configuration5);
    matrixClassHelper<glm::mat2x2, float>(_m, "m2x2");
    matrixClassHelper<glm::mat2x3, float>(_m, "m2x3");
    matrixClassHelper<glm::mat2x4, float>(_m, "m2x4");
    matrixClassHelper<glm::mat3x2, float>(_m, "m3x2");
    matrixClassHelper<glm::mat3x3, float>(_m, "m3x3");
    matrixClassHelper<glm::mat3x4, float>(_m, "m3x4");
    matrixClassHelper<glm::mat4x2, float>(_m, "m4x2");
    matrixClassHelper<glm::mat4x3, float>(_m, "m4x3");
    matrixClassHelper<glm::mat4x4, float>(_m, "m4x4");

    matrixClassHelper<glm::dmat2x2, double>(_m, "m2x2");
    matrixClassHelper<glm::dmat2x3, double>(_m, "m2x3");
    matrixClassHelper<glm::dmat2x4, double>(_m, "m2x4");
    matrixClassHelper<glm::dmat3x2, double>(_m, "m3x2");
    matrixClassHelper<glm::dmat3x3, double>(_m, "m3x3");
    matrixClassHelper<glm::dmat3x4, double>(_m, "m3x4");
    matrixClassHelper<glm::dmat4x2, double>(_m, "m4x2");
    matrixClassHelper<glm::dmat4x3, double>(_m, "m4x3");
    matrixClassHelper<glm::dmat4x4, double>(_m, "m4x4");
}

