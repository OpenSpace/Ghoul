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
#include <ghoul/glm.h>
#include <fstream>
#include <sstream>

/*
Test checklist:
+++ getValue
+++  basic types
+++  advanced types
+++ setValues
+++  basic types
+++  advanced types
+++ nested dictionaries
+++ timing
*/

class DictionaryTest : public testing::Test {
protected:
	DictionaryTest() {
        _d = new ghoul::Dictionary;
    }

	~DictionaryTest() {
        if (_d) {
            delete _d;
            _d = nullptr;
        }
    }

    void reset() {
        delete _d;
        _d = new ghoul::Dictionary;
    }

    void createDefaultDictionary() {
        ghoul::Dictionary dict;
        delete _d;
        _d = new ghoul::Dictionary(
              {{"bool", true},
               {"char", char(1)},
               {"signed char", static_cast<signed char>(1)},
               {"unsigned char", static_cast<unsigned char>(1)},
               {"wchar_t", wchar_t(1)},
               {"short", short(1)},
               {"unsigned short", static_cast<unsigned short>(1)},
               {"int", int(1)},
               {"unsigned int", static_cast<unsigned int>(1)},
               {"long long", static_cast<long long>(1)},
               {"unsigned long long", static_cast<unsigned long long>(1)},
               {"float", float(1)},
               {"double", double(1)},
               {"long double", static_cast<long double>(1)},
               {"dictionary", dict},
               {"vec2", glm::vec2(1.f, 2.f)},
               {"dvec2", glm::dvec2(1.0, 2.0)},
               {"ivec2", glm::ivec2(1, 2)},
               {"uvec2", glm::uvec2(1, 2)},
               {"bvec2", glm::bvec2(true, false)},
               {"vec3", glm::vec3(1.f, 2.f, 3.f)},
               {"dvec3", glm::dvec3(1.0, 2.0, 3.0)},
               {"ivec3", glm::ivec3(1, 2, 3)},
               {"uvec3", glm::uvec3(1, 2, 3)},
               {"bvec3", glm::bvec3(true, false, true)},
               {"vec4", glm::vec4(1.f, 2.f, 3.f, 4.f)},
               {"dvec4", glm::dvec4(1.0, 2.0, 3.0, 4.0)},
               {"ivec4", glm::ivec4(1, 2, 3, 4)},
               {"uvec4", glm::uvec4(1, 2, 3, 4)},
               {"bvec4", glm::bvec4(true, false, true, false)},
               {"mat2x2", glm::mat2x2(1.f, 2.f, 3.f, 4.f)},
               {"mat2x3", glm::mat2x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f)},
               {"mat2x4", glm::mat2x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f)},
               {"mat3x2", glm::mat3x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f)},
               {"mat3x3", glm::mat3x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f)},
               {"mat3x4", glm::mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f,
                                      11.f, 12.f)},
               {"mat4x2", glm::mat4x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f)},
               {"mat4x3", glm::mat4x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f,
                                      11.f, 12.f)},
               {"mat4x4", glm::mat4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f,
                                      11.f, 12.f, 13.f, 14.f, 15.f, 16.f)},
               {"dmat2x2", glm::dmat2x2(1.0, 2.0, 3.0, 4.0)},
               {"dmat2x3", glm::dmat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0)},
               {"dmat2x4", glm::dmat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0)},
               {"dmat3x2", glm::dmat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0)},
               {"dmat3x3", glm::dmat3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0)},
               {"dmat3x4", glm::dmat3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                                        11.0, 12.0)},
               {"dmat4x2", glm::dmat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0)},
               {"dmat4x3", glm::dmat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                                        11.0, 12.0)},
               {"dmat4x4", glm::dmat4x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
                                        11.0, 12.0, 13.0, 14.0, 15.0, 16.0)}});
    }

    ghoul::Dictionary* _d;
};

#ifdef GHL_TIMING_TESTS

TEST_F(DictionaryTest, TimingTest) {
	std::ofstream logFile("DictionaryTest.timing");

	START_TIMER(setValue, logFile, 25);
	_d->setValue("a", 1);
	FINISH_TIMER(setValue, logFile);

	int value;
	START_TIMER_PREPARE(getValueCorrect, logFile, 25, { _d->setValue("a", 1); });
	_d->getValue("a", value);
	FINISH_TIMER(getValueCorrect, logFile);

	START_TIMER_PREPARE(getValueWrong, logFile, 25, { _d->setValue("a", 1); });
	_d->getValue("b", value);
	FINISH_TIMER(getValueWrong, logFile);

	START_TIMER_PREPARE(hasValueCorrect, logFile, 25, { _d->setValue("a", 1); });
	_d->hasValue<int>("a");
	FINISH_TIMER(hasValueCorrect, logFile);

	START_TIMER_PREPARE(hasValueWrong, logFile, 25, { _d->setValue("a", 1); });
	_d->hasValue<int>("b");
	FINISH_TIMER(hasValueWrong, logFile);

	START_TIMER(setValueNumber1, logFile, 25);
	_d->setValue("a", 1);
	FINISH_TIMER(setValueNumber1, logFile);

	START_TIMER_PREPARE(setValueNumber2, logFile, 25, { _d->setValue("a", 1); });
	_d->setValue("b", 1);
	FINISH_TIMER(setValueNumber2, logFile);

	START_TIMER_PREPARE(setValueNumber3, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); });
	_d->setValue("c", 1);
	FINISH_TIMER(setValueNumber3, logFile);

	START_TIMER_PREPARE(setValueNumber4, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1);	_d->setValue("c", 1); });
	_d->setValue("d", 1);
	FINISH_TIMER(setValueNumber4, logFile);

	START_TIMER_PREPARE(setValueNumber5, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1);
	_d->setValue("c", 1); _d->setValue("d", 1); });
	_d->setValue("e", 1);
	FINISH_TIMER(setValueNumber5, logFile);

	START_TIMER_PREPARE(setValueNumber6, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); _d->setValue("c", 1);
	_d->setValue("d", 1); _d->setValue("e", 1); });
	_d->setValue("f", 1);
	FINISH_TIMER(setValueNumber6, logFile);

	START_TIMER_PREPARE(setValueNumber7, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); _d->setValue("c", 1);
	_d->setValue("d", 1); _d->setValue("e", 1); _d->setValue("f", 1); });
	_d->setValue("g", 1);
	FINISH_TIMER(setValueNumber7, logFile);

	START_TIMER_PREPARE(setValueNumber8, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); _d->setValue("c", 1);
	_d->setValue("d", 1); _d->setValue("e", 1); _d->setValue("f", 1);
	_d->setValue("g", 1); });
	_d->setValue("h", 1);
	FINISH_TIMER(setValueNumber8, logFile);

	START_TIMER_PREPARE(setValueNumber9, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); _d->setValue("c", 1);
	_d->setValue("d", 1); _d->setValue("e", 1); _d->setValue("f", 1);
	_d->setValue("g", 1); _d->setValue("h", 1); });
	_d->setValue("i", 1);
	FINISH_TIMER(setValueNumber9, logFile);

	START_TIMER_PREPARE(setValueNumber10, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); _d->setValue("c", 1);
	_d->setValue("d", 1); _d->setValue("e", 1); _d->setValue("f", 1);
	_d->setValue("g", 1); _d->setValue("h", 1); _d->setValue("i", 1); });
	_d->setValue("j", 1);
	FINISH_TIMER(setValueNumber10, logFile);

	ghoul::Dictionary d100;
	std::stringstream s;
	for (size_t i = 0; i < 100; ++i) {
		s << "a" << i;
		d100.setValue(s.str(), 1);
		s.str(std::string());
	}

	ghoul::Dictionary d101 = d100;
	d101.setValue("a100", 1);

	ghoul::Dictionary d1000;
	for (size_t i = 0; i < 1000; ++i) {
		s << "a" << i;
		d1000.setValue(s.str(), 1);
		s.str(std::string());
	}

	ghoul::Dictionary d1001 = d1000;
	d1001.setValue("a1000", 1);


	START_TIMER_PREPARE(setValueNumber100, logFile, 25, { *_d = d100; });
	_d->setValue("b", 1);
	FINISH_TIMER(setValueNumber100, logFile);

	START_TIMER_PREPARE(setValueNumber1000, logFile, 25, { *_d = d1000; });
	_d->setValue("b", 1);
	FINISH_TIMER(setValueNumber1000, logFile);

	START_TIMER_PREPARE(getValueNumber1, logFile, 25, { _d->setValue("a", 1); });
	_d->getValue("a", value);
	FINISH_TIMER(getValueNumber1, logFile);

	START_TIMER_PREPARE(getValueNumber2, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); });
	_d->getValue("a", value);
	FINISH_TIMER(getValueNumber2, logFile);

	START_TIMER_PREPARE(getValueNumber3, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1);	_d->setValue("c", 1); });
	_d->getValue("a", value);
	FINISH_TIMER(getValueNumber3, logFile);

	START_TIMER_PREPARE(getValueNumber4, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1);
	_d->setValue("c", 1); _d->setValue("d", 1); });
	_d->getValue("a", value);
	FINISH_TIMER(getValueNumber4, logFile);

	START_TIMER_PREPARE(getValueNumber5, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); _d->setValue("c", 1);
	_d->setValue("d", 1); _d->setValue("e", 1); });
	_d->getValue("a", value);
	FINISH_TIMER(getValueNumber5, logFile);

	START_TIMER_PREPARE(getValueNumber6, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); _d->setValue("c", 1);
	_d->setValue("d", 1); _d->setValue("e", 1); _d->setValue("f", 1); });
	_d->getValue("a", value);
	FINISH_TIMER(getValueNumber6, logFile);

	START_TIMER_PREPARE(getValueNumber7, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); _d->setValue("c", 1);
	_d->setValue("d", 1); _d->setValue("e", 1); _d->setValue("f", 1);
	_d->setValue("g", 1); });
	_d->getValue("a", value);
	FINISH_TIMER(getValueNumber7, logFile);

	START_TIMER_PREPARE(getValueNumber8, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); _d->setValue("c", 1);
	_d->setValue("d", 1); _d->setValue("e", 1); _d->setValue("f", 1);
	_d->setValue("g", 1); _d->setValue("h", 1); });
	_d->getValue("a", value);
	FINISH_TIMER(getValueNumber8, logFile);

	START_TIMER_PREPARE(getValueNumber9, logFile, 25,
	{ _d->setValue("a", 1); _d->setValue("b", 1); _d->setValue("c", 1);
	_d->setValue("d", 1); _d->setValue("e", 1); _d->setValue("f", 1);
	_d->setValue("g", 1); _d->setValue("h", 1); _d->setValue("i", 1); });
	_d->getValue("a", value);
	FINISH_TIMER(getValueNumber9, logFile);
	
	START_TIMER_PREPARE(getValueNumber100a1, logFile, 25, { *_d = d101; });
	_d->getValue("a1", value);
	FINISH_TIMER(getValueNumber100a1, logFile);

	START_TIMER_PREPARE(getValueNumber1000a1, logFile, 25, { *_d = d1001; });
	_d->getValue("a1", value);
	FINISH_TIMER(getValueNumber1000a1, logFile);

	START_TIMER_PREPARE(getValueNumber100a50, logFile, 25, { *_d = d101; });
	_d->getValue("a50", value);
	FINISH_TIMER(getValueNumber100a50, logFile);

	START_TIMER_PREPARE(getValueNumber1000a50, logFile, 25, { *_d = d1001; });
	_d->getValue("a50", value);
	FINISH_TIMER(getValueNumber1000a50, logFile);

	START_TIMER_PREPARE(getValueNumber100a75, logFile, 25, { *_d = d101; });
	_d->getValue("a75", value);
	FINISH_TIMER(getValueNumber100a75, logFile);

	START_TIMER_PREPARE(getValueNumber1000a75, logFile, 25, { *_d = d1001; });
	_d->getValue("a75", value);
	FINISH_TIMER(getValueNumber1000a75, logFile);

	START_TIMER_PREPARE(getValueNumber1000a500, logFile, 25, { *_d = d1001; });
	_d->getValue("a500", value);
	FINISH_TIMER(getValueNumber1000a500, logFile);

	START_TIMER_PREPARE(getValueNumber1000a750, logFile, 25, { *_d = d1001; });
	_d->getValue("a750", value);
	FINISH_TIMER(getValueNumber1000a750, logFile);
}

#endif // GHL_TIMING_TESTS

TEST_F(DictionaryTest, EmptyTest) {
    EXPECT_EQ(0, _d->size());
}

TEST_F(DictionaryTest, ClearTest) {
    _d->setValue("a", 1);
    EXPECT_EQ(1, _d->size());
    _d->clear();
    EXPECT_EQ(0, _d->size());
}

TEST_F(DictionaryTest, InitializerLists) {
	ghoul::Dictionary d = { { "a", 1 } };
	ghoul::Dictionary d2 = { { "a", 1 }, { "b", 2 } };
	ghoul::Dictionary d3 = { { "a", 1 }, { "b", 2 }, { "c", 3 } };

	EXPECT_EQ(1, d.size());
	EXPECT_EQ(2, d2.size());
	EXPECT_EQ(3, d3.size());
}

TEST_F(DictionaryTest, NestedDictionaries) {
	ghoul::Dictionary d = { { "a", 1 } };
	ghoul::Dictionary e = { { "a", 1 }, { "b", d } };

	EXPECT_EQ(2, e.size());
}

TEST_F(DictionaryTest, AssignmentOperator) {
	ghoul::Dictionary d = { { "a", 1 }, { "b", 2 } };
	ghoul::Dictionary e = d;
	EXPECT_EQ(2, e.size());
	int value;
	const bool successA = d.getValue("a", value);
	EXPECT_EQ(true, successA) << "success 'a'";
	EXPECT_EQ(1, value) << "value 'a'";
	const bool successB = d.getValue("b", value);
	EXPECT_EQ(true, successB) << "success 'b'";
	EXPECT_EQ(2, value) << "value 'b'";
}

TEST_F(DictionaryTest, CopyConstructor) {
	ghoul::Dictionary d = { { "a", 1 }, { "b", 2 } };
	ghoul::Dictionary e(d);
	EXPECT_EQ(2, e.size());
	int value;
	const bool successA = d.getValue("a", value);
	EXPECT_EQ(true, successA) << "success 'a'";
	EXPECT_EQ(1, value) << "value 'a'";
	const bool successB = d.getValue("b", value);
	EXPECT_EQ(true, successB) << "success 'b'";
	EXPECT_EQ(2, value) << "value 'b'";
}

TEST_F(DictionaryTest, HasValue) {
	createDefaultDictionary();

	// correct types
	ASSERT_EQ(48, _d->size());
	EXPECT_EQ(true, _d->hasValue<bool>("bool")) << "bool";
	EXPECT_EQ(true, _d->hasValue<char>("char")) << "char";
	EXPECT_EQ(true, _d->hasValue<signed char>("signed char")) << "signed char";
	EXPECT_EQ(true, _d->hasValue<unsigned char>("unsigned char")) << "unsigned char";
	EXPECT_EQ(true, _d->hasValue<wchar_t>("wchar_t")) << "wchar_t";
	EXPECT_EQ(true, _d->hasValue<short>("short")) << "short";
	EXPECT_EQ(true, _d->hasValue<unsigned short>("unsigned short")) << "unsigned short";
	EXPECT_EQ(true, _d->hasValue<int>("int")) << "int";
	EXPECT_EQ(true, _d->hasValue<unsigned int>("unsigned int")) << "unsigned int";
	EXPECT_EQ(true, _d->hasValue<long long>("long long")) << "long long";
	EXPECT_EQ(true, _d->hasValue<unsigned long long>("unsigned long long")) <<
		"unsigned long long";
	EXPECT_EQ(true, _d->hasValue<float>("float")) << "float";
	EXPECT_EQ(true, _d->hasValue<double>("double")) << "double";
	EXPECT_EQ(true, _d->hasValue<long double>("long double")) << "long double";
	EXPECT_EQ(true, _d->hasValue<ghoul::Dictionary>("dictionary")) << "Dictionary";
	EXPECT_EQ(true, _d->hasValue<glm::vec2>("vec2")) << "vec2";
	EXPECT_EQ(true, _d->hasValue<glm::dvec2>("dvec2")) << "dvec2";
	EXPECT_EQ(true, _d->hasValue<glm::ivec2>("ivec2")) << "ivec2";
	EXPECT_EQ(true, _d->hasValue<glm::uvec2>("uvec2")) << "uvec2";
	EXPECT_EQ(true, _d->hasValue<glm::bvec2>("bvec2")) << "bvec2";
	EXPECT_EQ(true, _d->hasValue<glm::vec3>("vec3")) << "vec3";
	EXPECT_EQ(true, _d->hasValue<glm::dvec3>("dvec3")) << "dvec3";
	EXPECT_EQ(true, _d->hasValue<glm::ivec3>("ivec3")) << "ivec3";
	EXPECT_EQ(true, _d->hasValue<glm::uvec3>("uvec3")) << "uvec3";
	EXPECT_EQ(true, _d->hasValue<glm::bvec3>("bvec3")) << "bvec3";
	EXPECT_EQ(true, _d->hasValue<glm::vec4>("vec4")) << "vec4";
	EXPECT_EQ(true, _d->hasValue<glm::dvec4>("dvec4")) << "dvec4";
	EXPECT_EQ(true, _d->hasValue<glm::ivec4>("ivec4")) << "ivec4";
	EXPECT_EQ(true, _d->hasValue<glm::uvec4>("uvec4")) << "uvec4";
	EXPECT_EQ(true, _d->hasValue<glm::bvec4>("bvec4")) << "bvec4";
	EXPECT_EQ(true, _d->hasValue<glm::mat2x2>("mat2x2")) << "mat2x2";
	EXPECT_EQ(true, _d->hasValue<glm::mat2x3>("mat2x3")) << "mat2x3";
	EXPECT_EQ(true, _d->hasValue<glm::mat2x4>("mat2x4")) << "mat2x4";
	EXPECT_EQ(true, _d->hasValue<glm::mat3x2>("mat3x2")) << "mat3x2";
	EXPECT_EQ(true, _d->hasValue<glm::mat3x3>("mat3x3")) << "mat3x3";
	EXPECT_EQ(true, _d->hasValue<glm::mat3x4>("mat3x4")) << "mat3x4";
	EXPECT_EQ(true, _d->hasValue<glm::mat4x2>("mat4x2")) << "mat4x2";
	EXPECT_EQ(true, _d->hasValue<glm::mat4x3>("mat4x3")) << "mat4x3";
	EXPECT_EQ(true, _d->hasValue<glm::mat4x4>("mat4x4")) << "mat4x4";
	EXPECT_EQ(true, _d->hasValue<glm::dmat2x2>("dmat2x2")) << "dmat2x2";
	EXPECT_EQ(true, _d->hasValue<glm::dmat2x3>("dmat2x3")) << "dmat2x3";
	EXPECT_EQ(true, _d->hasValue<glm::dmat2x4>("dmat2x4")) << "dmat2x4";
	EXPECT_EQ(true, _d->hasValue<glm::dmat3x2>("dmat3x2")) << "dmat3x2";
	EXPECT_EQ(true, _d->hasValue<glm::dmat3x3>("dmat3x3")) << "dmat3x3";
	EXPECT_EQ(true, _d->hasValue<glm::dmat3x4>("dmat3x4")) << "dmat3x4";
	EXPECT_EQ(true, _d->hasValue<glm::dmat4x2>("dmat4x2")) << "dmat4x2";
	EXPECT_EQ(true, _d->hasValue<glm::dmat4x3>("dmat4x3")) << "dmat4x3";
	EXPECT_EQ(true, _d->hasValue<glm::dmat4x4>("dmat4x4")) << "dmat4x4";

	// false types
	EXPECT_EQ(false, _d->hasValue<int>("float")) << "int / float";

	// not existing
	EXPECT_EQ(false, _d->hasValue<bool>("nokey")) << "not existing key";
}

TEST_F(DictionaryTest, GetValue) {
    createDefaultDictionary();
    ASSERT_EQ(48, _d->size());
    {
        bool value;
        const bool success = _d->getValue("bool", value);
        EXPECT_EQ(true, success) << "success 'bool'";
        EXPECT_EQ(true, value) << "value 'bool'";
    }
    {
        char value;
        const bool success = _d->getValue("char", value);
        EXPECT_EQ(true, success) << "success 'char'";
        EXPECT_EQ(char(1), value) << "value 'char'";
    }
    {
        signed char value;
        const bool success = _d->getValue("signed char", value);
        EXPECT_EQ(true, success) << "success 'signed char'";
        EXPECT_EQ(static_cast<signed char>(1), value) << "value 'signed char'";
    }
    {
        unsigned char value;
        const bool success = _d->getValue("unsigned char", value);
        EXPECT_EQ(true, success) << "success 'unsigned char'";
        EXPECT_EQ(static_cast<unsigned char>(1), value) << "value 'unsigned char'";
    }
    {
        wchar_t value;
        const bool success = _d->getValue("wchar_t", value);
        EXPECT_EQ(true, success) << "success 'wchar_t'";
        EXPECT_EQ(wchar_t(1), value) << "value 'wchar_t'";
    }
    {
        short value;
        const bool success = _d->getValue("short", value);
        EXPECT_EQ(true, success) << "success 'short'";
        EXPECT_EQ(short(1), value) << "value 'short'";
    }
    {
        unsigned short value;
        const bool success = _d->getValue("unsigned short", value);
        EXPECT_EQ(true, success) << "success 'unsigned short'";
        EXPECT_EQ(static_cast<unsigned short>(1), value) << "value 'unsigned short'";
    }
    {
        int value;
        const bool success = _d->getValue("int", value);
        EXPECT_EQ(true, success) << "success 'int'";
        EXPECT_EQ(int(1), value) << "value 'int'";
    }
    {
        unsigned int value;
        const bool success = _d->getValue("unsigned int", value);
        EXPECT_EQ(true, success) << "success 'unsigned int'";
        EXPECT_EQ(static_cast<unsigned int>(1), value) << "value 'unsigned int'";
    }
    {
        long long value;
        const bool success = _d->getValue("long long", value);
        EXPECT_EQ(true, success) << "success 'long long'";
        EXPECT_EQ(static_cast<long long>(1), value) << "value 'long long'";
    }
    {
        unsigned long long value;
        const bool success = _d->getValue("unsigned long long", value);
        EXPECT_EQ(true, success) << "success 'unsigned long long'";
        EXPECT_EQ(static_cast<unsigned long long>(1), value)
              << "value 'unsigned long long'";
    }
    {
        float value;
        const bool success = _d->getValue("float", value);
        EXPECT_EQ(true, success) << "success 'float'";
        EXPECT_EQ(float(1), value) << "value 'float'";
    }
    {
        double value;
        const bool success = _d->getValue("double", value);
        EXPECT_EQ(true, success) << "success 'double'";
        EXPECT_EQ(double(1), value) << "value 'double'";
    }
    {
        long double value;
        const bool success = _d->getValue("long double", value);
        EXPECT_EQ(true, success) << "success 'long double'";
        EXPECT_EQ(static_cast<long double>(1), value) << "value 'long double'";
    }
    {
        glm::vec2 value;
        const bool success = _d->getValue("vec2", value);
        EXPECT_EQ(true, success) << "success 'glm::vec2'";
        EXPECT_EQ(glm::vec2(1.f, 2.f), value) << "value 'glm::vec2'";
    }
    {
        glm::dvec2 value;
        const bool success = _d->getValue("dvec2", value);
        EXPECT_EQ(true, success) << "success 'glm::dvec2'";
        EXPECT_EQ(glm::dvec2(1.0, 2.0), value) << "value 'glm::dvec2'";
    }
    {
        glm::ivec2 value;
        const bool success = _d->getValue("ivec2", value);
        EXPECT_EQ(true, success) << "success 'glm::ivec2'";
        EXPECT_EQ(glm::ivec2(1, 2), value) << "value 'glm::ivec2'";
    }
    {
        glm::uvec2 value;
        const bool success = _d->getValue("uvec2", value);
        EXPECT_EQ(true, success) << "success 'glm::uvec2'";
        EXPECT_EQ(glm::uvec2(1, 2), value) << "value 'glm::uvec2'";
    }
    {
        glm::bvec2 value;
        const bool success = _d->getValue("bvec2", value);
        EXPECT_EQ(true, success) << "success 'glm::bvec2'";
        EXPECT_EQ(glm::bvec2(true, false), value) << "value 'glm::bvec2'";
    }
    {
        glm::vec3 value;
        const bool success = _d->getValue("vec3", value);
        EXPECT_EQ(true, success) << "success 'glm::vec3'";
        EXPECT_EQ(glm::vec3(1.f, 2.f, 3.f), value) << "value 'glm::vec3'";
    }
    {
        glm::dvec3 value;
        const bool success = _d->getValue("dvec3", value);
        EXPECT_EQ(true, success) << "success 'glm::dvec3'";
        EXPECT_EQ(glm::dvec3(1.0, 2.0, 3.0), value) << "value 'glm::dvec3'";
    }
    {
        glm::ivec3 value;
        const bool success = _d->getValue("ivec3", value);
        EXPECT_EQ(true, success) << "success 'glm::ivec3'";
        EXPECT_EQ(glm::ivec3(1, 2, 3), value) << "value 'glm::ivec3'";
    }
    {
        glm::uvec3 value;
        const bool success = _d->getValue("uvec3", value);
        EXPECT_EQ(true, success) << "success 'glm::uvec3'";
        EXPECT_EQ(glm::uvec3(1, 2, 3), value) << "value 'glm::uvec3'";
    }
    {
        glm::bvec3 value;
        const bool success = _d->getValue("bvec3", value);
        EXPECT_EQ(true, success) << "success 'glm::bvec3'";
        EXPECT_EQ(glm::bvec3(true, false, true), value) << "value 'glm::bvec3'";
    }
    {
        glm::vec4 value;
        const bool success = _d->getValue("vec4", value);
        EXPECT_EQ(true, success) << "success 'glm::vec4'";
        EXPECT_EQ(glm::vec4(1.f, 2.f, 3.f, 4.f), value) << "value 'glm::vec4'";
    }
    {
        glm::dvec4 value;
        const bool success = _d->getValue("dvec4", value);
        EXPECT_EQ(true, success) << "success 'glm::dvec4'";
        EXPECT_EQ(glm::dvec4(1.0, 2.0, 3.0, 4.0), value) << "value 'glm::dvec4'";
    }
    {
        glm::ivec4 value;
        const bool success = _d->getValue("ivec4", value);
        EXPECT_EQ(true, success) << "success 'glm::ivec4'";
        EXPECT_EQ(glm::ivec4(1, 2, 3, 4), value) << "value 'glm::ivec4'";
    }
    {
        glm::uvec4 value;
        const bool success = _d->getValue("uvec4", value);
        EXPECT_EQ(true, success) << "success 'glm::uvec4'";
        EXPECT_EQ(glm::uvec4(1, 2, 3, 4), value) << "value 'glm::uvec4'";
    }
    {
        glm::bvec4 value;
        const bool success = _d->getValue("bvec4", value);
        EXPECT_EQ(true, success) << "success 'glm::bvec4'";
        EXPECT_EQ(glm::bvec4(true, false, true, false), value) << "value 'glm::bvec4'";
    }
    {
        glm::mat2x2 value;
        const bool success = _d->getValue("mat2x2", value);
        EXPECT_EQ(true, success) << "success 'glm::mat2x2'";
        EXPECT_EQ(glm::mat2x2(1.f, 2.f, 3.f, 4.f), value) << "value 'glm::mat2x2'";
    }
    {
        glm::mat2x3 value;
        const bool success = _d->getValue("mat2x3", value);
        EXPECT_EQ(true, success) << "success 'glm::mat2x3'";
        EXPECT_EQ(glm::mat2x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f), value)
              << "value 'glm::mat2x3'";
    }
    {
        glm::mat2x4 value;
        const bool success = _d->getValue("mat2x4", value);
        EXPECT_EQ(true, success) << "success 'glm::mat2x4'";
        EXPECT_EQ(glm::mat2x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f), value)
              << "value 'glm::mat2x4'";
    }
    {
        glm::mat3x2 value;
        const bool success = _d->getValue("mat3x2", value);
        EXPECT_EQ(true, success) << "success 'glm::mat3x2'";
        EXPECT_EQ(glm::mat3x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f), value)
              << "value 'glm::mat3x2'";
    }
    {
        glm::mat3x3 value;
        const bool success = _d->getValue("mat3x3", value);
        EXPECT_EQ(true, success) << "success 'glm::mat3x3'";
        EXPECT_EQ(glm::mat3x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f), value)
              << "value 'glm::mat3x3'";
    }
    {
        glm::mat3x4 value;
        const bool success = _d->getValue("mat3x4", value);
        EXPECT_EQ(true, success) << "success 'glm::mat3x4'";
        EXPECT_EQ(
              glm::mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f),
              value)
              << "value 'glm::mat3x4'";
    }
    {
        glm::mat4x2 value;
        const bool success = _d->getValue("mat4x2", value);
        EXPECT_EQ(true, success) << "success 'glm::mat4x2'";
        EXPECT_EQ(glm::mat4x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f), value)
              << "value 'glm::mat4x2'";
    }
    {
        glm::mat4x3 value;
        const bool success = _d->getValue("mat4x3", value);
        EXPECT_EQ(true, success) << "success 'glm::mat4x3'";
        EXPECT_EQ(
              glm::mat4x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f),
              value)
              << "value 'glm::mat4x3'";
    }
    {
        glm::mat4x4 value;
        const bool success = _d->getValue("mat4x4", value);
        EXPECT_EQ(true, success) << "success 'glm::mat4x4'";
        EXPECT_EQ(glm::mat4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f,
                              12.f, 13.f, 14.f, 15.f, 16.f),
                  value)
              << "value 'glm::mat4x4'";
    }
    {
        glm::dmat2x2 value;
        const bool success = _d->getValue("dmat2x2", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat2x2'";
        EXPECT_EQ(glm::dmat2x2(1.0, 2.0, 3.0, 4.0), value) << "value 'glm::dmat2x2'";
    }
    {
        glm::dmat2x3 value;
        const bool success = _d->getValue("dmat2x3", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat2x3'";
        EXPECT_EQ(glm::dmat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0), value)
              << "value 'glm::dmat2x3'";
    }
    {
        glm::dmat2x4 value;
        const bool success = _d->getValue("dmat2x4", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat2x4'";
        EXPECT_EQ(glm::dmat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0), value)
              << "value 'glm::dmat2x4'";
    }
    {
        glm::dmat3x2 value;
        const bool success = _d->getValue("dmat3x2", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat3x2'";
        EXPECT_EQ(glm::dmat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0), value)
              << "value 'glm::dmat3x2'";
    }
    {
        glm::dmat3x3 value;
        const bool success = _d->getValue("dmat3x3", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat3x3'";
        EXPECT_EQ(glm::dmat3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0), value)
              << "value 'glm::dmat3x3'";
    }
    {
        glm::dmat3x4 value;
        const bool success = _d->getValue("dmat3x4", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat3x4'";
        EXPECT_EQ(
              glm::dmat3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0),
              value)
              << "value 'glm::dmat3x4'";
    }
    {
        glm::dmat4x2 value;
        const bool success = _d->getValue("dmat4x2", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat4x2'";
        EXPECT_EQ(glm::dmat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0), value)
              << "value 'glm::dmat4x2'";
    }
    {
        glm::dmat4x3 value;
        const bool success = _d->getValue("dmat4x3", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat4x3'";
        EXPECT_EQ(
              glm::dmat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0),
              value)
              << "value 'glm::dmat4x3'";
    }
    {
        glm::dmat4x4 value;
        const bool success = _d->getValue("dmat4x4", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat4x4'";
        EXPECT_EQ(glm::dmat4x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
                               12.0, 13.0, 14.0, 15.0, 16.0),
                  value)
              << "value 'glm::dmat4x4'";
    }
}

TEST_F(DictionaryTest, SetValue) {
    {
        EXPECT_EQ(false, _d->hasValue<bool>("bool")) << "!hasValue 'bool'";
        _d->setValue("bool", bool(true));
        EXPECT_EQ(true, _d->hasValue<bool>("bool")) << "hasValue 'bool'";
        bool value;
        const bool success = _d->getValue("bool", value);
        EXPECT_EQ(true, success) << "success 'bool'";
        EXPECT_EQ(true, value) << "value 'bool'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<char>("char")) << "!hasValue 'char'";
        _d->setValue("char", char(1));
        EXPECT_EQ(true, _d->hasValue<char>("char")) << "hasValue 'char'";
        char value;
        const bool success = _d->getValue("char", value);
        EXPECT_EQ(true, success) << "success 'char'";
        EXPECT_EQ(char(1), value) << "value 'char'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<signed char>("signed char"))
              << "!hasValue 'signed char'";
        _d->setValue("signed char", static_cast<signed char>(1));
        EXPECT_EQ(true, _d->hasValue<signed char>("signed char"))
              << "hasValue 'signed char'";
        signed char value;
        const bool success = _d->getValue("signed char", value);
        EXPECT_EQ(true, success) << "success 'signed char'";
        EXPECT_EQ(static_cast<signed char>(1), value) << "value 'signed char'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<unsigned char>("unsigned char"))
              << "!hasValue 'unsigned char'";
        _d->setValue("unsigned char", static_cast<unsigned char>(1));
        EXPECT_EQ(true, _d->hasValue<unsigned char>("unsigned char"))
              << "hasValue 'unsigned char'";
        unsigned char value;
        const bool success = _d->getValue("unsigned char", value);
        EXPECT_EQ(true, success) << "success 'unsigned char'";
        EXPECT_EQ(static_cast<unsigned char>(1), value) << "value 'unsigned char'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<wchar_t>("wchar_t")) << "!hasValue 'wchar_t'";
        _d->setValue("wchar_t", wchar_t(1));
        EXPECT_EQ(true, _d->hasValue<wchar_t>("wchar_t")) << "hasValue 'wchar_t'";
        wchar_t value;
        const bool success = _d->getValue("wchar_t", value);
        EXPECT_EQ(true, success) << "success 'wchar_t'";
        EXPECT_EQ(wchar_t(1), value) << "value 'wchar_t'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<short>("short")) << "!hasValue 'short'";
        _d->setValue("short", short(1));
        EXPECT_EQ(true, _d->hasValue<short>("short")) << "hasValue 'short'";
        short value;
        const bool success = _d->getValue("short", value);
        EXPECT_EQ(true, success) << "success 'short'";
        EXPECT_EQ(short(1), value) << "value 'short'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<unsigned short>("unsigned short"))
              << "!hasValue 'unsigned short'";
        _d->setValue("unsigned short", static_cast<unsigned short>(1));
        EXPECT_EQ(true, _d->hasValue<unsigned short>("unsigned short"))
              << "hasValue 'unsigned short'";
        unsigned short value;
        const bool success = _d->getValue("unsigned short", value);
        EXPECT_EQ(true, success) << "success 'unsigned short'";
        EXPECT_EQ(static_cast<unsigned short>(1), value) << "value 'unsigned short'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<int>("int")) << "!hasValue 'int'";
        _d->setValue("int", int(1));
        EXPECT_EQ(true, _d->hasValue<int>("int")) << "hasValue 'int'";
        int value;
        const bool success = _d->getValue("int", value);
        EXPECT_EQ(true, success) << "success 'int'";
        EXPECT_EQ(int(1), value) << "value 'int'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<unsigned int>("unsigned int"))
              << "!hasValue 'unsigned int'";
        _d->setValue("unsigned int", static_cast<unsigned int>(1));
        EXPECT_EQ(true, _d->hasValue<unsigned int>("unsigned int"))
              << "hasValue 'unsigned int'";
        unsigned int value;
        const bool success = _d->getValue("unsigned int", value);
        EXPECT_EQ(true, success) << "success 'unsigned int'";
        EXPECT_EQ(static_cast<unsigned int>(1), value) << "value 'unsigned int'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<long long>("long long")) << "!hasValue 'long long'";
        _d->setValue("long long", static_cast<long long>(1));
        EXPECT_EQ(true, _d->hasValue<long long>("long long")) << "hasValue 'long long'";
        long long value;
        const bool success = _d->getValue("long long", value);
        EXPECT_EQ(true, success) << "success 'long long'";
        EXPECT_EQ(static_cast<long long>(1), value) << "value 'long long'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<unsigned long long>("unsigned long long"))
              << "!hasValue 'unsigned long long'";
        _d->setValue("unsigned long long", static_cast<unsigned long long>(1));
        EXPECT_EQ(true, _d->hasValue<unsigned long long>("unsigned long long"))
              << "hasValue 'unsigned long long'";
        unsigned long long value;
        const bool success = _d->getValue("unsigned long long", value);
        EXPECT_EQ(true, success) << "success 'unsigned long long'";
        EXPECT_EQ(static_cast<unsigned long long>(1), value)
              << "value 'unsigned long long'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<float>("float")) << "!hasValue 'float'";
        _d->setValue("float", 1.f);
        EXPECT_EQ(true, _d->hasValue<float>("float")) << "hasValue 'float'";
        float value;
        const bool success = _d->getValue("float", value);
        EXPECT_EQ(true, success) << "success 'float'";
        EXPECT_EQ(1.f, value) << "value 'float'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<double>("double")) << "!hasValue 'double'";
        _d->setValue("double", 1.0);
        EXPECT_EQ(true, _d->hasValue<double>("double")) << "hasValue 'double'";
        double value;
        const bool success = _d->getValue("double", value);
        EXPECT_EQ(true, success) << "success 'double'";
        EXPECT_EQ(1.0, value) << "value 'double'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<long double>("long double"))
              << "!hasValue 'long double'";
        _d->setValue("long double", static_cast<long double>(1));
        EXPECT_EQ(true, _d->hasValue<long double>("long double"))
              << "hasValue 'long double'";
        long double value;
        const bool success = _d->getValue("long double", value);
        EXPECT_EQ(true, success) << "success 'long double'";
        EXPECT_EQ(static_cast<long double>(1), value) << "value 'long double'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::vec2>("vec2")) << "!hasValue 'glm::vec2'";
        _d->setValue("vec2", glm::vec2(1.f, 2.f));
        EXPECT_EQ(true, _d->hasValue<glm::vec2>("vec2")) << "hasValue 'glm::vec2'";
        glm::vec2 value;
        const bool success = _d->getValue("vec2", value);
        EXPECT_EQ(true, success) << "success 'glm::vec2'";
        EXPECT_EQ(glm::vec2(1.f, 2.f), value) << "value 'glm::vec2'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dvec2>("dvec2")) << "!hasValue 'glm::dvec2'";
        _d->setValue("dvec2", glm::dvec2(1.0, 2.0));
        EXPECT_EQ(true, _d->hasValue<glm::dvec2>("dvec2")) << "hasValue 'glm::dvec2'";
        glm::dvec2 value;
        const bool success = _d->getValue("dvec2", value);
        EXPECT_EQ(true, success) << "success 'glm::dvec2'";
        EXPECT_EQ(glm::dvec2(1.0, 2.0), value) << "value 'glm::dvec2'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::ivec2>("ivec2")) << "!hasValue 'glm::ivec2'";
        _d->setValue("ivec2", glm::ivec2(1, 2));
        EXPECT_EQ(true, _d->hasValue<glm::ivec2>("ivec2")) << "hasValue 'glm::ivec2'";
        glm::ivec2 value;
        const bool success = _d->getValue("ivec2", value);
        EXPECT_EQ(true, success) << "success 'glm::ivec2'";
        EXPECT_EQ(glm::ivec2(1, 2), value) << "value 'glm::ivec2'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::bvec2>("bvec2")) << "!hasValue 'glm::bvec2'";
        _d->setValue("bvec2", glm::bvec2(true, false));
        EXPECT_EQ(true, _d->hasValue<glm::bvec2>("bvec2")) << "hasValue 'glm::bvec2'";
        glm::bvec2 value;
        const bool success = _d->getValue("bvec2", value);
        EXPECT_EQ(true, success) << "success 'glm::bvec2'";
        EXPECT_EQ(glm::bvec2(true, false), value) << "value 'glm::bvec2'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::vec3>("vec3")) << "!hasValue 'glm::vec3'";
        _d->setValue("vec3", glm::vec3(1.f, 2.f, 3.f));
        EXPECT_EQ(true, _d->hasValue<glm::vec3>("vec3")) << "hasValue 'glm::vec3'";
        glm::vec3 value;
        const bool success = _d->getValue("vec3", value);
        EXPECT_EQ(true, success) << "success 'glm::vec3'";
        EXPECT_EQ(glm::vec3(1.f, 2.f, 3.f), value) << "value 'glm::vec3'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dvec3>("dvec3")) << "!hasValue 'glm::dvec3'";
        _d->setValue("dvec3", glm::dvec3(1.0, 2.0, 3.0));
        EXPECT_EQ(true, _d->hasValue<glm::dvec3>("dvec3")) << "hasValue 'glm::dvec3'";
        glm::dvec3 value;
        const bool success = _d->getValue("dvec3", value);
        EXPECT_EQ(true, success) << "success 'glm::dvec3'";
        EXPECT_EQ(glm::dvec3(1.0, 2.0, 3.0), value) << "value 'glm::dvec3'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::ivec3>("ivec3")) << "!hasValue 'glm::ivec3'";
        _d->setValue("ivec3", glm::ivec3(1, 2, 3));
        EXPECT_EQ(true, _d->hasValue<glm::ivec3>("ivec3")) << "hasValue 'glm::ivec3'";
        glm::ivec3 value;
        const bool success = _d->getValue("ivec3", value);
        EXPECT_EQ(true, success) << "success 'glm::ivec3'";
        EXPECT_EQ(glm::ivec3(1, 2, 3), value) << "value 'glm::ivec3'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::bvec3>("bvec3")) << "!hasValue 'glm::bvec3'";
        _d->setValue("bvec3", glm::bvec3(true, false, true));
        EXPECT_EQ(true, _d->hasValue<glm::bvec3>("bvec3")) << "hasValue 'glm::bvec3'";
        glm::bvec3 value;
        const bool success = _d->getValue("bvec3", value);
        EXPECT_EQ(true, success) << "success 'glm::bvec3'";
        EXPECT_EQ(glm::bvec3(true, false, true), value) << "value 'glm::bvec3'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::vec4>("vec4")) << "!hasValue 'glm::vec4'";
        _d->setValue("vec4", glm::vec4(1.f, 2.f, 3.f, 4.f));
        EXPECT_EQ(true, _d->hasValue<glm::vec4>("vec4")) << "hasValue 'glm::vec4'";
        glm::vec4 value;
        const bool success = _d->getValue("vec4", value);
        EXPECT_EQ(true, success) << "success 'glm::vec4'";
        EXPECT_EQ(glm::vec4(1.f, 2.f, 3.f, 4.f), value) << "value 'glm::vec4'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dvec4>("dvec4")) << "!hasValue 'glm::dvec4'";
        _d->setValue("dvec4", glm::dvec4(1.0, 2.0, 3.0, 4.0));
        EXPECT_EQ(true, _d->hasValue<glm::dvec4>("dvec4")) << "hasValue 'glm::dvec4'";
        glm::dvec4 value;
        const bool success = _d->getValue("dvec4", value);
        EXPECT_EQ(true, success) << "success 'glm::dvec4'";
        EXPECT_EQ(glm::dvec4(1.0, 2.0, 3.0, 4.0), value) << "value 'glm::dvec4'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::ivec4>("ivec4")) << "!hasValue 'glm::ivec4'";
        _d->setValue("ivec4", glm::ivec4(1, 2, 3, 4));
        EXPECT_EQ(true, _d->hasValue<glm::ivec4>("ivec4")) << "hasValue 'glm::ivec4'";
        glm::ivec4 value;
        const bool success = _d->getValue("ivec4", value);
        EXPECT_EQ(true, success) << "success 'glm::ivec4'";
        EXPECT_EQ(glm::ivec4(1, 2, 3, 4), value) << "value 'glm::ivec4'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::bvec4>("bvec4")) << "!hasValue 'glm::bvec4'";
        _d->setValue("bvec4", glm::bvec4(true, false, true, false));
        EXPECT_EQ(true, _d->hasValue<glm::bvec4>("bvec4")) << "hasValue 'glm::bvec4'";
        glm::bvec4 value;
        const bool success = _d->getValue("bvec4", value);
        EXPECT_EQ(true, success) << "success 'glm::bvec4'";
        EXPECT_EQ(glm::bvec4(true, false, true, false), value) << "value 'glm::bvec4'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::mat2x2>("mat2x2"))
              << "!hasValue 'glm::mat2x2'";
        _d->setValue("mat2x2", glm::mat2x2(1.f, 2.f, 3.f, 4.f));
        EXPECT_EQ(true, _d->hasValue<glm::mat2x2>("mat2x2")) << "hasValue 'glm::mat2x2'";
        glm::mat2x2 value;
        const bool success = _d->getValue("mat2x2", value);
        EXPECT_EQ(true, success) << "success 'glm::mat2x2'";
        EXPECT_EQ(glm::mat2x2(1.f, 2.f, 3.f, 4.f), value) << "value 'glm::mat2x2'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::mat2x3>("mat2x3"))
              << "!hasValue 'glm::mat2x3'";
        _d->setValue("mat2x3", glm::mat2x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f));
        EXPECT_EQ(true, _d->hasValue<glm::mat2x3>("mat2x3")) << "hasValue 'glm::mat2x3'";
        glm::mat2x3 value;
        const bool success = _d->getValue("mat2x3", value);
        EXPECT_EQ(true, success) << "success 'glm::mat2x3'";
        EXPECT_EQ(glm::mat2x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f), value)
              << "value 'glm::mat2x3'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::mat2x4>("mat2x4"))
              << "!hasValue 'glm::mat2x4'";
        _d->setValue("mat2x4", glm::mat2x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f));
        EXPECT_EQ(true, _d->hasValue<glm::mat2x4>("mat2x4")) << "hasValue 'glm::mat2x4'";
        glm::mat2x4 value;
        const bool success = _d->getValue("mat2x4", value);
        EXPECT_EQ(true, success) << "success 'glm::mat2x4'";
        EXPECT_EQ(glm::mat2x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f), value)
              << "value 'glm::mat2x4'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::mat3x2>("mat3x2"))
              << "!hasValue 'glm::mat3x2'";
        _d->setValue("mat3x2", glm::mat3x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f));
        EXPECT_EQ(true, _d->hasValue<glm::mat3x2>("mat3x2")) << "hasValue 'glm::mat3x2'";
        glm::mat3x2 value;
        const bool success = _d->getValue("mat3x2", value);
        EXPECT_EQ(true, success) << "success 'glm::mat3x2'";
        EXPECT_EQ(glm::mat3x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f), value)
              << "value 'glm::mat3x2'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::mat3x3>("mat3x3"))
              << "!hasValue 'glm::mat3x3'";
        _d->setValue("mat3x3", glm::mat3x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f));
        EXPECT_EQ(true, _d->hasValue<glm::mat3x3>("mat3x3")) << "hasValue 'glm::mat3x3'";
        glm::mat3x3 value;
        const bool success = _d->getValue("mat3x3", value);
        EXPECT_EQ(true, success) << "success 'glm::mat3x3'";
        EXPECT_EQ(glm::mat3x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f), value)
              << "value 'glm::mat3x3'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::mat3x4>("mat3x4"))
              << "!hasValue 'glm::mat3x4'";
        _d->setValue("mat3x4", glm::mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f,
                                           10.f, 11.f, 12.f));
        EXPECT_EQ(true, _d->hasValue<glm::mat3x4>("mat3x4")) << "hasValue 'glm::mat3x4'";
        glm::mat3x4 value;
        const bool success = _d->getValue("mat3x4", value);
        EXPECT_EQ(true, success) << "success 'glm::mat3x4'";
        EXPECT_EQ(
              glm::mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f),
              value)
              << "value 'glm::mat3x4'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::mat4x2>("mat4x2"))
              << "!hasValue 'glm::mat4x2'";
        _d->setValue("mat4x2", glm::mat4x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f));
        EXPECT_EQ(true, _d->hasValue<glm::mat4x2>("mat4x2")) << "hasValue 'glm::mat4x2'";
        glm::mat4x2 value;
        const bool success = _d->getValue("mat4x2", value);
        EXPECT_EQ(true, success) << "success 'glm::mat4x2'";
        EXPECT_EQ(glm::mat4x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f), value)
              << "value 'glm::mat4x2'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::mat4x3>("mat4x3"))
              << "!hasValue 'glm::mat4x3'";
        _d->setValue("mat4x3", glm::mat4x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f));
        EXPECT_EQ(true, _d->hasValue<glm::mat4x3>("mat4x3")) << "hasValue 'glm::mat4x3'";
        glm::mat4x3 value;
        const bool success = _d->getValue("mat4x3", value);
        EXPECT_EQ(true, success) << "success 'glm::mat4x3'";
        EXPECT_EQ(glm::mat4x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f), value) << "value 'glm::mat4x3'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::mat4x4>("mat4x4"))
              << "!hasValue 'glm::mat4x4'";
        _d->setValue("mat4x4", glm::mat4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f,
                                           10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f));
        EXPECT_EQ(true, _d->hasValue<glm::mat4x4>("mat4x4")) << "hasValue 'glm::mat4x4'";
        glm::mat4x4 value;
        const bool success = _d->getValue("mat4x4", value);
        EXPECT_EQ(true, success) << "success 'glm::mat4x4'";
        EXPECT_EQ(glm::mat4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f,
                              12.f, 13.f, 14.f, 15.f, 16.f),
                  value)
              << "value 'glm::mat4x4'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dmat2x2>("dmat2x2"))
              << "!hasValue 'glm::dmat2x2'";
        _d->setValue("dmat2x2", glm::dmat2x2(1.0, 2.0, 3.0, 4.0));
        EXPECT_EQ(true, _d->hasValue<glm::dmat2x2>("dmat2x2"))
              << "hasValue 'glm::dmat2x2'";
        glm::dmat2x2 value;
        const bool success = _d->getValue("dmat2x2", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat2x2'";
        EXPECT_EQ(glm::dmat2x2(1.0, 2.0, 3.0, 4.0), value) << "value 'glm::dmat2x2'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dmat2x3>("dmat2x3"))
              << "!hasValue 'glm::dmat2x3'";
        _d->setValue("dmat2x3", glm::dmat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0));
        EXPECT_EQ(true, _d->hasValue<glm::dmat2x3>("dmat2x3"))
              << "hasValue 'glm::dmat2x3'";
        glm::dmat2x3 value;
        const bool success = _d->getValue("dmat2x3", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat2x3'";
        EXPECT_EQ(glm::dmat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0), value)
              << "value 'glm::dmat2x3'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dmat2x4>("dmat2x4"))
              << "!hasValue 'glm::dmat2x4'";
        _d->setValue("dmat2x4", glm::dmat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));
        EXPECT_EQ(true, _d->hasValue<glm::dmat2x4>("dmat2x4"))
              << "hasValue 'glm::dmat2x4'";
        glm::dmat2x4 value;
        const bool success = _d->getValue("dmat2x4", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat2x4'";
        EXPECT_EQ(glm::dmat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0), value)
              << "value 'glm::dmat2x4'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dmat3x2>("dmat3x2"))
              << "!hasValue 'glm::dmat3x2'";
        _d->setValue("dmat3x2", glm::dmat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0));
        EXPECT_EQ(true, _d->hasValue<glm::dmat3x2>("dmat3x2"))
              << "hasValue 'glm::dmat3x2'";
        glm::dmat3x2 value;
        const bool success = _d->getValue("dmat3x2", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat3x2'";
        EXPECT_EQ(glm::dmat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0), value)
              << "value 'glm::dmat3x2'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dmat3x3>("dmat3x3"))
              << "!hasValue 'glm::dmat3x3'";
        _d->setValue("dmat3x3",
                     glm::dmat3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0));
        EXPECT_EQ(true, _d->hasValue<glm::dmat3x3>("dmat3x3"))
              << "hasValue 'glm::dmat3x3'";
        glm::dmat3x3 value;
        const bool success = _d->getValue("dmat3x3", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat3x3'";
        EXPECT_EQ(glm::dmat3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0), value)
              << "value 'glm::dmat3x3'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dmat3x4>("dmat3x4"))
              << "!hasValue 'glm::dmat3x4'";
        _d->setValue("dmat3x4", glm::dmat3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
                                             10.0, 11.0, 12.0));
        EXPECT_EQ(true, _d->hasValue<glm::dmat3x4>("dmat3x4"))
              << "hasValue 'glm::dmat3x4'";
        glm::dmat3x4 value;
        const bool success = _d->getValue("dmat3x4", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat3x4'";
        EXPECT_EQ(
              glm::dmat3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0),
              value)
              << "value 'glm::dmat3x4'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dmat4x2>("dmat4x2"))
              << "!hasValue 'glm::dmat4x2'";
        _d->setValue("dmat4x2", glm::dmat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0));
        EXPECT_EQ(true, _d->hasValue<glm::dmat4x2>("dmat4x2"))
              << "hasValue 'glm::dmat4x2'";
        glm::dmat4x2 value;
        const bool success = _d->getValue("dmat4x2", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat4x2'";
        EXPECT_EQ(glm::dmat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0), value)
              << "value 'glm::dmat4x2'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dmat4x3>("dmat4x3"))
              << "!hasValue 'glm::dmat4x3'";
        _d->setValue("dmat4x3", glm::dmat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
                                             10.0, 11.0, 12.0));
        EXPECT_EQ(true, _d->hasValue<glm::dmat4x3>("dmat4x3"))
              << "hasValue 'glm::dmat4x3'";
        glm::dmat4x3 value;
        const bool success = _d->getValue("dmat4x3", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat4x3'";
        EXPECT_EQ(
              glm::dmat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0),
              value)
              << "value 'glm::dmat4x3'";
    }
    {
        EXPECT_EQ(false, _d->hasValue<glm::dmat4x4>("dmat4x4"))
              << "!hasValue 'glm::dmat4x4'";
        _d->setValue("dmat4x4", glm::dmat4x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
                                             10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
        EXPECT_EQ(true, _d->hasValue<glm::dmat4x4>("dmat4x4"))
              << "hasValue 'glm::dmat4x4'";
        glm::dmat4x4 value;
        const bool success = _d->getValue("dmat4x4", value);
        EXPECT_EQ(true, success) << "success 'glm::dmat4x4'";
        EXPECT_EQ(glm::dmat4x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
                               12.0, 13.0, 14.0, 15.0, 16.0),
                  value)
              << "value 'glm::dmat4x4'";
    }
}

TEST_F(DictionaryTest, hasValueTypeCast) {
    createDefaultDictionary();
    bool success;

    success = _d->hasValue<bool>("bool");
    EXPECT_EQ(true, success) << "bool -> bool";

    success = _d->hasValue<bool>("char");
    EXPECT_EQ(true, success) << "char -> bool";

    success = _d->hasValue<bool>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> bool";

    success = _d->hasValue<bool>("wchar_t");
    EXPECT_EQ(true, success) << "wchar_t -> bool";

    success = _d->hasValue<bool>("short");
    EXPECT_EQ(true, success) << "short -> bool";

    success = _d->hasValue<bool>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> bool";

    success = _d->hasValue<bool>("int");
    EXPECT_EQ(true, success) << "int -> bool";

    success = _d->hasValue<bool>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> bool";

    success = _d->hasValue<bool>("long long");
    EXPECT_EQ(true, success) << "long long -> bool";

    success = _d->hasValue<bool>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> bool";

    success = _d->hasValue<bool>("float");
    EXPECT_EQ(false, success) << "float -> bool";

    success = _d->hasValue<bool>("double");
    EXPECT_EQ(false, success) << "double -> bool";

    success = _d->hasValue<bool>("long double");
    EXPECT_EQ(false, success) << "long double -> bool";

    success = _d->hasValue<bool>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> bool";

    success = _d->hasValue<bool>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> bool";

    success = _d->hasValue<bool>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> bool";

    success = _d->hasValue<bool>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> bool";

    success = _d->hasValue<bool>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> bool";

    success = _d->hasValue<bool>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> bool";

    success = _d->hasValue<bool>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> bool";

    success = _d->hasValue<bool>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> bool";

    success = _d->hasValue<bool>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> bool";

    success = _d->hasValue<bool>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> bool";

    success = _d->hasValue<bool>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> bool";

    success = _d->hasValue<bool>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> bool";

    success = _d->hasValue<bool>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> bool";

    success = _d->hasValue<bool>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> bool";

    success = _d->hasValue<bool>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> bool";

    success = _d->hasValue<bool>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> bool";

    success = _d->hasValue<bool>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> bool";

    success = _d->hasValue<bool>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> bool";

    success = _d->hasValue<bool>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> bool";

    success = _d->hasValue<bool>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> bool";

    success = _d->hasValue<bool>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> bool";

    success = _d->hasValue<bool>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> bool";

    success = _d->hasValue<bool>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> bool";

    success = _d->hasValue<bool>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> bool";

    success = _d->hasValue<bool>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> bool";

    success = _d->hasValue<bool>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> bool";

    success = _d->hasValue<bool>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> bool";

    success = _d->hasValue<bool>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> bool";

    success = _d->hasValue<bool>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> bool";

    success = _d->hasValue<bool>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> bool";

    success = _d->hasValue<bool>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> bool";

    success = _d->hasValue<bool>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> bool";

    success = _d->hasValue<bool>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> bool";

    success = _d->hasValue<bool>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> bool";

    success = _d->hasValue<char>("bool");
    EXPECT_EQ(true, success) << "bool -> char";

    success = _d->hasValue<char>("char");
    EXPECT_EQ(true, success) << "char -> char";

    success = _d->hasValue<char>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> char";

    success = _d->hasValue<char>("wchar_t");
    EXPECT_EQ(true, success) << "wchar_t -> char";

    success = _d->hasValue<char>("short");
    EXPECT_EQ(true, success) << "short -> char";

    success = _d->hasValue<char>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> char";

    success = _d->hasValue<char>("int");
    EXPECT_EQ(true, success) << "int -> char";

    success = _d->hasValue<char>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> char";

    success = _d->hasValue<char>("long long");
    EXPECT_EQ(true, success) << "long long -> char";

    success = _d->hasValue<char>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> char";

    success = _d->hasValue<char>("float");
    EXPECT_EQ(false, success) << "float -> char";

    success = _d->hasValue<char>("double");
    EXPECT_EQ(false, success) << "double -> char";

    success = _d->hasValue<char>("long double");
    EXPECT_EQ(false, success) << "long double -> char";

    success = _d->hasValue<char>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> char";

    success = _d->hasValue<char>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> char";

    success = _d->hasValue<char>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> char";

    success = _d->hasValue<char>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> char";

    success = _d->hasValue<char>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> char";

    success = _d->hasValue<char>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> char";

    success = _d->hasValue<char>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> char";

    success = _d->hasValue<char>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> char";

    success = _d->hasValue<char>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> char";

    success = _d->hasValue<char>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> char";

    success = _d->hasValue<char>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> char";

    success = _d->hasValue<char>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> char";

    success = _d->hasValue<char>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> char";

    success = _d->hasValue<char>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> char";

    success = _d->hasValue<char>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> char";

    success = _d->hasValue<char>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> char";

    success = _d->hasValue<char>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> char";

    success = _d->hasValue<char>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> char";

    success = _d->hasValue<char>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> char";

    success = _d->hasValue<char>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> char";

    success = _d->hasValue<char>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> char";

    success = _d->hasValue<char>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> char";

    success = _d->hasValue<char>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> char";

    success = _d->hasValue<char>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> char";

    success = _d->hasValue<char>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> char";

    success = _d->hasValue<char>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> char";

    success = _d->hasValue<char>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> char";

    success = _d->hasValue<char>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> char";

    success = _d->hasValue<char>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> char";

    success = _d->hasValue<char>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> char";

    success = _d->hasValue<char>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> char";

    success = _d->hasValue<char>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> char";

    success = _d->hasValue<char>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> char";

    success = _d->hasValue<char>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> char";

    success = _d->hasValue<unsigned char>("bool");
    EXPECT_EQ(false, success) << "bool -> unsigned char";

    success = _d->hasValue<unsigned char>("char");
    EXPECT_EQ(false, success) << "char -> unsigned char";

    success = _d->hasValue<unsigned char>("unsigned char");
    EXPECT_EQ(true, success) << "unsigned char -> unsigned char";

    success = _d->hasValue<unsigned char>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> unsigned char";

    success = _d->hasValue<unsigned char>("short");
    EXPECT_EQ(false, success) << "short -> unsigned char";

    success = _d->hasValue<unsigned char>("unsigned short");
    EXPECT_EQ(true, success) << "unsigned short -> unsigned char";

    success = _d->hasValue<unsigned char>("int");
    EXPECT_EQ(false, success) << "int -> unsigned char";

    success = _d->hasValue<unsigned char>("unsigned int");
    EXPECT_EQ(true, success) << "unsigned int -> unsigned char";

    success = _d->hasValue<unsigned char>("long long");
    EXPECT_EQ(false, success) << "long long -> unsigned char";

    success = _d->hasValue<unsigned char>("unsigned long long");
    EXPECT_EQ(true, success) << "unsigned long long -> unsigned char";

    success = _d->hasValue<unsigned char>("float");
    EXPECT_EQ(false, success) << "float -> unsigned char";

    success = _d->hasValue<unsigned char>("double");
    EXPECT_EQ(false, success) << "double -> unsigned char";

    success = _d->hasValue<unsigned char>("long double");
    EXPECT_EQ(false, success) << "long double -> unsigned char";

    success = _d->hasValue<unsigned char>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> unsigned char";

    success = _d->hasValue<unsigned char>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> unsigned char";

    success = _d->hasValue<unsigned char>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> unsigned char";

    success = _d->hasValue<unsigned char>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> unsigned char";

    success = _d->hasValue<unsigned char>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> unsigned char";

    success = _d->hasValue<unsigned char>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> unsigned char";

    success = _d->hasValue<unsigned char>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> unsigned char";

    success = _d->hasValue<unsigned char>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> unsigned char";

    success = _d->hasValue<unsigned char>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> unsigned char";

    success = _d->hasValue<unsigned char>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> unsigned char";

    success = _d->hasValue<unsigned char>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> unsigned char";

    success = _d->hasValue<unsigned char>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> unsigned char";

    success = _d->hasValue<unsigned char>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> unsigned char";

    success = _d->hasValue<unsigned char>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> unsigned char";

    success = _d->hasValue<unsigned char>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> unsigned char";

    success = _d->hasValue<unsigned char>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> unsigned char";

    success = _d->hasValue<unsigned char>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> unsigned char";

    success = _d->hasValue<unsigned char>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> unsigned char";

    success = _d->hasValue<unsigned char>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> unsigned char";

    success = _d->hasValue<unsigned char>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> unsigned char";

    success = _d->hasValue<unsigned char>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> unsigned char";

    success = _d->hasValue<unsigned char>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> unsigned char";

    success = _d->hasValue<unsigned char>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> unsigned char";

    success = _d->hasValue<unsigned char>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> unsigned char";

    success = _d->hasValue<unsigned char>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> unsigned char";

    success = _d->hasValue<unsigned char>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> unsigned char";

    success = _d->hasValue<unsigned char>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> unsigned char";

    success = _d->hasValue<unsigned char>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> unsigned char";

    success = _d->hasValue<unsigned char>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> unsigned char";

    success = _d->hasValue<unsigned char>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> unsigned char";

    success = _d->hasValue<unsigned char>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> unsigned char";

    success = _d->hasValue<unsigned char>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> unsigned char";

    success = _d->hasValue<unsigned char>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> unsigned char";

    success = _d->hasValue<unsigned char>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> unsigned char";

    success = _d->hasValue<wchar_t>("bool");
    EXPECT_EQ(true, success) << "bool -> wchar_t";

    success = _d->hasValue<wchar_t>("char");
    EXPECT_EQ(true, success) << "char -> wchar_t";

    success = _d->hasValue<wchar_t>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> wchar_t";

    success = _d->hasValue<wchar_t>("wchar_t");
    EXPECT_EQ(true, success) << "wchar_t -> wchar_t";

    success = _d->hasValue<wchar_t>("short");
    EXPECT_EQ(true, success) << "short -> wchar_t";

    success = _d->hasValue<wchar_t>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> wchar_t";

    success = _d->hasValue<wchar_t>("int");
    EXPECT_EQ(true, success) << "int -> wchar_t";

    success = _d->hasValue<wchar_t>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> wchar_t";

    success = _d->hasValue<wchar_t>("long long");
    EXPECT_EQ(true, success) << "long long -> wchar_t";

    success = _d->hasValue<wchar_t>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> wchar_t";

    success = _d->hasValue<wchar_t>("float");
    EXPECT_EQ(false, success) << "float -> wchar_t";

    success = _d->hasValue<wchar_t>("double");
    EXPECT_EQ(false, success) << "double -> wchar_t";

    success = _d->hasValue<wchar_t>("long double");
    EXPECT_EQ(false, success) << "long double -> wchar_t";

    success = _d->hasValue<wchar_t>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> wchar_t";

    success = _d->hasValue<wchar_t>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> wchar_t";

    success = _d->hasValue<wchar_t>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> wchar_t";

    success = _d->hasValue<wchar_t>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> wchar_t";

    success = _d->hasValue<wchar_t>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> wchar_t";

    success = _d->hasValue<wchar_t>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> wchar_t";

    success = _d->hasValue<wchar_t>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> wchar_t";

    success = _d->hasValue<wchar_t>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> wchar_t";

    success = _d->hasValue<wchar_t>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> wchar_t";

    success = _d->hasValue<wchar_t>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> wchar_t";

    success = _d->hasValue<wchar_t>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> wchar_t";

    success = _d->hasValue<wchar_t>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> wchar_t";

    success = _d->hasValue<wchar_t>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> wchar_t";

    success = _d->hasValue<wchar_t>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> wchar_t";

    success = _d->hasValue<wchar_t>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> wchar_t";

    success = _d->hasValue<wchar_t>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> wchar_t";

    success = _d->hasValue<wchar_t>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> wchar_t";

    success = _d->hasValue<wchar_t>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> wchar_t";

    success = _d->hasValue<wchar_t>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> wchar_t";

    success = _d->hasValue<wchar_t>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> wchar_t";

    success = _d->hasValue<wchar_t>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> wchar_t";

    success = _d->hasValue<wchar_t>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> wchar_t";

    success = _d->hasValue<wchar_t>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> wchar_t";

    success = _d->hasValue<wchar_t>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> wchar_t";

    success = _d->hasValue<wchar_t>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> wchar_t";

    success = _d->hasValue<wchar_t>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> wchar_t";

    success = _d->hasValue<wchar_t>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> wchar_t";

    success = _d->hasValue<wchar_t>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> wchar_t";

    success = _d->hasValue<wchar_t>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> wchar_t";

    success = _d->hasValue<wchar_t>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> wchar_t";

    success = _d->hasValue<wchar_t>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> wchar_t";

    success = _d->hasValue<wchar_t>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> wchar_t";

    success = _d->hasValue<wchar_t>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> wchar_t";

    success = _d->hasValue<wchar_t>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> wchar_t";

    success = _d->hasValue<short>("bool");
    EXPECT_EQ(true, success) << "bool -> short";

    success = _d->hasValue<short>("char");
    EXPECT_EQ(true, success) << "char -> short";

    success = _d->hasValue<short>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> short";

    success = _d->hasValue<short>("wchar_t");
    EXPECT_EQ(true, success) << "wchar_t -> short";

    success = _d->hasValue<short>("short");
    EXPECT_EQ(true, success) << "short -> short";

    success = _d->hasValue<short>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> short";

    success = _d->hasValue<short>("int");
    EXPECT_EQ(true, success) << "int -> short";

    success = _d->hasValue<short>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> short";

    success = _d->hasValue<short>("long long");
    EXPECT_EQ(true, success) << "long long -> short";

    success = _d->hasValue<short>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> short";

    success = _d->hasValue<short>("float");
    EXPECT_EQ(false, success) << "float -> short";

    success = _d->hasValue<short>("double");
    EXPECT_EQ(false, success) << "double -> short";

    success = _d->hasValue<short>("long double");
    EXPECT_EQ(false, success) << "long double -> short";

    success = _d->hasValue<short>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> short";

    success = _d->hasValue<short>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> short";

    success = _d->hasValue<short>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> short";

    success = _d->hasValue<short>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> short";

    success = _d->hasValue<short>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> short";

    success = _d->hasValue<short>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> short";

    success = _d->hasValue<short>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> short";

    success = _d->hasValue<short>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> short";

    success = _d->hasValue<short>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> short";

    success = _d->hasValue<short>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> short";

    success = _d->hasValue<short>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> short";

    success = _d->hasValue<short>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> short";

    success = _d->hasValue<short>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> short";

    success = _d->hasValue<short>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> short";

    success = _d->hasValue<short>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> short";

    success = _d->hasValue<short>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> short";

    success = _d->hasValue<short>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> short";

    success = _d->hasValue<short>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> short";

    success = _d->hasValue<short>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> short";

    success = _d->hasValue<short>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> short";

    success = _d->hasValue<short>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> short";

    success = _d->hasValue<short>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> short";

    success = _d->hasValue<short>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> short";

    success = _d->hasValue<short>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> short";

    success = _d->hasValue<short>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> short";

    success = _d->hasValue<short>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> short";

    success = _d->hasValue<short>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> short";

    success = _d->hasValue<short>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> short";

    success = _d->hasValue<short>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> short";

    success = _d->hasValue<short>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> short";

    success = _d->hasValue<short>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> short";

    success = _d->hasValue<short>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> short";

    success = _d->hasValue<short>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> short";

    success = _d->hasValue<short>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> short";

    success = _d->hasValue<unsigned short>("bool");
    EXPECT_EQ(false, success) << "bool -> unsigned short";

    success = _d->hasValue<unsigned short>("char");
    EXPECT_EQ(false, success) << "char -> unsigned short";

    success = _d->hasValue<unsigned short>("unsigned char");
    EXPECT_EQ(true, success) << "unsigned char -> unsigned short";

    success = _d->hasValue<unsigned short>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> unsigned short";

    success = _d->hasValue<unsigned short>("short");
    EXPECT_EQ(false, success) << "short -> unsigned short";

    success = _d->hasValue<unsigned short>("unsigned short");
    EXPECT_EQ(true, success) << "unsigned short -> unsigned short";

    success = _d->hasValue<unsigned short>("int");
    EXPECT_EQ(false, success) << "int -> unsigned short";

    success = _d->hasValue<unsigned short>("unsigned int");
    EXPECT_EQ(true, success) << "unsigned int -> unsigned short";

    success = _d->hasValue<unsigned short>("long long");
    EXPECT_EQ(false, success) << "long long -> unsigned short";

    success = _d->hasValue<unsigned short>("unsigned long long");
    EXPECT_EQ(true, success) << "unsigned long long -> unsigned short";

    success = _d->hasValue<unsigned short>("float");
    EXPECT_EQ(false, success) << "float -> unsigned short";

    success = _d->hasValue<unsigned short>("double");
    EXPECT_EQ(false, success) << "double -> unsigned short";

    success = _d->hasValue<unsigned short>("long double");
    EXPECT_EQ(false, success) << "long double -> unsigned short";

    success = _d->hasValue<unsigned short>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> unsigned short";

    success = _d->hasValue<unsigned short>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> unsigned short";

    success = _d->hasValue<unsigned short>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> unsigned short";

    success = _d->hasValue<unsigned short>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> unsigned short";

    success = _d->hasValue<unsigned short>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> unsigned short";

    success = _d->hasValue<unsigned short>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> unsigned short";

    success = _d->hasValue<unsigned short>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> unsigned short";

    success = _d->hasValue<unsigned short>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> unsigned short";

    success = _d->hasValue<unsigned short>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> unsigned short";

    success = _d->hasValue<unsigned short>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> unsigned short";

    success = _d->hasValue<unsigned short>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> unsigned short";

    success = _d->hasValue<unsigned short>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> unsigned short";

    success = _d->hasValue<unsigned short>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> unsigned short";

    success = _d->hasValue<unsigned short>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> unsigned short";

    success = _d->hasValue<unsigned short>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> unsigned short";

    success = _d->hasValue<unsigned short>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> unsigned short";

    success = _d->hasValue<unsigned short>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> unsigned short";

    success = _d->hasValue<unsigned short>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> unsigned short";

    success = _d->hasValue<unsigned short>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> unsigned short";

    success = _d->hasValue<unsigned short>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> unsigned short";

    success = _d->hasValue<unsigned short>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> unsigned short";

    success = _d->hasValue<unsigned short>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> unsigned short";

    success = _d->hasValue<unsigned short>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> unsigned short";

    success = _d->hasValue<unsigned short>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> unsigned short";

    success = _d->hasValue<unsigned short>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> unsigned short";

    success = _d->hasValue<unsigned short>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> unsigned short";

    success = _d->hasValue<unsigned short>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> unsigned short";

    success = _d->hasValue<unsigned short>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> unsigned short";

    success = _d->hasValue<unsigned short>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> unsigned short";

    success = _d->hasValue<unsigned short>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> unsigned short";

    success = _d->hasValue<unsigned short>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> unsigned short";

    success = _d->hasValue<unsigned short>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> unsigned short";

    success = _d->hasValue<unsigned short>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> unsigned short";

    success = _d->hasValue<unsigned short>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> unsigned short";

    success = _d->hasValue<int>("bool");
    EXPECT_EQ(true, success) << "bool -> int";

    success = _d->hasValue<int>("char");
    EXPECT_EQ(true, success) << "char -> int";

    success = _d->hasValue<int>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> int";

    success = _d->hasValue<int>("wchar_t");
    EXPECT_EQ(true, success) << "wchar_t -> int";

    success = _d->hasValue<int>("short");
    EXPECT_EQ(true, success) << "short -> int";

    success = _d->hasValue<int>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> int";

    success = _d->hasValue<int>("int");
    EXPECT_EQ(true, success) << "int -> int";

    success = _d->hasValue<int>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> int";

    success = _d->hasValue<int>("long long");
    EXPECT_EQ(true, success) << "long long -> int";

    success = _d->hasValue<int>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> int";

    success = _d->hasValue<int>("float");
    EXPECT_EQ(false, success) << "float -> int";

    success = _d->hasValue<int>("double");
    EXPECT_EQ(false, success) << "double -> int";

    success = _d->hasValue<int>("long double");
    EXPECT_EQ(false, success) << "long double -> int";

    success = _d->hasValue<int>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> int";

    success = _d->hasValue<int>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> int";

    success = _d->hasValue<int>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> int";

    success = _d->hasValue<int>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> int";

    success = _d->hasValue<int>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> int";

    success = _d->hasValue<int>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> int";

    success = _d->hasValue<int>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> int";

    success = _d->hasValue<int>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> int";

    success = _d->hasValue<int>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> int";

    success = _d->hasValue<int>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> int";

    success = _d->hasValue<int>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> int";

    success = _d->hasValue<int>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> int";

    success = _d->hasValue<int>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> int";

    success = _d->hasValue<int>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> int";

    success = _d->hasValue<int>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> int";

    success = _d->hasValue<int>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> int";

    success = _d->hasValue<int>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> int";

    success = _d->hasValue<int>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> int";

    success = _d->hasValue<int>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> int";

    success = _d->hasValue<int>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> int";

    success = _d->hasValue<int>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> int";

    success = _d->hasValue<int>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> int";

    success = _d->hasValue<int>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> int";

    success = _d->hasValue<int>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> int";

    success = _d->hasValue<int>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> int";

    success = _d->hasValue<int>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> int";

    success = _d->hasValue<int>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> int";

    success = _d->hasValue<int>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> int";

    success = _d->hasValue<int>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> int";

    success = _d->hasValue<int>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> int";

    success = _d->hasValue<int>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> int";

    success = _d->hasValue<int>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> int";

    success = _d->hasValue<int>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> int";

    success = _d->hasValue<int>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> int";

    success = _d->hasValue<unsigned int>("bool");
    EXPECT_EQ(false, success) << "bool -> unsigned int";

    success = _d->hasValue<unsigned int>("char");
    EXPECT_EQ(false, success) << "char -> unsigned int";

    success = _d->hasValue<unsigned int>("unsigned char");
    EXPECT_EQ(true, success) << "unsigned char -> unsigned int";

    success = _d->hasValue<unsigned int>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> unsigned int";

    success = _d->hasValue<unsigned int>("short");
    EXPECT_EQ(false, success) << "short -> unsigned int";

    success = _d->hasValue<unsigned int>("unsigned short");
    EXPECT_EQ(true, success) << "unsigned short -> unsigned int";

    success = _d->hasValue<unsigned int>("int");
    EXPECT_EQ(false, success) << "int -> unsigned int";

    success = _d->hasValue<unsigned int>("unsigned int");
    EXPECT_EQ(true, success) << "unsigned int -> unsigned int";

    success = _d->hasValue<unsigned int>("long long");
    EXPECT_EQ(false, success) << "long long -> unsigned int";

    success = _d->hasValue<unsigned int>("unsigned long long");
    EXPECT_EQ(true, success) << "unsigned long long -> unsigned int";

    success = _d->hasValue<unsigned int>("float");
    EXPECT_EQ(false, success) << "float -> unsigned int";

    success = _d->hasValue<unsigned int>("double");
    EXPECT_EQ(false, success) << "double -> unsigned int";

    success = _d->hasValue<unsigned int>("long double");
    EXPECT_EQ(false, success) << "long double -> unsigned int";

    success = _d->hasValue<unsigned int>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> unsigned int";

    success = _d->hasValue<unsigned int>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> unsigned int";

    success = _d->hasValue<unsigned int>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> unsigned int";

    success = _d->hasValue<unsigned int>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> unsigned int";

    success = _d->hasValue<unsigned int>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> unsigned int";

    success = _d->hasValue<unsigned int>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> unsigned int";

    success = _d->hasValue<unsigned int>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> unsigned int";

    success = _d->hasValue<unsigned int>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> unsigned int";

    success = _d->hasValue<unsigned int>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> unsigned int";

    success = _d->hasValue<unsigned int>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> unsigned int";

    success = _d->hasValue<unsigned int>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> unsigned int";

    success = _d->hasValue<unsigned int>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> unsigned int";

    success = _d->hasValue<unsigned int>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> unsigned int";

    success = _d->hasValue<unsigned int>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> unsigned int";

    success = _d->hasValue<unsigned int>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> unsigned int";

    success = _d->hasValue<unsigned int>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> unsigned int";

    success = _d->hasValue<unsigned int>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> unsigned int";

    success = _d->hasValue<unsigned int>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> unsigned int";

    success = _d->hasValue<unsigned int>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> unsigned int";

    success = _d->hasValue<unsigned int>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> unsigned int";

    success = _d->hasValue<unsigned int>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> unsigned int";

    success = _d->hasValue<unsigned int>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> unsigned int";

    success = _d->hasValue<unsigned int>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> unsigned int";

    success = _d->hasValue<unsigned int>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> unsigned int";

    success = _d->hasValue<unsigned int>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> unsigned int";

    success = _d->hasValue<unsigned int>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> unsigned int";

    success = _d->hasValue<unsigned int>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> unsigned int";

    success = _d->hasValue<unsigned int>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> unsigned int";

    success = _d->hasValue<unsigned int>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> unsigned int";

    success = _d->hasValue<unsigned int>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> unsigned int";

    success = _d->hasValue<unsigned int>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> unsigned int";

    success = _d->hasValue<unsigned int>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> unsigned int";

    success = _d->hasValue<unsigned int>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> unsigned int";

    success = _d->hasValue<unsigned int>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> unsigned int";

    success = _d->hasValue<long long>("bool");
    EXPECT_EQ(true, success) << "bool -> long long";

    success = _d->hasValue<long long>("char");
    EXPECT_EQ(true, success) << "char -> long long";

    success = _d->hasValue<long long>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> long long";

    success = _d->hasValue<long long>("wchar_t");
    EXPECT_EQ(true, success) << "wchar_t -> long long";

    success = _d->hasValue<long long>("short");
    EXPECT_EQ(true, success) << "short -> long long";

    success = _d->hasValue<long long>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> long long";

    success = _d->hasValue<long long>("int");
    EXPECT_EQ(true, success) << "int -> long long";

    success = _d->hasValue<long long>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> long long";

    success = _d->hasValue<long long>("long long");
    EXPECT_EQ(true, success) << "long long -> long long";

    success = _d->hasValue<long long>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> long long";

    success = _d->hasValue<long long>("float");
    EXPECT_EQ(false, success) << "float -> long long";

    success = _d->hasValue<long long>("double");
    EXPECT_EQ(false, success) << "double -> long long";

    success = _d->hasValue<long long>("long double");
    EXPECT_EQ(false, success) << "long double -> long long";

    success = _d->hasValue<long long>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> long long";

    success = _d->hasValue<long long>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> long long";

    success = _d->hasValue<long long>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> long long";

    success = _d->hasValue<long long>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> long long";

    success = _d->hasValue<long long>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> long long";

    success = _d->hasValue<long long>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> long long";

    success = _d->hasValue<long long>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> long long";

    success = _d->hasValue<long long>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> long long";

    success = _d->hasValue<long long>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> long long";

    success = _d->hasValue<long long>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> long long";

    success = _d->hasValue<long long>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> long long";

    success = _d->hasValue<long long>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> long long";

    success = _d->hasValue<long long>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> long long";

    success = _d->hasValue<long long>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> long long";

    success = _d->hasValue<long long>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> long long";

    success = _d->hasValue<long long>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> long long";

    success = _d->hasValue<long long>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> long long";

    success = _d->hasValue<long long>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> long long";

    success = _d->hasValue<long long>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> long long";

    success = _d->hasValue<long long>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> long long";

    success = _d->hasValue<long long>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> long long";

    success = _d->hasValue<long long>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> long long";

    success = _d->hasValue<long long>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> long long";

    success = _d->hasValue<long long>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> long long";

    success = _d->hasValue<long long>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> long long";

    success = _d->hasValue<long long>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> long long";

    success = _d->hasValue<long long>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> long long";

    success = _d->hasValue<long long>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> long long";

    success = _d->hasValue<long long>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> long long";

    success = _d->hasValue<long long>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> long long";

    success = _d->hasValue<long long>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> long long";

    success = _d->hasValue<long long>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> long long";

    success = _d->hasValue<long long>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> long long";

    success = _d->hasValue<long long>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> long long";

    success = _d->hasValue<unsigned long long>("bool");
    EXPECT_EQ(false, success) << "bool -> unsigned long long";

    success = _d->hasValue<unsigned long long>("char");
    EXPECT_EQ(false, success) << "char -> unsigned long long";

    success = _d->hasValue<unsigned long long>("unsigned char");
    EXPECT_EQ(true, success) << "unsigned char -> unsigned long long";

    success = _d->hasValue<unsigned long long>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> unsigned long long";

    success = _d->hasValue<unsigned long long>("short");
    EXPECT_EQ(false, success) << "short -> unsigned long long";

    success = _d->hasValue<unsigned long long>("unsigned short");
    EXPECT_EQ(true, success) << "unsigned short -> unsigned long long";

    success = _d->hasValue<unsigned long long>("int");
    EXPECT_EQ(false, success) << "int -> unsigned long long";

    success = _d->hasValue<unsigned long long>("unsigned int");
    EXPECT_EQ(true, success) << "unsigned int -> unsigned long long";

    success = _d->hasValue<unsigned long long>("long long");
    EXPECT_EQ(false, success) << "long long -> unsigned long long";

    success = _d->hasValue<unsigned long long>("unsigned long long");
    EXPECT_EQ(true, success) << "unsigned long long -> unsigned long long";

    success = _d->hasValue<unsigned long long>("float");
    EXPECT_EQ(false, success) << "float -> unsigned long long";

    success = _d->hasValue<unsigned long long>("double");
    EXPECT_EQ(false, success) << "double -> unsigned long long";

    success = _d->hasValue<unsigned long long>("long double");
    EXPECT_EQ(false, success) << "long double -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> unsigned long long";

    success = _d->hasValue<unsigned long long>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> unsigned long long";

    success = _d->hasValue<unsigned long long>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> unsigned long long";

    success = _d->hasValue<float>("bool");
    EXPECT_EQ(false, success) << "bool -> float";

    success = _d->hasValue<float>("char");
    EXPECT_EQ(false, success) << "char -> float";

    success = _d->hasValue<float>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> float";

    success = _d->hasValue<float>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> float";

    success = _d->hasValue<float>("short");
    EXPECT_EQ(false, success) << "short -> float";

    success = _d->hasValue<float>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> float";

    success = _d->hasValue<float>("int");
    EXPECT_EQ(false, success) << "int -> float";

    success = _d->hasValue<float>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> float";

    success = _d->hasValue<float>("long long");
    EXPECT_EQ(false, success) << "long long -> float";

    success = _d->hasValue<float>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> float";

    success = _d->hasValue<float>("float");
    EXPECT_EQ(true, success) << "float -> float";

    success = _d->hasValue<float>("double");
    EXPECT_EQ(true, success) << "double -> float";

    success = _d->hasValue<float>("long double");
    EXPECT_EQ(false, success) << "long double -> float"; // this is not a mistake

    success = _d->hasValue<float>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> float";

    success = _d->hasValue<float>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> float";

    success = _d->hasValue<float>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> float";

    success = _d->hasValue<float>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> float";

    success = _d->hasValue<float>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> float";

    success = _d->hasValue<float>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> float";

    success = _d->hasValue<float>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> float";

    success = _d->hasValue<float>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> float";

    success = _d->hasValue<float>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> float";

    success = _d->hasValue<float>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> float";

    success = _d->hasValue<float>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> float";

    success = _d->hasValue<float>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> float";

    success = _d->hasValue<float>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> float";

    success = _d->hasValue<float>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> float";

    success = _d->hasValue<float>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> float";

    success = _d->hasValue<float>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> float";

    success = _d->hasValue<float>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> float";

    success = _d->hasValue<float>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> float";

    success = _d->hasValue<float>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> float";

    success = _d->hasValue<float>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> float";

    success = _d->hasValue<float>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> float";

    success = _d->hasValue<float>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> float";

    success = _d->hasValue<float>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> float";

    success = _d->hasValue<float>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> float";

    success = _d->hasValue<float>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> float";

    success = _d->hasValue<float>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> float";

    success = _d->hasValue<float>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> float";

    success = _d->hasValue<float>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> float";

    success = _d->hasValue<float>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> float";

    success = _d->hasValue<float>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> float";

    success = _d->hasValue<float>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> float";

    success = _d->hasValue<float>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> float";

    success = _d->hasValue<float>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> float";

    success = _d->hasValue<float>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> float";

    success = _d->hasValue<double>("bool");
    EXPECT_EQ(false, success) << "bool -> double";

    success = _d->hasValue<double>("char");
    EXPECT_EQ(false, success) << "char -> double";

    success = _d->hasValue<double>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> double";

    success = _d->hasValue<double>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> double";

    success = _d->hasValue<double>("short");
    EXPECT_EQ(false, success) << "short -> double";

    success = _d->hasValue<double>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> double";

    success = _d->hasValue<double>("int");
    EXPECT_EQ(false, success) << "int -> double";

    success = _d->hasValue<double>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> double";

    success = _d->hasValue<double>("long long");
    EXPECT_EQ(false, success) << "long long -> double";

    success = _d->hasValue<double>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> double";

    success = _d->hasValue<double>("float");
    EXPECT_EQ(true, success) << "float -> double";

    success = _d->hasValue<double>("double");
    EXPECT_EQ(true, success) << "double -> double";

    success = _d->hasValue<double>("long double");
    EXPECT_EQ(false, success) << "long double -> double"; // this is not a mistake

    success = _d->hasValue<double>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> double";

    success = _d->hasValue<double>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> double";

    success = _d->hasValue<double>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> double";

    success = _d->hasValue<double>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> double";

    success = _d->hasValue<double>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> double";

    success = _d->hasValue<double>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> double";

    success = _d->hasValue<double>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> double";

    success = _d->hasValue<double>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> double";

    success = _d->hasValue<double>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> double";

    success = _d->hasValue<double>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> double";

    success = _d->hasValue<double>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> double";

    success = _d->hasValue<double>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> double";

    success = _d->hasValue<double>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> double";

    success = _d->hasValue<double>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> double";

    success = _d->hasValue<double>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> double";

    success = _d->hasValue<double>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> double";

    success = _d->hasValue<double>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> double";

    success = _d->hasValue<double>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> double";

    success = _d->hasValue<double>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> double";

    success = _d->hasValue<double>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> double";

    success = _d->hasValue<double>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> double";

    success = _d->hasValue<double>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> double";

    success = _d->hasValue<double>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> double";

    success = _d->hasValue<double>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> double";

    success = _d->hasValue<double>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> double";

    success = _d->hasValue<double>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> double";

    success = _d->hasValue<double>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> double";

    success = _d->hasValue<double>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> double";

    success = _d->hasValue<double>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> double";

    success = _d->hasValue<double>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> double";

    success = _d->hasValue<double>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> double";

    success = _d->hasValue<double>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> double";

    success = _d->hasValue<double>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> double";

    success = _d->hasValue<double>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> double";

    success = _d->hasValue<long double>("bool");
    EXPECT_EQ(false, success) << "bool -> long double";

    success = _d->hasValue<long double>("char");
    EXPECT_EQ(false, success) << "char -> long double";

    success = _d->hasValue<long double>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> long double";

    success = _d->hasValue<long double>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> long double";

    success = _d->hasValue<long double>("short");
    EXPECT_EQ(false, success) << "short -> long double";

    success = _d->hasValue<long double>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> long double";

    success = _d->hasValue<long double>("int");
    EXPECT_EQ(false, success) << "int -> long double";

    success = _d->hasValue<long double>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> long double";

    success = _d->hasValue<long double>("long long");
    EXPECT_EQ(false, success) << "long long -> long double";

    success = _d->hasValue<long double>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> long double";

    success = _d->hasValue<long double>("float");
    EXPECT_EQ(false, success) << "float -> long double";

    success = _d->hasValue<long double>("double");
    EXPECT_EQ(false, success) << "double -> long double";

    success = _d->hasValue<long double>("long double");
    EXPECT_EQ(true, success) << "long double -> long double"; // this is not a mistake

    success = _d->hasValue<long double>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> long double";

    success = _d->hasValue<long double>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> long double";

    success = _d->hasValue<long double>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> long double";

    success = _d->hasValue<long double>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> long double";

    success = _d->hasValue<long double>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> long double";

    success = _d->hasValue<long double>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> long double";

    success = _d->hasValue<long double>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> long double";

    success = _d->hasValue<long double>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> long double";

    success = _d->hasValue<long double>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> long double";

    success = _d->hasValue<long double>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> long double";

    success = _d->hasValue<long double>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> long double";

    success = _d->hasValue<long double>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> long double";

    success = _d->hasValue<long double>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> long double";

    success = _d->hasValue<long double>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> long double";

    success = _d->hasValue<long double>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> long double";

    success = _d->hasValue<long double>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> long double";

    success = _d->hasValue<long double>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> long double";

    success = _d->hasValue<long double>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> long double";

    success = _d->hasValue<long double>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> long double";

    success = _d->hasValue<long double>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> long double";

    success = _d->hasValue<long double>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> long double";

    success = _d->hasValue<long double>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> long double";

    success = _d->hasValue<long double>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> long double";

    success = _d->hasValue<long double>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> long double";

    success = _d->hasValue<long double>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> long double";

    success = _d->hasValue<long double>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> long double";

    success = _d->hasValue<long double>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> long double";

    success = _d->hasValue<long double>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> long double";

    success = _d->hasValue<long double>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> long double";

    success = _d->hasValue<long double>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> long double";

    success = _d->hasValue<long double>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> long double";

    success = _d->hasValue<long double>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> long double";

    success = _d->hasValue<long double>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> long double";

    success = _d->hasValue<long double>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> long double";

    using glm::vec2;

    success = _d->hasValue<vec2>("bool");
    EXPECT_EQ(false, success) << "bool -> vec2";

    success = _d->hasValue<vec2>("char");
    EXPECT_EQ(false, success) << "char -> vec2";

    success = _d->hasValue<vec2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> vec2";

    success = _d->hasValue<vec2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> vec2";

    success = _d->hasValue<vec2>("short");
    EXPECT_EQ(false, success) << "short -> vec2";

    success = _d->hasValue<vec2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> vec2";

    success = _d->hasValue<vec2>("int");
    EXPECT_EQ(false, success) << "int -> vec2";

    success = _d->hasValue<vec2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> vec2";

    success = _d->hasValue<vec2>("long long");
    EXPECT_EQ(false, success) << "long long -> vec2";

    success = _d->hasValue<vec2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> vec2";

    success = _d->hasValue<vec2>("float");
    EXPECT_EQ(false, success) << "float -> vec2";

    success = _d->hasValue<vec2>("double");
    EXPECT_EQ(false, success) << "double -> vec2";

    success = _d->hasValue<vec2>("long double");
    EXPECT_EQ(false, success) << "long double -> vec2";

    success = _d->hasValue<vec2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> vec2";

    success = _d->hasValue<vec2>("vec2");
    EXPECT_EQ(true, success) << "vec2 -> vec2";

    success = _d->hasValue<vec2>("dvec2");
    EXPECT_EQ(true, success) << "dvec2 -> vec2";

    success = _d->hasValue<vec2>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> vec2";

    success = _d->hasValue<vec2>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> vec2";

    success = _d->hasValue<vec2>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> vec2";

    success = _d->hasValue<vec2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> vec2";

    success = _d->hasValue<vec2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> vec2";

    success = _d->hasValue<vec2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> vec2";

    success = _d->hasValue<vec2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> vec2";

    success = _d->hasValue<vec2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> vec2";

    success = _d->hasValue<vec2>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> vec2";

    success = _d->hasValue<vec2>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> vec2";

    success = _d->hasValue<vec2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> vec2";

    success = _d->hasValue<vec2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> vec2";

    success = _d->hasValue<vec2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> vec2";

    success = _d->hasValue<vec2>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> vec2";

    success = _d->hasValue<vec2>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> vec2";

    success = _d->hasValue<vec2>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> vec2";

    success = _d->hasValue<vec2>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> vec2";

    success = _d->hasValue<vec2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> vec2";

    success = _d->hasValue<vec2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> vec2";

    success = _d->hasValue<vec2>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> vec2";

    success = _d->hasValue<vec2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> vec2";

    success = _d->hasValue<vec2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> vec2";

    success = _d->hasValue<vec2>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> vec2";

    success = _d->hasValue<vec2>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> vec2";

    success = _d->hasValue<vec2>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> vec2";

    success = _d->hasValue<vec2>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> vec2";

    success = _d->hasValue<vec2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> vec2";

    success = _d->hasValue<vec2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> vec2";

    success = _d->hasValue<vec2>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> vec2";

    success = _d->hasValue<vec2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> vec2";

    success = _d->hasValue<vec2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> vec2";

    using glm::dvec2;

    success = _d->hasValue<dvec2>("bool");
    EXPECT_EQ(false, success) << "bool -> dvec2";

    success = _d->hasValue<dvec2>("char");
    EXPECT_EQ(false, success) << "char -> dvec2";

    success = _d->hasValue<dvec2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dvec2";

    success = _d->hasValue<dvec2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dvec2";

    success = _d->hasValue<dvec2>("short");
    EXPECT_EQ(false, success) << "short -> dvec2";

    success = _d->hasValue<dvec2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dvec2";

    success = _d->hasValue<dvec2>("int");
    EXPECT_EQ(false, success) << "int -> dvec2";

    success = _d->hasValue<dvec2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dvec2";

    success = _d->hasValue<dvec2>("long long");
    EXPECT_EQ(false, success) << "long long -> dvec2";

    success = _d->hasValue<dvec2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dvec2";

    success = _d->hasValue<dvec2>("float");
    EXPECT_EQ(false, success) << "float -> dvec2";

    success = _d->hasValue<dvec2>("double");
    EXPECT_EQ(false, success) << "double -> dvec2";

    success = _d->hasValue<dvec2>("long double");
    EXPECT_EQ(false, success) << "long double -> dvec2";

    success = _d->hasValue<dvec2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dvec2";

    success = _d->hasValue<dvec2>("vec2");
    EXPECT_EQ(true, success) << "vec2 -> dvec2";

    success = _d->hasValue<dvec2>("dvec2");
    EXPECT_EQ(true, success) << "dvec2 -> dvec2";

    success = _d->hasValue<dvec2>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> dvec2";

    success = _d->hasValue<dvec2>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dvec2";

    success = _d->hasValue<dvec2>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dvec2";

    success = _d->hasValue<dvec2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dvec2";

    success = _d->hasValue<dvec2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dvec2";

    success = _d->hasValue<dvec2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dvec2";

    success = _d->hasValue<dvec2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dvec2";

    success = _d->hasValue<dvec2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dvec2";

    success = _d->hasValue<dvec2>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> dvec2";

    success = _d->hasValue<dvec2>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> dvec2";

    success = _d->hasValue<dvec2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dvec2";

    success = _d->hasValue<dvec2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dvec2";

    success = _d->hasValue<dvec2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dvec2";

    success = _d->hasValue<dvec2>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> dvec2";

    success = _d->hasValue<dvec2>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> dvec2";

    success = _d->hasValue<dvec2>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> dvec2";

    success = _d->hasValue<dvec2>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> dvec2";

    success = _d->hasValue<dvec2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dvec2";

    success = _d->hasValue<dvec2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> dvec2";

    success = _d->hasValue<dvec2>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> dvec2";

    success = _d->hasValue<dvec2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> dvec2";

    success = _d->hasValue<dvec2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dvec2";

    success = _d->hasValue<dvec2>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> dvec2";

    success = _d->hasValue<dvec2>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> dvec2";

    success = _d->hasValue<dvec2>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> dvec2";

    success = _d->hasValue<dvec2>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> dvec2";

    success = _d->hasValue<dvec2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dvec2";

    success = _d->hasValue<dvec2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> dvec2";

    success = _d->hasValue<dvec2>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> dvec2";

    success = _d->hasValue<dvec2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> dvec2";

    success = _d->hasValue<dvec2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dvec2";

    using glm::ivec2;

    success = _d->hasValue<ivec2>("bool");
    EXPECT_EQ(false, success) << "bool -> ivec2";

    success = _d->hasValue<ivec2>("char");
    EXPECT_EQ(false, success) << "char -> ivec2";

    success = _d->hasValue<ivec2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> ivec2";

    success = _d->hasValue<ivec2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> ivec2";

    success = _d->hasValue<ivec2>("short");
    EXPECT_EQ(false, success) << "short -> ivec2";

    success = _d->hasValue<ivec2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> ivec2";

    success = _d->hasValue<ivec2>("int");
    EXPECT_EQ(false, success) << "int -> ivec2";

    success = _d->hasValue<ivec2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> ivec2";

    success = _d->hasValue<ivec2>("long long");
    EXPECT_EQ(false, success) << "long long -> ivec2";

    success = _d->hasValue<ivec2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> ivec2";

    success = _d->hasValue<ivec2>("float");
    EXPECT_EQ(false, success) << "float -> ivec2";

    success = _d->hasValue<ivec2>("double");
    EXPECT_EQ(false, success) << "double -> ivec2";

    success = _d->hasValue<ivec2>("long double");
    EXPECT_EQ(false, success) << "long double -> ivec2";

    success = _d->hasValue<ivec2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> ivec2";

    success = _d->hasValue<ivec2>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> ivec2";

    success = _d->hasValue<ivec2>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> ivec2";

    success = _d->hasValue<ivec2>("ivec2");
    EXPECT_EQ(true, success) << "ivec2 -> ivec2";

    success = _d->hasValue<ivec2>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> ivec2";

    success = _d->hasValue<ivec2>("bvec2");
    EXPECT_EQ(true, success) << "bvec2 -> ivec2";

    success = _d->hasValue<ivec2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> ivec2";

    success = _d->hasValue<ivec2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> ivec2";

    success = _d->hasValue<ivec2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> ivec2";

    success = _d->hasValue<ivec2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> ivec2";

    success = _d->hasValue<ivec2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> ivec2";

    success = _d->hasValue<ivec2>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> ivec2";

    success = _d->hasValue<ivec2>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> ivec2";

    success = _d->hasValue<ivec2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> ivec2";

    success = _d->hasValue<ivec2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> ivec2";

    success = _d->hasValue<ivec2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> ivec2";

    success = _d->hasValue<ivec2>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> ivec2";

    success = _d->hasValue<ivec2>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> ivec2";

    success = _d->hasValue<ivec2>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> ivec2";

    success = _d->hasValue<ivec2>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> ivec2";

    success = _d->hasValue<ivec2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> ivec2";

    success = _d->hasValue<ivec2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> ivec2";

    success = _d->hasValue<ivec2>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> ivec2";

    success = _d->hasValue<ivec2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> ivec2";

    success = _d->hasValue<ivec2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> ivec2";

    success = _d->hasValue<ivec2>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> ivec2";

    success = _d->hasValue<ivec2>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> ivec2";

    success = _d->hasValue<ivec2>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> ivec2";

    success = _d->hasValue<ivec2>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> ivec2";

    success = _d->hasValue<ivec2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> ivec2";

    success = _d->hasValue<ivec2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> ivec2";

    success = _d->hasValue<ivec2>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> ivec2";

    success = _d->hasValue<ivec2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> ivec2";

    success = _d->hasValue<ivec2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> ivec2";

    using glm::uvec2;

    success = _d->hasValue<uvec2>("bool");
    EXPECT_EQ(false, success) << "bool -> uvec2";

    success = _d->hasValue<uvec2>("char");
    EXPECT_EQ(false, success) << "char -> uvec2";

    success = _d->hasValue<uvec2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> uvec2";

    success = _d->hasValue<uvec2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> uvec2";

    success = _d->hasValue<uvec2>("short");
    EXPECT_EQ(false, success) << "short -> uvec2";

    success = _d->hasValue<uvec2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> uvec2";

    success = _d->hasValue<uvec2>("int");
    EXPECT_EQ(false, success) << "int -> uvec2";

    success = _d->hasValue<uvec2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> uvec2";

    success = _d->hasValue<uvec2>("long long");
    EXPECT_EQ(false, success) << "long long -> uvec2";

    success = _d->hasValue<uvec2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> uvec2";

    success = _d->hasValue<uvec2>("float");
    EXPECT_EQ(false, success) << "float -> uvec2";

    success = _d->hasValue<uvec2>("double");
    EXPECT_EQ(false, success) << "double -> uvec2";

    success = _d->hasValue<uvec2>("long double");
    EXPECT_EQ(false, success) << "long double -> uvec2";

    success = _d->hasValue<uvec2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> uvec2";

    success = _d->hasValue<uvec2>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> uvec2";

    success = _d->hasValue<uvec2>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> uvec2";

    success = _d->hasValue<uvec2>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> uvec2";

    success = _d->hasValue<uvec2>("uvec2");
    EXPECT_EQ(true, success) << "uvec2 -> uvec2";

    success = _d->hasValue<uvec2>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> uvec2";

    success = _d->hasValue<uvec2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> uvec2";

    success = _d->hasValue<uvec2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> uvec2";

    success = _d->hasValue<uvec2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> uvec2";

    success = _d->hasValue<uvec2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> uvec2";

    success = _d->hasValue<uvec2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> uvec2";

    success = _d->hasValue<uvec2>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> uvec2";

    success = _d->hasValue<uvec2>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> uvec2";

    success = _d->hasValue<uvec2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> uvec2";

    success = _d->hasValue<uvec2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> uvec2";

    success = _d->hasValue<uvec2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> uvec2";

    success = _d->hasValue<uvec2>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> uvec2";

    success = _d->hasValue<uvec2>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> uvec2";

    success = _d->hasValue<uvec2>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> uvec2";

    success = _d->hasValue<uvec2>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> uvec2";

    success = _d->hasValue<uvec2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> uvec2";

    success = _d->hasValue<uvec2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> uvec2";

    success = _d->hasValue<uvec2>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> uvec2";

    success = _d->hasValue<uvec2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> uvec2";

    success = _d->hasValue<uvec2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> uvec2";

    success = _d->hasValue<uvec2>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> uvec2";

    success = _d->hasValue<uvec2>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> uvec2";

    success = _d->hasValue<uvec2>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> uvec2";

    success = _d->hasValue<uvec2>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> uvec2";

    success = _d->hasValue<uvec2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> uvec2";

    success = _d->hasValue<uvec2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> uvec2";

    success = _d->hasValue<uvec2>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> uvec2";

    success = _d->hasValue<uvec2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> uvec2";

    success = _d->hasValue<uvec2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> uvec2";

    using glm::bvec2;

    success = _d->hasValue<bvec2>("bool");
    EXPECT_EQ(false, success) << "bool -> bvec2";

    success = _d->hasValue<bvec2>("char");
    EXPECT_EQ(false, success) << "char -> bvec2";

    success = _d->hasValue<bvec2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> bvec2";

    success = _d->hasValue<bvec2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> bvec2";

    success = _d->hasValue<bvec2>("short");
    EXPECT_EQ(false, success) << "short -> bvec2";

    success = _d->hasValue<bvec2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> bvec2";

    success = _d->hasValue<bvec2>("int");
    EXPECT_EQ(false, success) << "int -> bvec2";

    success = _d->hasValue<bvec2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> bvec2";

    success = _d->hasValue<bvec2>("long long");
    EXPECT_EQ(false, success) << "long long -> bvec2";

    success = _d->hasValue<bvec2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> bvec2";

    success = _d->hasValue<bvec2>("float");
    EXPECT_EQ(false, success) << "float -> bvec2";

    success = _d->hasValue<bvec2>("double");
    EXPECT_EQ(false, success) << "double -> bvec2";

    success = _d->hasValue<bvec2>("long double");
    EXPECT_EQ(false, success) << "long double -> bvec2";

    success = _d->hasValue<bvec2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> bvec2";

    success = _d->hasValue<bvec2>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> bvec2";

    success = _d->hasValue<bvec2>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> bvec2";

    success = _d->hasValue<bvec2>("bvec2");
    EXPECT_EQ(true, success) << "ivec2 -> bvec2";

    success = _d->hasValue<bvec2>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> bvec2";

    success = _d->hasValue<bvec2>("bvec2");
    EXPECT_EQ(true, success) << "bvec2 -> bvec2";

    success = _d->hasValue<bvec2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> bvec2";

    success = _d->hasValue<bvec2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> bvec2";

    success = _d->hasValue<bvec2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> bvec2";

    success = _d->hasValue<bvec2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> bvec2";

    success = _d->hasValue<bvec2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> bvec2";

    success = _d->hasValue<bvec2>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> bvec2";

    success = _d->hasValue<bvec2>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> bvec2";

    success = _d->hasValue<bvec2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> bvec2";

    success = _d->hasValue<bvec2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> bvec2";

    success = _d->hasValue<bvec2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> bvec2";

    success = _d->hasValue<bvec2>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> bvec2";

    success = _d->hasValue<bvec2>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> bvec2";

    success = _d->hasValue<bvec2>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> bvec2";

    success = _d->hasValue<bvec2>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> bvec2";

    success = _d->hasValue<bvec2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> bvec2";

    success = _d->hasValue<bvec2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> bvec2";

    success = _d->hasValue<bvec2>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> bvec2";

    success = _d->hasValue<bvec2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> bvec2";

    success = _d->hasValue<bvec2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> bvec2";

    success = _d->hasValue<bvec2>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> bvec2";

    success = _d->hasValue<bvec2>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> bvec2";

    success = _d->hasValue<bvec2>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> bvec2";

    success = _d->hasValue<bvec2>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> bvec2";

    success = _d->hasValue<bvec2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> bvec2";

    success = _d->hasValue<bvec2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> bvec2";

    success = _d->hasValue<bvec2>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> bvec2";

    success = _d->hasValue<bvec2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> bvec2";

    success = _d->hasValue<bvec2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> bvec2";

    using glm::vec3;

    success = _d->hasValue<vec3>("bool");
    EXPECT_EQ(false, success) << "bool -> vec3";

    success = _d->hasValue<vec3>("char");
    EXPECT_EQ(false, success) << "char -> vec3";

    success = _d->hasValue<vec3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> vec3";

    success = _d->hasValue<vec3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> vec3";

    success = _d->hasValue<vec3>("short");
    EXPECT_EQ(false, success) << "short -> vec3";

    success = _d->hasValue<vec3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> vec3";

    success = _d->hasValue<vec3>("int");
    EXPECT_EQ(false, success) << "int -> vec3";

    success = _d->hasValue<vec3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> vec3";

    success = _d->hasValue<vec3>("long long");
    EXPECT_EQ(false, success) << "long long -> vec3";

    success = _d->hasValue<vec3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> vec3";

    success = _d->hasValue<vec3>("float");
    EXPECT_EQ(false, success) << "float -> vec3";

    success = _d->hasValue<vec3>("double");
    EXPECT_EQ(false, success) << "double -> vec3";

    success = _d->hasValue<vec3>("long double");
    EXPECT_EQ(false, success) << "long double -> vec3";

    success = _d->hasValue<vec3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> vec3";

    success = _d->hasValue<vec3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> vec3";

    success = _d->hasValue<vec3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> vec3";

    success = _d->hasValue<vec3>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> vec3";

    success = _d->hasValue<vec3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> vec3";

    success = _d->hasValue<vec3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> vec3";

    success = _d->hasValue<vec3>("vec3");
    EXPECT_EQ(true, success) << "vec3 -> vec3";

    success = _d->hasValue<vec3>("dvec3");
    EXPECT_EQ(true, success) << "dvec3 -> vec3";

    success = _d->hasValue<vec3>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> vec3";

    success = _d->hasValue<vec3>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> vec3";

    success = _d->hasValue<vec3>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> vec3";

    success = _d->hasValue<vec3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> vec3";

    success = _d->hasValue<vec3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> vec3";

    success = _d->hasValue<vec3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> vec3";

    success = _d->hasValue<vec3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> vec3";

    success = _d->hasValue<vec3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> vec3";

    success = _d->hasValue<vec3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> vec3";

    success = _d->hasValue<vec3>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> vec3";

    success = _d->hasValue<vec3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> vec3";

    success = _d->hasValue<vec3>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> vec3";

    success = _d->hasValue<vec3>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> vec3";

    success = _d->hasValue<vec3>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> vec3";

    success = _d->hasValue<vec3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> vec3";

    success = _d->hasValue<vec3>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> vec3";

    success = _d->hasValue<vec3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> vec3";

    success = _d->hasValue<vec3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> vec3";

    success = _d->hasValue<vec3>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> vec3";

    success = _d->hasValue<vec3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> vec3";

    success = _d->hasValue<vec3>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> vec3";

    success = _d->hasValue<vec3>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> vec3";

    success = _d->hasValue<vec3>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> vec3";

    success = _d->hasValue<vec3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> vec3";

    success = _d->hasValue<vec3>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> vec3";

    success = _d->hasValue<vec3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> vec3";

    using glm::dvec3;

    success = _d->hasValue<dvec3>("bool");
    EXPECT_EQ(false, success) << "bool -> dvec3";

    success = _d->hasValue<dvec3>("char");
    EXPECT_EQ(false, success) << "char -> dvec3";

    success = _d->hasValue<dvec3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dvec3";

    success = _d->hasValue<dvec3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dvec3";

    success = _d->hasValue<dvec3>("short");
    EXPECT_EQ(false, success) << "short -> dvec3";

    success = _d->hasValue<dvec3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dvec3";

    success = _d->hasValue<dvec3>("int");
    EXPECT_EQ(false, success) << "int -> dvec3";

    success = _d->hasValue<dvec3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dvec3";

    success = _d->hasValue<dvec3>("long long");
    EXPECT_EQ(false, success) << "long long -> dvec3";

    success = _d->hasValue<dvec3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dvec3";

    success = _d->hasValue<dvec3>("float");
    EXPECT_EQ(false, success) << "float -> dvec3";

    success = _d->hasValue<dvec3>("double");
    EXPECT_EQ(false, success) << "double -> dvec3";

    success = _d->hasValue<dvec3>("long double");
    EXPECT_EQ(false, success) << "long double -> dvec3";

    success = _d->hasValue<dvec3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dvec3";

    success = _d->hasValue<dvec3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dvec3";

    success = _d->hasValue<dvec3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dvec3";

    success = _d->hasValue<dvec3>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> dvec3";

    success = _d->hasValue<dvec3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dvec3";

    success = _d->hasValue<dvec3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dvec3";

    success = _d->hasValue<dvec3>("vec3");
    EXPECT_EQ(true, success) << "vec3 -> dvec3";

    success = _d->hasValue<dvec3>("dvec3");
    EXPECT_EQ(true, success) << "dvec3 -> dvec3";

    success = _d->hasValue<dvec3>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dvec3";

    success = _d->hasValue<dvec3>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dvec3";

    success = _d->hasValue<dvec3>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dvec3";

    success = _d->hasValue<dvec3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> dvec3";

    success = _d->hasValue<dvec3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> dvec3";

    success = _d->hasValue<dvec3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dvec3";

    success = _d->hasValue<dvec3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dvec3";

    success = _d->hasValue<dvec3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dvec3";

    success = _d->hasValue<dvec3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> dvec3";

    success = _d->hasValue<dvec3>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> dvec3";

    success = _d->hasValue<dvec3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> dvec3";

    success = _d->hasValue<dvec3>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> dvec3";

    success = _d->hasValue<dvec3>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dvec3";

    success = _d->hasValue<dvec3>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> dvec3";

    success = _d->hasValue<dvec3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> dvec3";

    success = _d->hasValue<dvec3>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> dvec3";

    success = _d->hasValue<dvec3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dvec3";

    success = _d->hasValue<dvec3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> dvec3";

    success = _d->hasValue<dvec3>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> dvec3";

    success = _d->hasValue<dvec3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> dvec3";

    success = _d->hasValue<dvec3>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> dvec3";

    success = _d->hasValue<dvec3>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dvec3";

    success = _d->hasValue<dvec3>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> dvec3";

    success = _d->hasValue<dvec3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> dvec3";

    success = _d->hasValue<dvec3>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> dvec3";

    success = _d->hasValue<dvec3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dvec3";

    using glm::ivec3;

    success = _d->hasValue<ivec3>("bool");
    EXPECT_EQ(false, success) << "bool -> ivec3";

    success = _d->hasValue<ivec3>("char");
    EXPECT_EQ(false, success) << "char -> ivec3";

    success = _d->hasValue<ivec3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> ivec3";

    success = _d->hasValue<ivec3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> ivec3";

    success = _d->hasValue<ivec3>("short");
    EXPECT_EQ(false, success) << "short -> ivec3";

    success = _d->hasValue<ivec3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> ivec3";

    success = _d->hasValue<ivec3>("int");
    EXPECT_EQ(false, success) << "int -> ivec3";

    success = _d->hasValue<ivec3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> ivec3";

    success = _d->hasValue<ivec3>("long long");
    EXPECT_EQ(false, success) << "long long -> ivec3";

    success = _d->hasValue<ivec3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> ivec3";

    success = _d->hasValue<ivec3>("float");
    EXPECT_EQ(false, success) << "float -> ivec3";

    success = _d->hasValue<ivec3>("double");
    EXPECT_EQ(false, success) << "double -> ivec3";

    success = _d->hasValue<ivec3>("long double");
    EXPECT_EQ(false, success) << "long double -> ivec3";

    success = _d->hasValue<ivec3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> ivec3";

    success = _d->hasValue<ivec3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> ivec3";

    success = _d->hasValue<ivec3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> ivec3";

    success = _d->hasValue<ivec3>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> ivec3";

    success = _d->hasValue<ivec3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> ivec3";

    success = _d->hasValue<ivec3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> ivec3";

    success = _d->hasValue<ivec3>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> ivec3";

    success = _d->hasValue<ivec3>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> ivec3";

    success = _d->hasValue<ivec3>("ivec3");
    EXPECT_EQ(true, success) << "ivec3 -> ivec3";

    success = _d->hasValue<ivec3>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> ivec3";

    success = _d->hasValue<ivec3>("bvec3");
    EXPECT_EQ(true, success) << "bvec3 -> ivec3";

    success = _d->hasValue<ivec3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> ivec3";

    success = _d->hasValue<ivec3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> ivec3";

    success = _d->hasValue<ivec3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> ivec3";

    success = _d->hasValue<ivec3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> ivec3";

    success = _d->hasValue<ivec3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> ivec3";

    success = _d->hasValue<ivec3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> ivec3";

    success = _d->hasValue<ivec3>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> ivec3";

    success = _d->hasValue<ivec3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> ivec3";

    success = _d->hasValue<ivec3>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> ivec3";

    success = _d->hasValue<ivec3>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> ivec3";

    success = _d->hasValue<ivec3>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> ivec3";

    success = _d->hasValue<ivec3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> ivec3";

    success = _d->hasValue<ivec3>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> ivec3";

    success = _d->hasValue<ivec3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> ivec3";

    success = _d->hasValue<ivec3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> ivec3";

    success = _d->hasValue<ivec3>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> ivec3";

    success = _d->hasValue<ivec3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> ivec3";

    success = _d->hasValue<ivec3>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> ivec3";

    success = _d->hasValue<ivec3>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> ivec3";

    success = _d->hasValue<ivec3>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> ivec3";

    success = _d->hasValue<ivec3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> ivec3";

    success = _d->hasValue<ivec3>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> ivec3";

    success = _d->hasValue<ivec3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> ivec3";

    using glm::uvec3;

    success = _d->hasValue<uvec3>("bool");
    EXPECT_EQ(false, success) << "bool -> uvec3";

    success = _d->hasValue<uvec3>("char");
    EXPECT_EQ(false, success) << "char -> uvec3";

    success = _d->hasValue<uvec3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> uvec3";

    success = _d->hasValue<uvec3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> uvec3";

    success = _d->hasValue<uvec3>("short");
    EXPECT_EQ(false, success) << "short -> uvec3";

    success = _d->hasValue<uvec3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> uvec3";

    success = _d->hasValue<uvec3>("int");
    EXPECT_EQ(false, success) << "int -> uvec3";

    success = _d->hasValue<uvec3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> uvec3";

    success = _d->hasValue<uvec3>("long long");
    EXPECT_EQ(false, success) << "long long -> uvec3";

    success = _d->hasValue<uvec3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> uvec3";

    success = _d->hasValue<uvec3>("float");
    EXPECT_EQ(false, success) << "float -> uvec3";

    success = _d->hasValue<uvec3>("double");
    EXPECT_EQ(false, success) << "double -> uvec3";

    success = _d->hasValue<uvec3>("long double");
    EXPECT_EQ(false, success) << "long double -> uvec3";

    success = _d->hasValue<uvec3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> uvec3";

    success = _d->hasValue<uvec3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> uvec3";

    success = _d->hasValue<uvec3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> uvec3";

    success = _d->hasValue<uvec3>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> uvec3";

    success = _d->hasValue<uvec3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> uvec3";

    success = _d->hasValue<uvec3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> uvec3";

    success = _d->hasValue<uvec3>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> uvec3";

    success = _d->hasValue<uvec3>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> uvec3";

    success = _d->hasValue<uvec3>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> uvec3";

    success = _d->hasValue<uvec3>("uvec3");
    EXPECT_EQ(true, success) << "uvec3 -> uvec3";

    success = _d->hasValue<uvec3>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> uvec3";

    success = _d->hasValue<uvec3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> uvec3";

    success = _d->hasValue<uvec3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> uvec3";

    success = _d->hasValue<uvec3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> uvec3";

    success = _d->hasValue<uvec3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> uvec3";

    success = _d->hasValue<uvec3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> uvec3";

    success = _d->hasValue<uvec3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> uvec3";

    success = _d->hasValue<uvec3>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> uvec3";

    success = _d->hasValue<uvec3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> uvec3";

    success = _d->hasValue<uvec3>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> uvec3";

    success = _d->hasValue<uvec3>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> uvec3";

    success = _d->hasValue<uvec3>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> uvec3";

    success = _d->hasValue<uvec3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> uvec3";

    success = _d->hasValue<uvec3>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> uvec3";

    success = _d->hasValue<uvec3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> uvec3";

    success = _d->hasValue<uvec3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> uvec3";

    success = _d->hasValue<uvec3>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> uvec3";

    success = _d->hasValue<uvec3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> uvec3";

    success = _d->hasValue<uvec3>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> uvec3";

    success = _d->hasValue<uvec3>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> uvec3";

    success = _d->hasValue<uvec3>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> uvec3";

    success = _d->hasValue<uvec3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> uvec3";

    success = _d->hasValue<uvec3>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> uvec3";

    success = _d->hasValue<uvec3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> uvec3";

    using glm::bvec3;

    success = _d->hasValue<bvec3>("bool");
    EXPECT_EQ(false, success) << "bool -> bvec3";

    success = _d->hasValue<bvec3>("char");
    EXPECT_EQ(false, success) << "char -> bvec3";

    success = _d->hasValue<bvec3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> bvec3";

    success = _d->hasValue<bvec3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> bvec3";

    success = _d->hasValue<bvec3>("short");
    EXPECT_EQ(false, success) << "short -> bvec3";

    success = _d->hasValue<bvec3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> bvec3";

    success = _d->hasValue<bvec3>("int");
    EXPECT_EQ(false, success) << "int -> bvec3";

    success = _d->hasValue<bvec3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> bvec3";

    success = _d->hasValue<bvec3>("long long");
    EXPECT_EQ(false, success) << "long long -> bvec3";

    success = _d->hasValue<bvec3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> bvec3";

    success = _d->hasValue<bvec3>("float");
    EXPECT_EQ(false, success) << "float -> bvec3";

    success = _d->hasValue<bvec3>("double");
    EXPECT_EQ(false, success) << "double -> bvec3";

    success = _d->hasValue<bvec3>("long double");
    EXPECT_EQ(false, success) << "long double -> bvec3";

    success = _d->hasValue<bvec3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> bvec3";

    success = _d->hasValue<bvec3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> bvec3";

    success = _d->hasValue<bvec3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> bvec3";

    success = _d->hasValue<bvec3>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> bvec3";

    success = _d->hasValue<bvec3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> bvec3";

    success = _d->hasValue<bvec3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> bvec3";

    success = _d->hasValue<bvec3>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> bvec3";

    success = _d->hasValue<bvec3>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> bvec3";

    success = _d->hasValue<bvec3>("ivec3");
    EXPECT_EQ(true, success) << "ivec3 -> bvec3";

    success = _d->hasValue<bvec3>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> bvec3";

    success = _d->hasValue<bvec3>("bvec3");
    EXPECT_EQ(true, success) << "bvec3 -> bvec3";

    success = _d->hasValue<bvec3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> bvec3";

    success = _d->hasValue<bvec3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> bvec3";

    success = _d->hasValue<bvec3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> bvec3";

    success = _d->hasValue<bvec3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> bvec3";

    success = _d->hasValue<bvec3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> bvec3";

    success = _d->hasValue<bvec3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> bvec3";

    success = _d->hasValue<bvec3>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> bvec3";

    success = _d->hasValue<bvec3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> bvec3";

    success = _d->hasValue<bvec3>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> bvec3";

    success = _d->hasValue<bvec3>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> bvec3";

    success = _d->hasValue<bvec3>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> bvec3";

    success = _d->hasValue<bvec3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> bvec3";

    success = _d->hasValue<bvec3>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> bvec3";

    success = _d->hasValue<bvec3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> bvec3";

    success = _d->hasValue<bvec3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> bvec3";

    success = _d->hasValue<bvec3>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> bvec3";

    success = _d->hasValue<bvec3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> bvec3";

    success = _d->hasValue<bvec3>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> bvec3";

    success = _d->hasValue<bvec3>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> bvec3";

    success = _d->hasValue<bvec3>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> bvec3";

    success = _d->hasValue<bvec3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> bvec3";

    success = _d->hasValue<bvec3>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> bvec3";

    success = _d->hasValue<bvec3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> bvec3";

    using glm::vec4;

    success = _d->hasValue<vec4>("bool");
    EXPECT_EQ(false, success) << "bool -> vec4";

    success = _d->hasValue<vec4>("char");
    EXPECT_EQ(false, success) << "char -> vec4";

    success = _d->hasValue<vec4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> vec4";

    success = _d->hasValue<vec4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> vec4";

    success = _d->hasValue<vec4>("short");
    EXPECT_EQ(false, success) << "short -> vec4";

    success = _d->hasValue<vec4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> vec4";

    success = _d->hasValue<vec4>("int");
    EXPECT_EQ(false, success) << "int -> vec4";

    success = _d->hasValue<vec4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> vec4";

    success = _d->hasValue<vec4>("long long");
    EXPECT_EQ(false, success) << "long long -> vec4";

    success = _d->hasValue<vec4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> vec4";

    success = _d->hasValue<vec4>("float");
    EXPECT_EQ(false, success) << "float -> vec4";

    success = _d->hasValue<vec4>("double");
    EXPECT_EQ(false, success) << "double -> vec4";

    success = _d->hasValue<vec4>("long double");
    EXPECT_EQ(false, success) << "long double -> vec4";

    success = _d->hasValue<vec4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> vec4";

    success = _d->hasValue<vec4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> vec4";

    success = _d->hasValue<vec4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> vec4";

    success = _d->hasValue<vec4>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> vec4";

    success = _d->hasValue<vec4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> vec4";

    success = _d->hasValue<vec4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> vec4";

    success = _d->hasValue<vec4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> vec4";

    success = _d->hasValue<vec4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> vec4";

    success = _d->hasValue<vec4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> vec4";

    success = _d->hasValue<vec4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> vec4";

    success = _d->hasValue<vec4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> vec4";

    success = _d->hasValue<vec4>("vec4");
    EXPECT_EQ(true, success) << "vec4 -> vec4";

    success = _d->hasValue<vec4>("dvec4");
    EXPECT_EQ(true, success) << "dvec4 -> vec4";

    success = _d->hasValue<vec4>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> vec4";

    success = _d->hasValue<vec4>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> vec4";

    success = _d->hasValue<vec4>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> vec4";

    success = _d->hasValue<vec4>("mat2x2");
    EXPECT_EQ(true, success) << "mat2x2 -> vec4";

    success = _d->hasValue<vec4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> vec4";

    success = _d->hasValue<vec4>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> vec4";

    success = _d->hasValue<vec4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> vec4";

    success = _d->hasValue<vec4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> vec4";

    success = _d->hasValue<vec4>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> vec4";

    success = _d->hasValue<vec4>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> vec4";

    success = _d->hasValue<vec4>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> vec4";

    success = _d->hasValue<vec4>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> vec4";

    success = _d->hasValue<vec4>("dmat2x2");
    EXPECT_EQ(true, success) << "dmat2x2 -> vec4";

    success = _d->hasValue<vec4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> vec4";

    success = _d->hasValue<vec4>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> vec4";

    success = _d->hasValue<vec4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> vec4";

    success = _d->hasValue<vec4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> vec4";

    success = _d->hasValue<vec4>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> vec4";

    success = _d->hasValue<vec4>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> vec4";

    success = _d->hasValue<vec4>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> vec4";

    success = _d->hasValue<vec4>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> vec4";

    using glm::dvec4;

    success = _d->hasValue<dvec4>("bool");
    EXPECT_EQ(false, success) << "bool -> dvec4";

    success = _d->hasValue<dvec4>("char");
    EXPECT_EQ(false, success) << "char -> dvec4";

    success = _d->hasValue<dvec4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dvec4";

    success = _d->hasValue<dvec4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dvec4";

    success = _d->hasValue<dvec4>("short");
    EXPECT_EQ(false, success) << "short -> dvec4";

    success = _d->hasValue<dvec4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dvec4";

    success = _d->hasValue<dvec4>("int");
    EXPECT_EQ(false, success) << "int -> dvec4";

    success = _d->hasValue<dvec4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dvec4";

    success = _d->hasValue<dvec4>("long long");
    EXPECT_EQ(false, success) << "long long -> dvec4";

    success = _d->hasValue<dvec4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dvec4";

    success = _d->hasValue<dvec4>("float");
    EXPECT_EQ(false, success) << "float -> dvec4";

    success = _d->hasValue<dvec4>("double");
    EXPECT_EQ(false, success) << "double -> dvec4";

    success = _d->hasValue<dvec4>("long double");
    EXPECT_EQ(false, success) << "long double -> dvec4";

    success = _d->hasValue<dvec4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dvec4";

    success = _d->hasValue<dvec4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dvec4";

    success = _d->hasValue<dvec4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dvec4";

    success = _d->hasValue<dvec4>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> dvec4";

    success = _d->hasValue<dvec4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dvec4";

    success = _d->hasValue<dvec4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dvec4";

    success = _d->hasValue<dvec4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dvec4";

    success = _d->hasValue<dvec4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dvec4";

    success = _d->hasValue<dvec4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dvec4";

    success = _d->hasValue<dvec4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dvec4";

    success = _d->hasValue<dvec4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dvec4";

    success = _d->hasValue<dvec4>("vec4");
    EXPECT_EQ(true, success) << "vec4 -> dvec4";

    success = _d->hasValue<dvec4>("dvec4");
    EXPECT_EQ(true, success) << "dvec4 -> dvec4";

    success = _d->hasValue<dvec4>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dvec4";

    success = _d->hasValue<dvec4>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dvec4";

    success = _d->hasValue<dvec4>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dvec4";

    success = _d->hasValue<dvec4>("mat2x2");
    EXPECT_EQ(true, success) << "mat2x2 -> dvec4";

    success = _d->hasValue<dvec4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> dvec4";

    success = _d->hasValue<dvec4>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> dvec4";

    success = _d->hasValue<dvec4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> dvec4";

    success = _d->hasValue<dvec4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dvec4";

    success = _d->hasValue<dvec4>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> dvec4";

    success = _d->hasValue<dvec4>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> dvec4";

    success = _d->hasValue<dvec4>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> dvec4";

    success = _d->hasValue<dvec4>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dvec4";

    success = _d->hasValue<dvec4>("dmat2x2");
    EXPECT_EQ(true, success) << "dmat2x2 -> dvec4";

    success = _d->hasValue<dvec4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> dvec4";

    success = _d->hasValue<dvec4>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> dvec4";

    success = _d->hasValue<dvec4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> dvec4";

    success = _d->hasValue<dvec4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dvec4";

    success = _d->hasValue<dvec4>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> dvec4";

    success = _d->hasValue<dvec4>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> dvec4";

    success = _d->hasValue<dvec4>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> dvec4";

    success = _d->hasValue<dvec4>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dvec4";

    using glm::ivec4;

    success = _d->hasValue<ivec4>("bool");
    EXPECT_EQ(false, success) << "bool -> ivec4";

    success = _d->hasValue<ivec4>("char");
    EXPECT_EQ(false, success) << "char -> ivec4";

    success = _d->hasValue<ivec4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> ivec4";

    success = _d->hasValue<ivec4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> ivec4";

    success = _d->hasValue<ivec4>("short");
    EXPECT_EQ(false, success) << "short -> ivec4";

    success = _d->hasValue<ivec4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> ivec4";

    success = _d->hasValue<ivec4>("int");
    EXPECT_EQ(false, success) << "int -> ivec4";

    success = _d->hasValue<ivec4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> ivec4";

    success = _d->hasValue<ivec4>("long long");
    EXPECT_EQ(false, success) << "long long -> ivec4";

    success = _d->hasValue<ivec4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> ivec4";

    success = _d->hasValue<ivec4>("float");
    EXPECT_EQ(false, success) << "float -> ivec4";

    success = _d->hasValue<ivec4>("double");
    EXPECT_EQ(false, success) << "double -> ivec4";

    success = _d->hasValue<ivec4>("long double");
    EXPECT_EQ(false, success) << "long double -> ivec4";

    success = _d->hasValue<ivec4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> ivec4";

    success = _d->hasValue<ivec4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> ivec4";

    success = _d->hasValue<ivec4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> ivec4";

    success = _d->hasValue<ivec4>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> ivec4";

    success = _d->hasValue<ivec4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> ivec4";

    success = _d->hasValue<ivec4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> ivec4";

    success = _d->hasValue<ivec4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> ivec4";

    success = _d->hasValue<ivec4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> ivec4";

    success = _d->hasValue<ivec4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> ivec4";

    success = _d->hasValue<ivec4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> ivec4";

    success = _d->hasValue<ivec4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> ivec4";

    success = _d->hasValue<ivec4>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> ivec4";

    success = _d->hasValue<ivec4>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> ivec4";

    success = _d->hasValue<ivec4>("ivec4");
    EXPECT_EQ(true, success) << "ivec4 -> ivec4";

    success = _d->hasValue<ivec4>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> ivec4";

    success = _d->hasValue<ivec4>("bvec4");
    EXPECT_EQ(true, success) << "bvec4 -> ivec4";

    success = _d->hasValue<ivec4>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> ivec4";

    success = _d->hasValue<ivec4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> ivec4";

    success = _d->hasValue<ivec4>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> ivec4";

    success = _d->hasValue<ivec4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> ivec4";

    success = _d->hasValue<ivec4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> ivec4";

    success = _d->hasValue<ivec4>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> ivec4";

    success = _d->hasValue<ivec4>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> ivec4";

    success = _d->hasValue<ivec4>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> ivec4";

    success = _d->hasValue<ivec4>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> ivec4";

    success = _d->hasValue<ivec4>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> ivec4";

    success = _d->hasValue<ivec4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> ivec4";

    success = _d->hasValue<ivec4>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> ivec4";

    success = _d->hasValue<ivec4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> ivec4";

    success = _d->hasValue<ivec4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> ivec4";

    success = _d->hasValue<ivec4>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> ivec4";

    success = _d->hasValue<ivec4>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> ivec4";

    success = _d->hasValue<ivec4>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> ivec4";

    success = _d->hasValue<ivec4>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> ivec4";

    using glm::uvec4;

    success = _d->hasValue<uvec4>("bool");
    EXPECT_EQ(false, success) << "bool -> uvec4";

    success = _d->hasValue<uvec4>("char");
    EXPECT_EQ(false, success) << "char -> uvec4";

    success = _d->hasValue<uvec4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> uvec4";

    success = _d->hasValue<uvec4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> uvec4";

    success = _d->hasValue<uvec4>("short");
    EXPECT_EQ(false, success) << "short -> uvec4";

    success = _d->hasValue<uvec4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> uvec4";

    success = _d->hasValue<uvec4>("int");
    EXPECT_EQ(false, success) << "int -> uvec4";

    success = _d->hasValue<uvec4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> uvec4";

    success = _d->hasValue<uvec4>("long long");
    EXPECT_EQ(false, success) << "long long -> uvec4";

    success = _d->hasValue<uvec4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> uvec4";

    success = _d->hasValue<uvec4>("float");
    EXPECT_EQ(false, success) << "float -> uvec4";

    success = _d->hasValue<uvec4>("double");
    EXPECT_EQ(false, success) << "double -> uvec4";

    success = _d->hasValue<uvec4>("long double");
    EXPECT_EQ(false, success) << "long double -> uvec4";

    success = _d->hasValue<uvec4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> uvec4";

    success = _d->hasValue<uvec4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> uvec4";

    success = _d->hasValue<uvec4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> uvec4";

    success = _d->hasValue<uvec4>("ivec2");
    EXPECT_EQ(false, success) << "ivec2 -> uvec4";

    success = _d->hasValue<uvec4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> uvec4";

    success = _d->hasValue<uvec4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> uvec4";

    success = _d->hasValue<uvec4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> uvec4";

    success = _d->hasValue<uvec4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> uvec4";

    success = _d->hasValue<uvec4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> uvec4";

    success = _d->hasValue<uvec4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> uvec4";

    success = _d->hasValue<uvec4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> uvec4";

    success = _d->hasValue<uvec4>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> uvec4";

    success = _d->hasValue<uvec4>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> uvec4";

    success = _d->hasValue<uvec4>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> uvec4";

    success = _d->hasValue<uvec4>("uvec4");
    EXPECT_EQ(true, success) << "uvec4 -> uvec4";

    success = _d->hasValue<uvec4>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> uvec4";

    success = _d->hasValue<uvec4>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> uvec4";

    success = _d->hasValue<uvec4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> uvec4";

    success = _d->hasValue<uvec4>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> uvec4";

    success = _d->hasValue<uvec4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> uvec4";

    success = _d->hasValue<uvec4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> uvec4";

    success = _d->hasValue<uvec4>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> uvec4";

    success = _d->hasValue<uvec4>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> uvec4";

    success = _d->hasValue<uvec4>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> uvec4";

    success = _d->hasValue<uvec4>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> uvec4";

    success = _d->hasValue<uvec4>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> uvec4";

    success = _d->hasValue<uvec4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> uvec4";

    success = _d->hasValue<uvec4>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> uvec4";

    success = _d->hasValue<uvec4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> uvec4";

    success = _d->hasValue<uvec4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> uvec4";

    success = _d->hasValue<uvec4>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> uvec4";

    success = _d->hasValue<uvec4>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> uvec4";

    success = _d->hasValue<uvec4>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> uvec4";

    success = _d->hasValue<uvec4>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> uvec4";

    using glm::bvec4;

    success = _d->hasValue<bvec4>("bool");
    EXPECT_EQ(false, success) << "bool -> bvec4";

    success = _d->hasValue<bvec4>("char");
    EXPECT_EQ(false, success) << "char -> bvec4";

    success = _d->hasValue<bvec4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> bvec4";

    success = _d->hasValue<bvec4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> bvec4";

    success = _d->hasValue<bvec4>("short");
    EXPECT_EQ(false, success) << "short -> bvec4";

    success = _d->hasValue<bvec4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> bvec4";

    success = _d->hasValue<bvec4>("int");
    EXPECT_EQ(false, success) << "int -> bvec4";

    success = _d->hasValue<bvec4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> bvec4";

    success = _d->hasValue<bvec4>("long long");
    EXPECT_EQ(false, success) << "long long -> bvec4";

    success = _d->hasValue<bvec4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> bvec4";

    success = _d->hasValue<bvec4>("float");
    EXPECT_EQ(false, success) << "float -> bvec4";

    success = _d->hasValue<bvec4>("double");
    EXPECT_EQ(false, success) << "double -> bvec4";

    success = _d->hasValue<bvec4>("long double");
    EXPECT_EQ(false, success) << "long double -> bvec4";

    success = _d->hasValue<bvec4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> bvec4";

    success = _d->hasValue<bvec4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> bvec4";

    success = _d->hasValue<bvec4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> bvec4";

    success = _d->hasValue<bvec4>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> bvec4";

    success = _d->hasValue<bvec4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> bvec4";

    success = _d->hasValue<bvec4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> bvec4";

    success = _d->hasValue<bvec4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> bvec4";

    success = _d->hasValue<bvec4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> bvec4";

    success = _d->hasValue<bvec4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> bvec4";

    success = _d->hasValue<bvec4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> bvec4";

    success = _d->hasValue<bvec4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> bvec4";

    success = _d->hasValue<bvec4>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> bvec4";

    success = _d->hasValue<bvec4>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> bvec4";

    success = _d->hasValue<bvec4>("ivec4");
    EXPECT_EQ(true, success) << "ivec4 -> bvec4";

    success = _d->hasValue<bvec4>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> bvec4";

    success = _d->hasValue<bvec4>("bvec4");
    EXPECT_EQ(true, success) << "bvec4 -> bvec4";

    success = _d->hasValue<bvec4>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> bvec4";

    success = _d->hasValue<bvec4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> bvec4";

    success = _d->hasValue<bvec4>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> bvec4";

    success = _d->hasValue<bvec4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> bvec4";

    success = _d->hasValue<bvec4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> bvec4";

    success = _d->hasValue<bvec4>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> bvec4";

    success = _d->hasValue<bvec4>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> bvec4";

    success = _d->hasValue<bvec4>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> bvec4";

    success = _d->hasValue<bvec4>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> bvec4";

    success = _d->hasValue<bvec4>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> bvec4";

    success = _d->hasValue<bvec4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> bvec4";

    success = _d->hasValue<bvec4>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> bvec4";

    success = _d->hasValue<bvec4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> bvec4";

    success = _d->hasValue<bvec4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> bvec4";

    success = _d->hasValue<bvec4>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> bvec4";

    success = _d->hasValue<bvec4>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> bvec4";

    success = _d->hasValue<bvec4>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> bvec4";

    success = _d->hasValue<bvec4>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> bvec4";

    using glm::mat2x2;

    success = _d->hasValue<mat2x2>("bool");
    EXPECT_EQ(false, success) << "bool -> mat2x2";

    success = _d->hasValue<mat2x2>("char");
    EXPECT_EQ(false, success) << "char -> mat2x2";

    success = _d->hasValue<mat2x2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> mat2x2";

    success = _d->hasValue<mat2x2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> mat2x2";

    success = _d->hasValue<mat2x2>("short");
    EXPECT_EQ(false, success) << "short -> mat2x2";

    success = _d->hasValue<mat2x2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> mat2x2";

    success = _d->hasValue<mat2x2>("int");
    EXPECT_EQ(false, success) << "int -> mat2x2";

    success = _d->hasValue<mat2x2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> mat2x2";

    success = _d->hasValue<mat2x2>("long long");
    EXPECT_EQ(false, success) << "long long -> mat2x2";

    success = _d->hasValue<mat2x2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> mat2x2";

    success = _d->hasValue<mat2x2>("float");
    EXPECT_EQ(false, success) << "float -> mat2x2";

    success = _d->hasValue<mat2x2>("double");
    EXPECT_EQ(false, success) << "double -> mat2x2";

    success = _d->hasValue<mat2x2>("long double");
    EXPECT_EQ(false, success) << "long double -> mat2x2";

    success = _d->hasValue<mat2x2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> mat2x2";

    success = _d->hasValue<mat2x2>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> mat2x2";

    success = _d->hasValue<mat2x2>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> mat2x2";

    success = _d->hasValue<mat2x2>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> mat2x2";

    success = _d->hasValue<mat2x2>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> mat2x2";

    success = _d->hasValue<mat2x2>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> mat2x2";

    success = _d->hasValue<mat2x2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> mat2x2";

    success = _d->hasValue<mat2x2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> mat2x2";

    success = _d->hasValue<mat2x2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> mat2x2";

    success = _d->hasValue<mat2x2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> mat2x2";

    success = _d->hasValue<mat2x2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> mat2x2";

    success = _d->hasValue<mat2x2>("vec4");
    EXPECT_EQ(true, success) << "vec4 -> mat2x2";

    success = _d->hasValue<mat2x2>("dvec4");
    EXPECT_EQ(true, success) << "dvec4 -> mat2x2";

    success = _d->hasValue<mat2x2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> mat2x2";

    success = _d->hasValue<mat2x2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> mat2x2";

    success = _d->hasValue<mat2x2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> mat2x2";

    success = _d->hasValue<mat2x2>("mat2x2");
    EXPECT_EQ(true, success) << "mat2x2 -> mat2x2";

    success = _d->hasValue<mat2x2>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> mat2x2";

    success = _d->hasValue<mat2x2>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> mat2x2";

    success = _d->hasValue<mat2x2>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> mat2x2";

    success = _d->hasValue<mat2x2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> mat2x2";

    success = _d->hasValue<mat2x2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> mat2x2";

    success = _d->hasValue<mat2x2>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> mat2x2";

    success = _d->hasValue<mat2x2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> mat2x2";

    success = _d->hasValue<mat2x2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> mat2x2";

    success = _d->hasValue<mat2x2>("dmat2x2");
    EXPECT_EQ(true, success) << "dmat2x2 -> mat2x2";

    success = _d->hasValue<mat2x2>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> mat2x2";

    success = _d->hasValue<mat2x2>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> mat2x2";

    success = _d->hasValue<mat2x2>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> mat2x2";

    success = _d->hasValue<mat2x2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> mat2x2";

    success = _d->hasValue<mat2x2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> mat2x2";

    success = _d->hasValue<mat2x2>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> mat2x2";

    success = _d->hasValue<mat2x2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> mat2x2";

    success = _d->hasValue<mat2x2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> mat2x2";

    using glm::mat2x3;

    success = _d->hasValue<mat2x3>("bool");
    EXPECT_EQ(false, success) << "bool -> mat2x3";

    success = _d->hasValue<mat2x3>("char");
    EXPECT_EQ(false, success) << "char -> mat2x3";

    success = _d->hasValue<mat2x3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> mat2x3";

    success = _d->hasValue<mat2x3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> mat2x3";

    success = _d->hasValue<mat2x3>("short");
    EXPECT_EQ(false, success) << "short -> mat2x3";

    success = _d->hasValue<mat2x3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> mat2x3";

    success = _d->hasValue<mat2x3>("int");
    EXPECT_EQ(false, success) << "int -> mat2x3";

    success = _d->hasValue<mat2x3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> mat2x3";

    success = _d->hasValue<mat2x3>("long long");
    EXPECT_EQ(false, success) << "long long -> mat2x3";

    success = _d->hasValue<mat2x3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> mat2x3";

    success = _d->hasValue<mat2x3>("float");
    EXPECT_EQ(false, success) << "float -> mat2x3";

    success = _d->hasValue<mat2x3>("double");
    EXPECT_EQ(false, success) << "double -> mat2x3";

    success = _d->hasValue<mat2x3>("long double");
    EXPECT_EQ(false, success) << "long double -> mat2x3";

    success = _d->hasValue<mat2x3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> mat2x3";

    success = _d->hasValue<mat2x3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> mat2x3";

    success = _d->hasValue<mat2x3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> mat2x3";

    success = _d->hasValue<mat2x3>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> mat2x3";

    success = _d->hasValue<mat2x3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> mat2x3";

    success = _d->hasValue<mat2x3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> mat2x3";

    success = _d->hasValue<mat2x3>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> mat2x3";

    success = _d->hasValue<mat2x3>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> mat2x3";

    success = _d->hasValue<mat2x3>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> mat2x3";

    success = _d->hasValue<mat2x3>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> mat2x3";

    success = _d->hasValue<mat2x3>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> mat2x3";

    success = _d->hasValue<mat2x3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> mat2x3";

    success = _d->hasValue<mat2x3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> mat2x3";

    success = _d->hasValue<mat2x3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> mat2x3";

    success = _d->hasValue<mat2x3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> mat2x3";

    success = _d->hasValue<mat2x3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> mat2x3";

    success = _d->hasValue<mat2x3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> mat2x3";

    success = _d->hasValue<mat2x3>("mat2x3");
    EXPECT_EQ(true, success) << "mat2x3 -> mat2x3";

    success = _d->hasValue<mat2x3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> mat2x3";

    success = _d->hasValue<mat2x3>("mat3x2");
    EXPECT_EQ(true, success) << "mat3x2 -> mat2x3";

    success = _d->hasValue<mat2x3>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> mat2x3";

    success = _d->hasValue<mat2x3>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> mat2x3";

    success = _d->hasValue<mat2x3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> mat2x3";

    success = _d->hasValue<mat2x3>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> mat2x3";

    success = _d->hasValue<mat2x3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> mat2x3";

    success = _d->hasValue<mat2x3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> mat2x3";

    success = _d->hasValue<mat2x3>("dmat2x3");
    EXPECT_EQ(true, success) << "dmat2x3 -> mat2x3";

    success = _d->hasValue<mat2x3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> mat2x3";

    success = _d->hasValue<mat2x3>("dmat3x2");
    EXPECT_EQ(true, success) << "dmat3x2 -> mat2x3";

    success = _d->hasValue<mat2x3>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> mat2x3";

    success = _d->hasValue<mat2x3>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> mat2x3";

    success = _d->hasValue<mat2x3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> mat2x3";

    success = _d->hasValue<mat2x3>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> mat2x3";

    success = _d->hasValue<mat2x3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> mat2x3";

    using glm::mat2x4;

    success = _d->hasValue<mat2x4>("bool");
    EXPECT_EQ(false, success) << "bool -> mat2x4";

    success = _d->hasValue<mat2x4>("char");
    EXPECT_EQ(false, success) << "char -> mat2x4";

    success = _d->hasValue<mat2x4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> mat2x4";

    success = _d->hasValue<mat2x4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> mat2x4";

    success = _d->hasValue<mat2x4>("short");
    EXPECT_EQ(false, success) << "short -> mat2x4";

    success = _d->hasValue<mat2x4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> mat2x4";

    success = _d->hasValue<mat2x4>("int");
    EXPECT_EQ(false, success) << "int -> mat2x4";

    success = _d->hasValue<mat2x4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> mat2x4";

    success = _d->hasValue<mat2x4>("long long");
    EXPECT_EQ(false, success) << "long long -> mat2x4";

    success = _d->hasValue<mat2x4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> mat2x4";

    success = _d->hasValue<mat2x4>("float");
    EXPECT_EQ(false, success) << "float -> mat2x4";

    success = _d->hasValue<mat2x4>("double");
    EXPECT_EQ(false, success) << "double -> mat2x4";

    success = _d->hasValue<mat2x4>("long double");
    EXPECT_EQ(false, success) << "long double -> mat2x4";

    success = _d->hasValue<mat2x4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> mat2x4";

    success = _d->hasValue<mat2x4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> mat2x4";

    success = _d->hasValue<mat2x4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> mat2x4";

    success = _d->hasValue<mat2x4>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> mat2x4";

    success = _d->hasValue<mat2x4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> mat2x4";

    success = _d->hasValue<mat2x4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> mat2x4";

    success = _d->hasValue<mat2x4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> mat2x4";

    success = _d->hasValue<mat2x4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> mat2x4";

    success = _d->hasValue<mat2x4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> mat2x4";

    success = _d->hasValue<mat2x4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> mat2x4";

    success = _d->hasValue<mat2x4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> mat2x4";

    success = _d->hasValue<mat2x4>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> mat2x4";

    success = _d->hasValue<mat2x4>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> mat2x4";

    success = _d->hasValue<mat2x4>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> mat2x4";

    success = _d->hasValue<mat2x4>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> mat2x4";

    success = _d->hasValue<mat2x4>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> mat2x4";

    success = _d->hasValue<mat2x4>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> mat2x4";

    success = _d->hasValue<mat2x4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> mat2x4";

    success = _d->hasValue<mat2x4>("mat2x4");
    EXPECT_EQ(true, success) << "mat2x4 -> mat2x4";

    success = _d->hasValue<mat2x4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> mat2x4";

    success = _d->hasValue<mat2x4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> mat2x4";

    success = _d->hasValue<mat2x4>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> mat2x4";

    success = _d->hasValue<mat2x4>("mat4x2");
    EXPECT_EQ(true, success) << "mat4x2 -> mat2x4";

    success = _d->hasValue<mat2x4>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> mat2x4";

    success = _d->hasValue<mat2x4>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> mat2x4";

    success = _d->hasValue<mat2x4>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> mat2x4";

    success = _d->hasValue<mat2x4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> mat2x4";

    success = _d->hasValue<mat2x4>("dmat2x4");
    EXPECT_EQ(true, success) << "dmat2x4 -> mat2x4";

    success = _d->hasValue<mat2x4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> mat2x4";

    success = _d->hasValue<mat2x4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> mat2x4";

    success = _d->hasValue<mat2x4>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> mat2x4";

    success = _d->hasValue<mat2x4>("dmat4x2");
    EXPECT_EQ(true, success) << "dmat4x2 -> mat2x4";

    success = _d->hasValue<mat2x4>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> mat2x4";

    success = _d->hasValue<mat2x4>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> mat2x4";

    using glm::mat3x2;

    success = _d->hasValue<mat3x2>("bool");
    EXPECT_EQ(false, success) << "bool -> mat3x2";

    success = _d->hasValue<mat3x2>("char");
    EXPECT_EQ(false, success) << "char -> mat3x2";

    success = _d->hasValue<mat3x2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> mat3x2";

    success = _d->hasValue<mat3x2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> mat3x2";

    success = _d->hasValue<mat3x2>("short");
    EXPECT_EQ(false, success) << "short -> mat3x2";

    success = _d->hasValue<mat3x2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> mat3x2";

    success = _d->hasValue<mat3x2>("int");
    EXPECT_EQ(false, success) << "int -> mat3x2";

    success = _d->hasValue<mat3x2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> mat3x2";

    success = _d->hasValue<mat3x2>("long long");
    EXPECT_EQ(false, success) << "long long -> mat3x2";

    success = _d->hasValue<mat3x2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> mat3x2";

    success = _d->hasValue<mat3x2>("float");
    EXPECT_EQ(false, success) << "float -> mat3x2";

    success = _d->hasValue<mat3x2>("double");
    EXPECT_EQ(false, success) << "double -> mat3x2";

    success = _d->hasValue<mat3x2>("long double");
    EXPECT_EQ(false, success) << "long double -> mat3x2";

    success = _d->hasValue<mat3x2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> mat3x2";

    success = _d->hasValue<mat3x2>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> mat3x2";

    success = _d->hasValue<mat3x2>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> mat3x2";

    success = _d->hasValue<mat3x2>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> mat3x2";

    success = _d->hasValue<mat3x2>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> mat3x2";

    success = _d->hasValue<mat3x2>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> mat3x2";

    success = _d->hasValue<mat3x2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> mat3x2";

    success = _d->hasValue<mat3x2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> mat3x2";

    success = _d->hasValue<mat3x2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> mat3x2";

    success = _d->hasValue<mat3x2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> mat3x2";

    success = _d->hasValue<mat3x2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> mat3x2";

    success = _d->hasValue<mat3x2>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> mat3x2";

    success = _d->hasValue<mat3x2>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> mat3x2";

    success = _d->hasValue<mat3x2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> mat3x2";

    success = _d->hasValue<mat3x2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> mat3x2";

    success = _d->hasValue<mat3x2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> mat3x2";

    success = _d->hasValue<mat3x2>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> mat3x2";

    success = _d->hasValue<mat3x2>("mat2x3");
    EXPECT_EQ(true, success) << "mat2x3 -> mat3x2";

    success = _d->hasValue<mat3x2>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> mat3x2";

    success = _d->hasValue<mat3x2>("mat3x2");
    EXPECT_EQ(true, success) << "mat3x2 -> mat3x2";

    success = _d->hasValue<mat3x2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> mat3x2";

    success = _d->hasValue<mat3x2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> mat3x2";

    success = _d->hasValue<mat3x2>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> mat3x2";

    success = _d->hasValue<mat3x2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> mat3x2";

    success = _d->hasValue<mat3x2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> mat3x2";

    success = _d->hasValue<mat3x2>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> mat3x2";

    success = _d->hasValue<mat3x2>("dmat2x3");
    EXPECT_EQ(true, success) << "dmat2x3 -> mat3x2";

    success = _d->hasValue<mat3x2>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> mat3x2";

    success = _d->hasValue<mat3x2>("dmat3x2");
    EXPECT_EQ(true, success) << "dmat3x2 -> mat3x2";

    success = _d->hasValue<mat3x2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> mat3x2";

    success = _d->hasValue<mat3x2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> mat3x2";

    success = _d->hasValue<mat3x2>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> mat3x2";

    success = _d->hasValue<mat3x2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> mat3x2";

    success = _d->hasValue<mat3x2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> mat3x2";

    using glm::mat3x3;

    success = _d->hasValue<mat3x3>("bool");
    EXPECT_EQ(false, success) << "bool -> mat3x3";

    success = _d->hasValue<mat3x3>("char");
    EXPECT_EQ(false, success) << "char -> mat3x3";

    success = _d->hasValue<mat3x3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> mat3x3";

    success = _d->hasValue<mat3x3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> mat3x3";

    success = _d->hasValue<mat3x3>("short");
    EXPECT_EQ(false, success) << "short -> mat3x3";

    success = _d->hasValue<mat3x3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> mat3x3";

    success = _d->hasValue<mat3x3>("int");
    EXPECT_EQ(false, success) << "int -> mat3x3";

    success = _d->hasValue<mat3x3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> mat3x3";

    success = _d->hasValue<mat3x3>("long long");
    EXPECT_EQ(false, success) << "long long -> mat3x3";

    success = _d->hasValue<mat3x3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> mat3x3";

    success = _d->hasValue<mat3x3>("float");
    EXPECT_EQ(false, success) << "float -> mat3x3";

    success = _d->hasValue<mat3x3>("double");
    EXPECT_EQ(false, success) << "double -> mat3x3";

    success = _d->hasValue<mat3x3>("long double");
    EXPECT_EQ(false, success) << "long double -> mat3x3";

    success = _d->hasValue<mat3x3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> mat3x3";

    success = _d->hasValue<mat3x3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> mat3x3";

    success = _d->hasValue<mat3x3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> mat3x3";

    success = _d->hasValue<mat3x3>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> mat3x3";

    success = _d->hasValue<mat3x3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> mat3x3";

    success = _d->hasValue<mat3x3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> mat3x3";

    success = _d->hasValue<mat3x3>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> mat3x3";

    success = _d->hasValue<mat3x3>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> mat3x3";

    success = _d->hasValue<mat3x3>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> mat3x3";

    success = _d->hasValue<mat3x3>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> mat3x3";

    success = _d->hasValue<mat3x3>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> mat3x3";

    success = _d->hasValue<mat3x3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> mat3x3";

    success = _d->hasValue<mat3x3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> mat3x3";

    success = _d->hasValue<mat3x3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> mat3x3";

    success = _d->hasValue<mat3x3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> mat3x3";

    success = _d->hasValue<mat3x3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> mat3x3";

    success = _d->hasValue<mat3x3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> mat3x3";

    success = _d->hasValue<mat3x3>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> mat3x3";

    success = _d->hasValue<mat3x3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> mat3x3";

    success = _d->hasValue<mat3x3>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> mat3x3";

    success = _d->hasValue<mat3x3>("mat3x3");
    EXPECT_EQ(true, success) << "mat3x3 -> mat3x3";

    success = _d->hasValue<mat3x3>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> mat3x3";

    success = _d->hasValue<mat3x3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> mat3x3";

    success = _d->hasValue<mat3x3>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> mat3x3";

    success = _d->hasValue<mat3x3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> mat3x3";

    success = _d->hasValue<mat3x3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> mat3x3";

    success = _d->hasValue<mat3x3>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> mat3x3";

    success = _d->hasValue<mat3x3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> mat3x3";

    success = _d->hasValue<mat3x3>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> mat3x3";

    success = _d->hasValue<mat3x3>("dmat3x3");
    EXPECT_EQ(true, success) << "dmat3x3 -> mat3x3";

    success = _d->hasValue<mat3x3>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> mat3x3";

    success = _d->hasValue<mat3x3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> mat3x3";

    success = _d->hasValue<mat3x3>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> mat3x3";

    success = _d->hasValue<mat3x3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> mat3x3";

    using glm::mat3x4;

    success = _d->hasValue<mat3x4>("bool");
    EXPECT_EQ(false, success) << "bool -> mat3x4";

    success = _d->hasValue<mat3x4>("char");
    EXPECT_EQ(false, success) << "char -> mat3x4";

    success = _d->hasValue<mat3x4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> mat3x4";

    success = _d->hasValue<mat3x4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> mat3x4";

    success = _d->hasValue<mat3x4>("short");
    EXPECT_EQ(false, success) << "short -> mat3x4";

    success = _d->hasValue<mat3x4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> mat3x4";

    success = _d->hasValue<mat3x4>("int");
    EXPECT_EQ(false, success) << "int -> mat3x4";

    success = _d->hasValue<mat3x4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> mat3x4";

    success = _d->hasValue<mat3x4>("long long");
    EXPECT_EQ(false, success) << "long long -> mat3x4";

    success = _d->hasValue<mat3x4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> mat3x4";

    success = _d->hasValue<mat3x4>("float");
    EXPECT_EQ(false, success) << "float -> mat3x4";

    success = _d->hasValue<mat3x4>("double");
    EXPECT_EQ(false, success) << "double -> mat3x4";

    success = _d->hasValue<mat3x4>("long double");
    EXPECT_EQ(false, success) << "long double -> mat3x4";

    success = _d->hasValue<mat3x4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> mat3x4";

    success = _d->hasValue<mat3x4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> mat3x4";

    success = _d->hasValue<mat3x4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> mat3x4";

    success = _d->hasValue<mat3x4>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> mat3x4";

    success = _d->hasValue<mat3x4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> mat3x4";

    success = _d->hasValue<mat3x4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> mat3x4";

    success = _d->hasValue<mat3x4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> mat3x4";

    success = _d->hasValue<mat3x4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> mat3x4";

    success = _d->hasValue<mat3x4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> mat3x4";

    success = _d->hasValue<mat3x4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> mat3x4";

    success = _d->hasValue<mat3x4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> mat3x4";

    success = _d->hasValue<mat3x4>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> mat3x4";

    success = _d->hasValue<mat3x4>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> mat3x4";

    success = _d->hasValue<mat3x4>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> mat3x4";

    success = _d->hasValue<mat3x4>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> mat3x4";

    success = _d->hasValue<mat3x4>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> mat3x4";

    success = _d->hasValue<mat3x4>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> mat3x4";

    success = _d->hasValue<mat3x4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> mat3x4";

    success = _d->hasValue<mat3x4>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> mat3x4";

    success = _d->hasValue<mat3x4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> mat3x4";

    success = _d->hasValue<mat3x4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> mat3x4";

    success = _d->hasValue<mat3x4>("mat3x4");
    EXPECT_EQ(true, success) << "mat3x4 -> mat3x4";

    success = _d->hasValue<mat3x4>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> mat3x4";

    success = _d->hasValue<mat3x4>("mat4x3");
    EXPECT_EQ(true, success) << "mat4x3 -> mat3x4";

    success = _d->hasValue<mat3x4>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> mat3x4";

    success = _d->hasValue<mat3x4>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> mat3x4";

    success = _d->hasValue<mat3x4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> mat3x4";

    success = _d->hasValue<mat3x4>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> mat3x4";

    success = _d->hasValue<mat3x4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> mat3x4";

    success = _d->hasValue<mat3x4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> mat3x4";

    success = _d->hasValue<mat3x4>("dmat3x4");
    EXPECT_EQ(true, success) << "dmat3x4 -> mat3x4";

    success = _d->hasValue<mat3x4>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> mat3x4";

    success = _d->hasValue<mat3x4>("dmat4x3");
    EXPECT_EQ(true, success) << "dmat4x3 -> mat3x4";

    success = _d->hasValue<mat3x4>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> mat3x4";

    using glm::mat4x2;

    success = _d->hasValue<mat4x2>("bool");
    EXPECT_EQ(false, success) << "bool -> mat4x2";

    success = _d->hasValue<mat4x2>("char");
    EXPECT_EQ(false, success) << "char -> mat4x2";

    success = _d->hasValue<mat4x2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> mat4x2";

    success = _d->hasValue<mat4x2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> mat4x2";

    success = _d->hasValue<mat4x2>("short");
    EXPECT_EQ(false, success) << "short -> mat4x2";

    success = _d->hasValue<mat4x2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> mat4x2";

    success = _d->hasValue<mat4x2>("int");
    EXPECT_EQ(false, success) << "int -> mat4x2";

    success = _d->hasValue<mat4x2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> mat4x2";

    success = _d->hasValue<mat4x2>("long long");
    EXPECT_EQ(false, success) << "long long -> mat4x2";

    success = _d->hasValue<mat4x2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> mat4x2";

    success = _d->hasValue<mat4x2>("float");
    EXPECT_EQ(false, success) << "float -> mat4x2";

    success = _d->hasValue<mat4x2>("double");
    EXPECT_EQ(false, success) << "double -> mat4x2";

    success = _d->hasValue<mat4x2>("long double");
    EXPECT_EQ(false, success) << "long double -> mat4x2";

    success = _d->hasValue<mat4x2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> mat4x2";

    success = _d->hasValue<mat4x2>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> mat4x2";

    success = _d->hasValue<mat4x2>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> mat4x2";

    success = _d->hasValue<mat4x2>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> mat4x2";

    success = _d->hasValue<mat4x2>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> mat4x2";

    success = _d->hasValue<mat4x2>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> mat4x2";

    success = _d->hasValue<mat4x2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> mat4x2";

    success = _d->hasValue<mat4x2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> mat4x2";

    success = _d->hasValue<mat4x2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> mat4x2";

    success = _d->hasValue<mat4x2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> mat4x2";

    success = _d->hasValue<mat4x2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> mat4x2";

    success = _d->hasValue<mat4x2>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> mat4x2";

    success = _d->hasValue<mat4x2>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> mat4x2";

    success = _d->hasValue<mat4x2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> mat4x2";

    success = _d->hasValue<mat4x2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> mat4x2";

    success = _d->hasValue<mat4x2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> mat4x2";

    success = _d->hasValue<mat4x2>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> mat4x2";

    success = _d->hasValue<mat4x2>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> mat4x2";

    success = _d->hasValue<mat4x2>("mat2x4");
    EXPECT_EQ(true, success) << "mat2x4 -> mat4x2";

    success = _d->hasValue<mat4x2>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> mat4x2";

    success = _d->hasValue<mat4x2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> mat4x2";

    success = _d->hasValue<mat4x2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> mat4x2";

    success = _d->hasValue<mat4x2>("mat4x2");
    EXPECT_EQ(true, success) << "mat4x2 -> mat4x2";

    success = _d->hasValue<mat4x2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> mat4x2";

    success = _d->hasValue<mat4x2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> mat4x2";

    success = _d->hasValue<mat4x2>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> mat4x2";

    success = _d->hasValue<mat4x2>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> mat4x2";

    success = _d->hasValue<mat4x2>("dmat2x4");
    EXPECT_EQ(true, success) << "dmat2x4 -> mat4x2";

    success = _d->hasValue<mat4x2>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> mat4x2";

    success = _d->hasValue<mat4x2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> mat4x2";

    success = _d->hasValue<mat4x2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> mat4x2";

    success = _d->hasValue<mat4x2>("dmat4x2");
    EXPECT_EQ(true, success) << "dmat4x2 -> mat4x2";

    success = _d->hasValue<mat4x2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> mat4x2";

    success = _d->hasValue<mat4x2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> mat4x2";

    using glm::mat4x3;

    success = _d->hasValue<mat4x3>("bool");
    EXPECT_EQ(false, success) << "bool -> mat4x3";

    success = _d->hasValue<mat4x3>("char");
    EXPECT_EQ(false, success) << "char -> mat4x3";

    success = _d->hasValue<mat4x3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> mat4x3";

    success = _d->hasValue<mat4x3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> mat4x3";

    success = _d->hasValue<mat4x3>("short");
    EXPECT_EQ(false, success) << "short -> mat4x3";

    success = _d->hasValue<mat4x3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> mat4x3";

    success = _d->hasValue<mat4x3>("int");
    EXPECT_EQ(false, success) << "int -> mat4x3";

    success = _d->hasValue<mat4x3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> mat4x3";

    success = _d->hasValue<mat4x3>("long long");
    EXPECT_EQ(false, success) << "long long -> mat4x3";

    success = _d->hasValue<mat4x3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> mat4x3";

    success = _d->hasValue<mat4x3>("float");
    EXPECT_EQ(false, success) << "float -> mat4x3";

    success = _d->hasValue<mat4x3>("double");
    EXPECT_EQ(false, success) << "double -> mat4x3";

    success = _d->hasValue<mat4x3>("long double");
    EXPECT_EQ(false, success) << "long double -> mat4x3";

    success = _d->hasValue<mat4x3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> mat4x3";

    success = _d->hasValue<mat4x3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> mat4x3";

    success = _d->hasValue<mat4x3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> mat4x3";

    success = _d->hasValue<mat4x3>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> mat4x3";

    success = _d->hasValue<mat4x3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> mat4x3";

    success = _d->hasValue<mat4x3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> mat4x3";

    success = _d->hasValue<mat4x3>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> mat4x3";

    success = _d->hasValue<mat4x3>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> mat4x3";

    success = _d->hasValue<mat4x3>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> mat4x3";

    success = _d->hasValue<mat4x3>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> mat4x3";

    success = _d->hasValue<mat4x3>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> mat4x3";

    success = _d->hasValue<mat4x3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> mat4x3";

    success = _d->hasValue<mat4x3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> mat4x3";

    success = _d->hasValue<mat4x3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> mat4x3";

    success = _d->hasValue<mat4x3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> mat4x3";

    success = _d->hasValue<mat4x3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> mat4x3";

    success = _d->hasValue<mat4x3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> mat4x3";

    success = _d->hasValue<mat4x3>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> mat4x3";

    success = _d->hasValue<mat4x3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> mat4x3";

    success = _d->hasValue<mat4x3>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> mat4x3";

    success = _d->hasValue<mat4x3>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> mat4x3";

    success = _d->hasValue<mat4x3>("mat3x4");
    EXPECT_EQ(true, success) << "mat3x4 -> mat4x3";

    success = _d->hasValue<mat4x3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> mat4x3";

    success = _d->hasValue<mat4x3>("mat4x3");
    EXPECT_EQ(true, success) << "mat4x3 -> mat4x3";

    success = _d->hasValue<mat4x3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> mat4x3";

    success = _d->hasValue<mat4x3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> mat4x3";

    success = _d->hasValue<mat4x3>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> mat4x3";

    success = _d->hasValue<mat4x3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> mat4x3";

    success = _d->hasValue<mat4x3>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> mat4x3";

    success = _d->hasValue<mat4x3>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> mat4x3";

    success = _d->hasValue<mat4x3>("dmat3x4");
    EXPECT_EQ(true, success) << "dmat3x4 -> mat4x3";

    success = _d->hasValue<mat4x3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> mat4x3";

    success = _d->hasValue<mat4x3>("dmat4x3");
    EXPECT_EQ(true, success) << "dmat4x3 -> mat4x3";

    success = _d->hasValue<mat4x3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> mat4x3";

    using glm::mat4x4;

    success = _d->hasValue<mat4x4>("bool");
    EXPECT_EQ(false, success) << "bool -> mat4x4";

    success = _d->hasValue<mat4x4>("char");
    EXPECT_EQ(false, success) << "char -> mat4x4";

    success = _d->hasValue<mat4x4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> mat4x4";

    success = _d->hasValue<mat4x4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> mat4x4";

    success = _d->hasValue<mat4x4>("short");
    EXPECT_EQ(false, success) << "short -> mat4x4";

    success = _d->hasValue<mat4x4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> mat4x4";

    success = _d->hasValue<mat4x4>("int");
    EXPECT_EQ(false, success) << "int -> mat4x4";

    success = _d->hasValue<mat4x4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> mat4x4";

    success = _d->hasValue<mat4x4>("long long");
    EXPECT_EQ(false, success) << "long long -> mat4x4";

    success = _d->hasValue<mat4x4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> mat4x4";

    success = _d->hasValue<mat4x4>("float");
    EXPECT_EQ(false, success) << "float -> mat4x4";

    success = _d->hasValue<mat4x4>("double");
    EXPECT_EQ(false, success) << "double -> mat4x4";

    success = _d->hasValue<mat4x4>("long double");
    EXPECT_EQ(false, success) << "long double -> mat4x4";

    success = _d->hasValue<mat4x4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> mat4x4";

    success = _d->hasValue<mat4x4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> mat4x4";

    success = _d->hasValue<mat4x4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> mat4x4";

    success = _d->hasValue<mat4x4>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> mat4x4";

    success = _d->hasValue<mat4x4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> mat4x4";

    success = _d->hasValue<mat4x4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> mat4x4";

    success = _d->hasValue<mat4x4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> mat4x4";

    success = _d->hasValue<mat4x4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> mat4x4";

    success = _d->hasValue<mat4x4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> mat4x4";

    success = _d->hasValue<mat4x4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> mat4x4";

    success = _d->hasValue<mat4x4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> mat4x4";

    success = _d->hasValue<mat4x4>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> mat4x4";

    success = _d->hasValue<mat4x4>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> mat4x4";

    success = _d->hasValue<mat4x4>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> mat4x4";

    success = _d->hasValue<mat4x4>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> mat4x4";

    success = _d->hasValue<mat4x4>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> mat4x4";

    success = _d->hasValue<mat4x4>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> mat4x4";

    success = _d->hasValue<mat4x4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> mat4x4";

    success = _d->hasValue<mat4x4>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> mat4x4";

    success = _d->hasValue<mat4x4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> mat4x4";

    success = _d->hasValue<mat4x4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> mat4x4";

    success = _d->hasValue<mat4x4>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> mat4x4";

    success = _d->hasValue<mat4x4>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> mat4x4";

    success = _d->hasValue<mat4x4>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> mat4x4";

    success = _d->hasValue<mat4x4>("mat4x4");
    EXPECT_EQ(true, success) << "mat4x4 -> mat4x4";

    success = _d->hasValue<mat4x4>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> mat4x4";

    success = _d->hasValue<mat4x4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> mat4x4";

    success = _d->hasValue<mat4x4>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> mat4x4";

    success = _d->hasValue<mat4x4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> mat4x4";

    success = _d->hasValue<mat4x4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> mat4x4";

    success = _d->hasValue<mat4x4>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> mat4x4";

    success = _d->hasValue<mat4x4>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> mat4x4";

    success = _d->hasValue<mat4x4>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> mat4x4";

    success = _d->hasValue<mat4x4>("dmat4x4");
    EXPECT_EQ(true, success) << "dmat4x4 -> mat4x4";

    using glm::dmat2x2;

    success = _d->hasValue<dmat2x2>("bool");
    EXPECT_EQ(false, success) << "bool -> dmat2x2";

    success = _d->hasValue<dmat2x2>("char");
    EXPECT_EQ(false, success) << "char -> dmat2x2";

    success = _d->hasValue<dmat2x2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dmat2x2";

    success = _d->hasValue<dmat2x2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dmat2x2";

    success = _d->hasValue<dmat2x2>("short");
    EXPECT_EQ(false, success) << "short -> dmat2x2";

    success = _d->hasValue<dmat2x2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dmat2x2";

    success = _d->hasValue<dmat2x2>("int");
    EXPECT_EQ(false, success) << "int -> dmat2x2";

    success = _d->hasValue<dmat2x2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dmat2x2";

    success = _d->hasValue<dmat2x2>("long long");
    EXPECT_EQ(false, success) << "long long -> dmat2x2";

    success = _d->hasValue<dmat2x2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dmat2x2";

    success = _d->hasValue<dmat2x2>("float");
    EXPECT_EQ(false, success) << "float -> dmat2x2";

    success = _d->hasValue<dmat2x2>("double");
    EXPECT_EQ(false, success) << "double -> dmat2x2";

    success = _d->hasValue<dmat2x2>("long double");
    EXPECT_EQ(false, success) << "long double -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dmat2x2";

    success = _d->hasValue<dmat2x2>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("vec4");
    EXPECT_EQ(true, success) << "vec4 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dvec4");
    EXPECT_EQ(true, success) << "dvec4 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("mat2x2");
    EXPECT_EQ(true, success) << "mat2x2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dmat2x2");
    EXPECT_EQ(true, success) << "dmat2x2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> dmat2x2";

    success = _d->hasValue<dmat2x2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dmat2x2";

    using glm::dmat2x3;

    success = _d->hasValue<dmat2x3>("bool");
    EXPECT_EQ(false, success) << "bool -> dmat2x3";

    success = _d->hasValue<dmat2x3>("char");
    EXPECT_EQ(false, success) << "char -> dmat2x3";

    success = _d->hasValue<dmat2x3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dmat2x3";

    success = _d->hasValue<dmat2x3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dmat2x3";

    success = _d->hasValue<dmat2x3>("short");
    EXPECT_EQ(false, success) << "short -> dmat2x3";

    success = _d->hasValue<dmat2x3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dmat2x3";

    success = _d->hasValue<dmat2x3>("int");
    EXPECT_EQ(false, success) << "int -> dmat2x3";

    success = _d->hasValue<dmat2x3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dmat2x3";

    success = _d->hasValue<dmat2x3>("long long");
    EXPECT_EQ(false, success) << "long long -> dmat2x3";

    success = _d->hasValue<dmat2x3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dmat2x3";

    success = _d->hasValue<dmat2x3>("float");
    EXPECT_EQ(false, success) << "float -> dmat2x3";

    success = _d->hasValue<dmat2x3>("double");
    EXPECT_EQ(false, success) << "double -> dmat2x3";

    success = _d->hasValue<dmat2x3>("long double");
    EXPECT_EQ(false, success) << "long double -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dmat2x3";

    success = _d->hasValue<dmat2x3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("mat2x3");
    EXPECT_EQ(true, success) << "mat2x3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("mat3x2");
    EXPECT_EQ(true, success) << "mat3x2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dmat2x3");
    EXPECT_EQ(true, success) << "dmat2x3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dmat3x2");
    EXPECT_EQ(true, success) << "dmat3x2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> dmat2x3";

    success = _d->hasValue<dmat2x3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dmat2x3";

    using glm::dmat2x4;

    success = _d->hasValue<dmat2x4>("bool");
    EXPECT_EQ(false, success) << "bool -> dmat2x4";

    success = _d->hasValue<dmat2x4>("char");
    EXPECT_EQ(false, success) << "char -> dmat2x4";

    success = _d->hasValue<dmat2x4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dmat2x4";

    success = _d->hasValue<dmat2x4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dmat2x4";

    success = _d->hasValue<dmat2x4>("short");
    EXPECT_EQ(false, success) << "short -> dmat2x4";

    success = _d->hasValue<dmat2x4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dmat2x4";

    success = _d->hasValue<dmat2x4>("int");
    EXPECT_EQ(false, success) << "int -> dmat2x4";

    success = _d->hasValue<dmat2x4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dmat2x4";

    success = _d->hasValue<dmat2x4>("long long");
    EXPECT_EQ(false, success) << "long long -> dmat2x4";

    success = _d->hasValue<dmat2x4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dmat2x4";

    success = _d->hasValue<dmat2x4>("float");
    EXPECT_EQ(false, success) << "float -> dmat2x4";

    success = _d->hasValue<dmat2x4>("double");
    EXPECT_EQ(false, success) << "double -> dmat2x4";

    success = _d->hasValue<dmat2x4>("long double");
    EXPECT_EQ(false, success) << "long double -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dmat2x4";

    success = _d->hasValue<dmat2x4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("mat2x4");
    EXPECT_EQ(true, success) << "mat2x4 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("mat4x2");
    EXPECT_EQ(true, success) << "mat4x2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dmat2x4");
    EXPECT_EQ(true, success) << "dmat2x4 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dmat4x2");
    EXPECT_EQ(true, success) << "dmat4x2 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> dmat2x4";

    success = _d->hasValue<dmat2x4>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dmat2x4";

    using glm::dmat3x2;

    success = _d->hasValue<dmat3x2>("bool");
    EXPECT_EQ(false, success) << "bool -> dmat3x2";

    success = _d->hasValue<dmat3x2>("char");
    EXPECT_EQ(false, success) << "char -> dmat3x2";

    success = _d->hasValue<dmat3x2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dmat3x2";

    success = _d->hasValue<dmat3x2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dmat3x2";

    success = _d->hasValue<dmat3x2>("short");
    EXPECT_EQ(false, success) << "short -> dmat3x2";

    success = _d->hasValue<dmat3x2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dmat3x2";

    success = _d->hasValue<dmat3x2>("int");
    EXPECT_EQ(false, success) << "int -> dmat3x2";

    success = _d->hasValue<dmat3x2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dmat3x2";

    success = _d->hasValue<dmat3x2>("long long");
    EXPECT_EQ(false, success) << "long long -> dmat3x2";

    success = _d->hasValue<dmat3x2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dmat3x2";

    success = _d->hasValue<dmat3x2>("float");
    EXPECT_EQ(false, success) << "float -> dmat3x2";

    success = _d->hasValue<dmat3x2>("double");
    EXPECT_EQ(false, success) << "double -> dmat3x2";

    success = _d->hasValue<dmat3x2>("long double");
    EXPECT_EQ(false, success) << "long double -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dmat3x2";

    success = _d->hasValue<dmat3x2>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("mat2x3");
    EXPECT_EQ(true, success) << "mat2x3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("mat3x2");
    EXPECT_EQ(true, success) << "mat3x2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dmat2x3");
    EXPECT_EQ(true, success) << "dmat2x3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dmat3x2");
    EXPECT_EQ(true, success) << "dmat3x2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> dmat3x2";

    success = _d->hasValue<dmat3x2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dmat3x2";

    using glm::dmat3x3;

    success = _d->hasValue<dmat3x3>("bool");
    EXPECT_EQ(false, success) << "bool -> dmat3x3";

    success = _d->hasValue<dmat3x3>("char");
    EXPECT_EQ(false, success) << "char -> dmat3x3";

    success = _d->hasValue<dmat3x3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dmat3x3";

    success = _d->hasValue<dmat3x3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dmat3x3";

    success = _d->hasValue<dmat3x3>("short");
    EXPECT_EQ(false, success) << "short -> dmat3x3";

    success = _d->hasValue<dmat3x3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dmat3x3";

    success = _d->hasValue<dmat3x3>("int");
    EXPECT_EQ(false, success) << "int -> dmat3x3";

    success = _d->hasValue<dmat3x3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dmat3x3";

    success = _d->hasValue<dmat3x3>("long long");
    EXPECT_EQ(false, success) << "long long -> dmat3x3";

    success = _d->hasValue<dmat3x3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dmat3x3";

    success = _d->hasValue<dmat3x3>("float");
    EXPECT_EQ(false, success) << "float -> dmat3x3";

    success = _d->hasValue<dmat3x3>("double");
    EXPECT_EQ(false, success) << "double -> dmat3x3";

    success = _d->hasValue<dmat3x3>("long double");
    EXPECT_EQ(false, success) << "long double -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dmat3x3";

    success = _d->hasValue<dmat3x3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("mat3x3");
    EXPECT_EQ(true, success) << "mat3x3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dmat3x3");
    EXPECT_EQ(true, success) << "dmat3x3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> dmat3x3";

    success = _d->hasValue<dmat3x3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dmat3x3";

    using glm::dmat3x4;

    success = _d->hasValue<dmat3x4>("bool");
    EXPECT_EQ(false, success) << "bool -> dmat3x4";

    success = _d->hasValue<dmat3x4>("char");
    EXPECT_EQ(false, success) << "char -> dmat3x4";

    success = _d->hasValue<dmat3x4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dmat3x4";

    success = _d->hasValue<dmat3x4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dmat3x4";

    success = _d->hasValue<dmat3x4>("short");
    EXPECT_EQ(false, success) << "short -> dmat3x4";

    success = _d->hasValue<dmat3x4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dmat3x4";

    success = _d->hasValue<dmat3x4>("int");
    EXPECT_EQ(false, success) << "int -> dmat3x4";

    success = _d->hasValue<dmat3x4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dmat3x4";

    success = _d->hasValue<dmat3x4>("long long");
    EXPECT_EQ(false, success) << "long long -> dmat3x4";

    success = _d->hasValue<dmat3x4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dmat3x4";

    success = _d->hasValue<dmat3x4>("float");
    EXPECT_EQ(false, success) << "float -> dmat3x4";

    success = _d->hasValue<dmat3x4>("double");
    EXPECT_EQ(false, success) << "double -> dmat3x4";

    success = _d->hasValue<dmat3x4>("long double");
    EXPECT_EQ(false, success) << "long double -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dmat3x4";

    success = _d->hasValue<dmat3x4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("mat3x4");
    EXPECT_EQ(true, success) << "mat3x4 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("mat4x3");
    EXPECT_EQ(true, success) << "mat4x3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dmat3x4");
    EXPECT_EQ(true, success) << "dmat3x4 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dmat4x3");
    EXPECT_EQ(true, success) << "dmat4x3 -> dmat3x4";

    success = _d->hasValue<dmat3x4>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dmat3x4";

    using glm::dmat4x2;

    success = _d->hasValue<dmat4x2>("bool");
    EXPECT_EQ(false, success) << "bool -> dmat4x2";

    success = _d->hasValue<dmat4x2>("char");
    EXPECT_EQ(false, success) << "char -> dmat4x2";

    success = _d->hasValue<dmat4x2>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dmat4x2";

    success = _d->hasValue<dmat4x2>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dmat4x2";

    success = _d->hasValue<dmat4x2>("short");
    EXPECT_EQ(false, success) << "short -> dmat4x2";

    success = _d->hasValue<dmat4x2>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dmat4x2";

    success = _d->hasValue<dmat4x2>("int");
    EXPECT_EQ(false, success) << "int -> dmat4x2";

    success = _d->hasValue<dmat4x2>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dmat4x2";

    success = _d->hasValue<dmat4x2>("long long");
    EXPECT_EQ(false, success) << "long long -> dmat4x2";

    success = _d->hasValue<dmat4x2>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dmat4x2";

    success = _d->hasValue<dmat4x2>("float");
    EXPECT_EQ(false, success) << "float -> dmat4x2";

    success = _d->hasValue<dmat4x2>("double");
    EXPECT_EQ(false, success) << "double -> dmat4x2";

    success = _d->hasValue<dmat4x2>("long double");
    EXPECT_EQ(false, success) << "long double -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dmat4x2";

    success = _d->hasValue<dmat4x2>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("mat2x4");
    EXPECT_EQ(true, success) << "mat2x4 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("mat4x2");
    EXPECT_EQ(true, success) << "mat4x2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dmat2x4");
    EXPECT_EQ(true, success) << "dmat2x4 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dmat4x2");
    EXPECT_EQ(true, success) << "dmat4x2 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> dmat4x2";

    success = _d->hasValue<dmat4x2>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dmat4x2";

    using glm::dmat4x3;

    success = _d->hasValue<dmat4x3>("bool");
    EXPECT_EQ(false, success) << "bool -> dmat4x3";

    success = _d->hasValue<dmat4x3>("char");
    EXPECT_EQ(false, success) << "char -> dmat4x3";

    success = _d->hasValue<dmat4x3>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dmat4x3";

    success = _d->hasValue<dmat4x3>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dmat4x3";

    success = _d->hasValue<dmat4x3>("short");
    EXPECT_EQ(false, success) << "short -> dmat4x3";

    success = _d->hasValue<dmat4x3>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dmat4x3";

    success = _d->hasValue<dmat4x3>("int");
    EXPECT_EQ(false, success) << "int -> dmat4x3";

    success = _d->hasValue<dmat4x3>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dmat4x3";

    success = _d->hasValue<dmat4x3>("long long");
    EXPECT_EQ(false, success) << "long long -> dmat4x3";

    success = _d->hasValue<dmat4x3>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dmat4x3";

    success = _d->hasValue<dmat4x3>("float");
    EXPECT_EQ(false, success) << "float -> dmat4x3";

    success = _d->hasValue<dmat4x3>("double");
    EXPECT_EQ(false, success) << "double -> dmat4x3";

    success = _d->hasValue<dmat4x3>("long double");
    EXPECT_EQ(false, success) << "long double -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dmat4x3";

    success = _d->hasValue<dmat4x3>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("mat3x4");
    EXPECT_EQ(true, success) << "mat3x4 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("mat4x3");
    EXPECT_EQ(true, success) << "mat4x3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("mat4x4");
    EXPECT_EQ(false, success) << "mat4x4 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dmat3x4");
    EXPECT_EQ(true, success) << "dmat3x4 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dmat4x3");
    EXPECT_EQ(true, success) << "dmat4x3 -> dmat4x3";

    success = _d->hasValue<dmat4x3>("dmat4x4");
    EXPECT_EQ(false, success) << "dmat4x4 -> dmat4x3";

    using glm::dmat4x4;

    success = _d->hasValue<dmat4x4>("bool");
    EXPECT_EQ(false, success) << "bool -> dmat4x4";

    success = _d->hasValue<dmat4x4>("char");
    EXPECT_EQ(false, success) << "char -> dmat4x4";

    success = _d->hasValue<dmat4x4>("unsigned char");
    EXPECT_EQ(false, success) << "unsigned char -> dmat4x4";

    success = _d->hasValue<dmat4x4>("wchar_t");
    EXPECT_EQ(false, success) << "wchar_t -> dmat4x4";

    success = _d->hasValue<dmat4x4>("short");
    EXPECT_EQ(false, success) << "short -> dmat4x4";

    success = _d->hasValue<dmat4x4>("unsigned short");
    EXPECT_EQ(false, success) << "unsigned short -> dmat4x4";

    success = _d->hasValue<dmat4x4>("int");
    EXPECT_EQ(false, success) << "int -> dmat4x4";

    success = _d->hasValue<dmat4x4>("unsigned int");
    EXPECT_EQ(false, success) << "unsigned int -> dmat4x4";

    success = _d->hasValue<dmat4x4>("long long");
    EXPECT_EQ(false, success) << "long long -> dmat4x4";

    success = _d->hasValue<dmat4x4>("unsigned long long");
    EXPECT_EQ(false, success) << "unsigned long long -> dmat4x4";

    success = _d->hasValue<dmat4x4>("float");
    EXPECT_EQ(false, success) << "float -> dmat4x4";

    success = _d->hasValue<dmat4x4>("double");
    EXPECT_EQ(false, success) << "double -> dmat4x4";

    success = _d->hasValue<dmat4x4>("long double");
    EXPECT_EQ(false, success) << "long double -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dictionary");
    EXPECT_EQ(false, success) << "dictionary -> dmat4x4";

    success = _d->hasValue<dmat4x4>("vec2");
    EXPECT_EQ(false, success) << "vec2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dvec2");
    EXPECT_EQ(false, success) << "dvec2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("bvec2");
    EXPECT_EQ(false, success) << "ivec2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("uvec2");
    EXPECT_EQ(false, success) << "uvec2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("bvec2");
    EXPECT_EQ(false, success) << "bvec2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("vec3");
    EXPECT_EQ(false, success) << "vec3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dvec3");
    EXPECT_EQ(false, success) << "dvec3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("ivec3");
    EXPECT_EQ(false, success) << "ivec3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("uvec3");
    EXPECT_EQ(false, success) << "uvec3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("bvec3");
    EXPECT_EQ(false, success) << "bvec3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("vec4");
    EXPECT_EQ(false, success) << "vec4 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dvec4");
    EXPECT_EQ(false, success) << "dvec4 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("ivec4");
    EXPECT_EQ(false, success) << "ivec4 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("uvec4");
    EXPECT_EQ(false, success) << "uvec4 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("bvec4");
    EXPECT_EQ(false, success) << "bvec4 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("mat2x2");
    EXPECT_EQ(false, success) << "mat2x2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("mat2x3");
    EXPECT_EQ(false, success) << "mat2x3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("mat2x4");
    EXPECT_EQ(false, success) << "mat2x4 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("mat3x2");
    EXPECT_EQ(false, success) << "mat3x2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("mat3x3");
    EXPECT_EQ(false, success) << "mat3x3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("mat3x4");
    EXPECT_EQ(false, success) << "mat3x4 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("mat4x2");
    EXPECT_EQ(false, success) << "mat4x2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("mat4x3");
    EXPECT_EQ(false, success) << "mat4x3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("mat4x4");
    EXPECT_EQ(true, success) << "mat4x4 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dmat2x2");
    EXPECT_EQ(false, success) << "dmat2x2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dmat2x3");
    EXPECT_EQ(false, success) << "dmat2x3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dmat2x4");
    EXPECT_EQ(false, success) << "dmat2x4 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dmat3x2");
    EXPECT_EQ(false, success) << "dmat3x2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dmat3x3");
    EXPECT_EQ(false, success) << "dmat3x3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dmat3x4");
    EXPECT_EQ(false, success) << "dmat3x4 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dmat4x2");
    EXPECT_EQ(false, success) << "dmat4x2 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dmat4x3");
    EXPECT_EQ(false, success) << "dmat4x3 -> dmat4x4";

    success = _d->hasValue<dmat4x4>("dmat4x4");
    EXPECT_EQ(true, success) << "dmat4x4 -> dmat4x4";
}

TEST_F(DictionaryTest, ConversionFromDictionary) {
    ghoul::Dictionary dict1ElemInteger = {{"0", 1}};
    ghoul::Dictionary dict1ElemUnsignedInteger = {{"0", 1u}};
    ghoul::Dictionary dict1ElemFloat = {{"0", 1.f}};
    ghoul::Dictionary dict2ElemInteger = {{"0", 1}, {"1", 2}};
    ghoul::Dictionary dict2ElemUnsignedInteger = {{"0", 1u}, {"1", 2u}};
    ghoul::Dictionary dict2ElemFloat = {{"0", 1.f}, {"1", 2.f}};
    ghoul::Dictionary dict3ElemInteger = {{"0", 1}, {"1", 2}, {"2", 3}};
    ghoul::Dictionary dict3ElemUnsignedInteger = {{"0", 1u}, {"1", 2u}, {"2", 3u}};
    ghoul::Dictionary dict3ElemFloat = {{"0", 1.f}, {"1", 2.f}, {"2", 3.f}};
    ghoul::Dictionary dict4ElemInteger = {{"0", 1}, {"1", 2}, {"2", 3}, {"3", 4}};
    ghoul::Dictionary dict4ElemUnsignedInteger
          = {{"0", 1u}, {"1", 2u}, {"2", 3u}, {"3", 4u}};
    ghoul::Dictionary dict4ElemFloat = {{"0", 1.f}, {"1", 2.f}, {"2", 3.f}, {"3", 4.f}};
    ghoul::Dictionary dict6ElemInteger
          = {{"0", 1}, {"1", 2}, {"2", 3}, {"3", 4}, {"4", 5}, {"5", 6}};
    ghoul::Dictionary dict6ElemUnsignedInteger
          = {{"0", 1u}, {"1", 2u}, {"2", 3u}, {"3", 4u}, {"4", 5u}, {"5", 6u}};
    ghoul::Dictionary dict6ElemFloat
          = {{"0", 1.f}, {"1", 2.f}, {"2", 3.f}, {"3", 4.f}, {"4", 5.f}, {"5", 6.f}};
    ghoul::Dictionary dict8ElemInteger
        = { { "0", 1 }, { "1", 2 }, { "2", 3 }, { "3", 4 }, { "4", 5 }, { "5", 6 }, {"6", 7}, {"7", 8} };
    ghoul::Dictionary dict8ElemUnsignedInteger = {{"0", 1u},
                                                  {"1", 2u},
                                                  {"2", 3u},
                                                  {"3", 4u},
                                                  {"4", 5u},
                                                  {"5", 6u},
                                                  {"6", 7u},
                                                  {"7", 8u}};
    ghoul::Dictionary dict8ElemFloat = {{"0", 1.f},
                                        {"1", 2.f},
                                        {"2", 3.f},
                                        {"3", 4.f},
                                        {"4", 5.f},
                                        {"5", 6.f},
                                        {"6", 7.f},
                                        {"7", 8.f}};
    ghoul::Dictionary dict9ElemInteger = {{"0", 1},
                                          {"1", 2},
                                          {"2", 3},
                                          {"3", 4},
                                          {"4", 5},
                                          {"5", 6},
                                          {"6", 7},
                                          {"7", 8},
                                          {"8", 9}};
    ghoul::Dictionary dict9ElemUnsignedInteger = {{"0", 1u},
                                                  {"1", 2u},
                                                  {"2", 3u},
                                                  {"3", 4u},
                                                  {"4", 5u},
                                                  {"5", 6u},
                                                  {"6", 7u},
                                                  {"7", 8u},
                                                  {"8", 9u}};
    ghoul::Dictionary dict9ElemFloat = {{"0", 1.f},
                                        {"1", 2.f},
                                        {"2", 3.f},
                                        {"3", 4.f},
                                        {"4", 5.f},
                                        {"5", 6.f},
                                        {"6", 7.f},
                                        {"7", 8.f},
                                        {"8", 9.f}};
    ghoul::Dictionary dict12ElemInteger = {{"0", 1},
                                           {"1", 2},
                                           {"2", 3},
                                           {"3", 4},
                                           {"4", 5},
                                           {"5", 6},
                                           {"6", 7},
                                           {"7", 8},
                                           {"8", 9},
                                           {"9", 10},
                                           {"10", 11},
                                           {"11", 12}};
    ghoul::Dictionary dict12ElemUnsignedInteger = {{"0", 1u},
                                                   {"1", 2u},
                                                   {"2", 3u},
                                                   {"3", 4u},
                                                   {"4", 5u},
                                                   {"5", 6u},
                                                   {"6", 7u},
                                                   {"7", 8u},
                                                   {"8", 9u},
                                                   {"9", 10u},
                                                   {"10", 11u},
                                                   {"11", 12u}};
    ghoul::Dictionary dict12ElemFloat = {{"0", 1.f},
                                         {"1", 2.f},
                                         {"2", 3.f},
                                         {"3", 4.f},
                                         {"4", 5.f},
                                         {"5", 6.f},
                                         {"6", 7.f},
                                         {"7", 8.f},
                                         {"8", 9.f},
                                         {"9", 10.f},
                                         {"10", 11.f},
                                         {"11", 12.f}};
    ghoul::Dictionary dict16ElemInteger = {{"0", 1},
                                           {"1", 2},
                                           {"2", 3},
                                           {"3", 4},
                                           {"4", 5},
                                           {"5", 6},
                                           {"6", 7},
                                           {"7", 8},
                                           {"8", 9},
                                           {"9", 10},
                                           {"10", 11},
                                           {"11", 12},
                                           {"12", 13},
                                           {"13", 14},
                                           {"14", 15},
                                           {"15", 16}};
    ghoul::Dictionary dict16ElemUnsignedInteger = {{"0", 1u},
                                                   {"1", 2u},
                                                   {"2", 3u},
                                                   {"3", 4u},
                                                   {"4", 5u},
                                                   {"5", 6u},
                                                   {"6", 7u},
                                                   {"7", 8u},
                                                   {"8", 9u},
                                                   {"9", 10u},
                                                   {"10", 11u},
                                                   {"11", 12u},
                                                   {"12", 13u},
                                                   {"13", 14u},
                                                   {"14", 15u},
                                                   {"15", 16u}};
    ghoul::Dictionary dict16ElemFloat = {{"0", 1.f},
                                         {"1", 2.f},
                                         {"2", 3.f},
                                         {"3", 4.f},
                                         {"4", 5.f},
                                         {"5", 6.f},
                                         {"6", 7.f},
                                         {"7", 8.f},
                                         {"8", 9.f},
                                         {"9", 10.f},
                                         {"10", 11.f},
                                         {"11", 12.f},
                                         {"12", 13.f},
                                         {"13", 14.f},
                                         {"14", 15.f},
                                         {"15", 16.f}};

    ghoul::Dictionary d = {
        { "1int", dict1ElemInteger },
        { "1uint", dict1ElemUnsignedInteger },
        { "1float", dict1ElemFloat},
        { "2int", dict2ElemInteger },
        { "2uint", dict2ElemUnsignedInteger },
        { "2float", dict2ElemFloat },
        { "3int", dict3ElemInteger },
        { "3uint", dict3ElemUnsignedInteger },
        { "3float", dict3ElemFloat },
        { "4int", dict4ElemInteger },
        { "4uint", dict4ElemUnsignedInteger },
        { "4float", dict4ElemFloat },
        { "6int", dict6ElemInteger },
        { "6uint", dict6ElemUnsignedInteger },
        { "6float", dict6ElemFloat },
        { "8int", dict8ElemInteger },
        { "8uint", dict8ElemUnsignedInteger },
        { "8float", dict8ElemFloat },
        { "9int", dict9ElemInteger },
        { "9uint", dict9ElemUnsignedInteger },
        { "9float", dict9ElemFloat },
        { "12int", dict12ElemInteger },
        { "12uint", dict12ElemUnsignedInteger },
        { "12float", dict12ElemFloat },
        { "16int", dict16ElemInteger },
        { "16uint", dict16ElemUnsignedInteger },
        { "16float", dict16ElemFloat }
    };

    bool success;
    success = d.hasValue<bool>("1int");
    ASSERT_EQ(true, success) << "hasValue<bool>(\"1int\")";
    success = d.hasValue<bool>("1uint");
    EXPECT_EQ(false, success) << "hasValue<bool>(\"1uint\")";
    success = d.hasValue<bool>("1float");
    EXPECT_EQ(false, success) << "hasValue<bool>(\"1float\")";
    success = d.hasValue<bool>("2int");
    EXPECT_EQ(false, success) << "hasValue<bool>(\"2int\")";
    {
        bool value;
        success = d.getValue("1int", value);
        ASSERT_EQ(true, success) << "success getValue<bool>";
        EXPECT_EQ(true, value) << "correct getValue<bool>";
    }

    success = d.hasValue<char>("1int");
    ASSERT_EQ(true, success) << "hasValue<char>(\"1int\")";
    success = d.hasValue<char>("1uint");
    EXPECT_EQ(false, success) << "hasValue<char>(\"1uint\")";
    success = d.hasValue<char>("1float");
    EXPECT_EQ(false, success) << "hasValue<char>(\"1float\")";
    success = d.hasValue<char>("2int");
    EXPECT_EQ(false, success) << "hasValue<char>(\"2int\")";
    {
        char value;
        success = d.getValue("1int", value);
        ASSERT_EQ(true, success) << "success getValue<char>";
        EXPECT_EQ(char(1), value) << "correct getValue<char>";
    }

    success = d.hasValue<signed char>("1int");
    ASSERT_EQ(true, success) << "hasValue<signed char>(\"1int\")";
    success = d.hasValue<signed char>("1uint");
    EXPECT_EQ(false, success) << "hasValue<signed char>(\"1uint\")";
    success = d.hasValue<signed char>("1float");
    EXPECT_EQ(false, success) << "hasValue<signed char>(\"1float\")";
    success = d.hasValue<signed char>("2int");
    EXPECT_EQ(false, success) << "hasValue<signed char>(\"2int\")";
    {
        signed char value;
        success = d.getValue("1int", value);
        ASSERT_EQ(true, success) << "success getValue<signed char>";
        EXPECT_EQ(static_cast<signed char>(1), value) << "correct getValue<signed char>";
    }
    success = d.hasValue<unsigned char>("1int");
    EXPECT_EQ(false, success) << "hasValue<unsigned char>(\"1int\")";
    success = d.hasValue<unsigned char>("1uint");
    ASSERT_EQ(true, success) << "hasValue<unsigned char>(\"1uint\")";
    success = d.hasValue<unsigned char>("1float");
    EXPECT_EQ(false, success) << "hasValue<unsigned char>(\"1float\")";
    success = d.hasValue<unsigned char>("2int");
    EXPECT_EQ(false, success) << "hasValue<unsigned char>(\"2int\")";
    {
        unsigned char value;
        success = d.getValue("1uint", value);
        ASSERT_EQ(true, success) << "success getValue<unsigned char>";
        EXPECT_EQ(static_cast<unsigned char>(1), value)
              << "correct getValue<unsigned char>";
    }

    success = d.hasValue<wchar_t>("1int");
    ASSERT_EQ(true, success) << "hasValue<wchar_t>(\"1int\")";
    success = d.hasValue<wchar_t>("1uint");
    EXPECT_EQ(false, success) << "hasValue<wchar_t>(\"1uint\")";
    success = d.hasValue<wchar_t>("1float");
    EXPECT_EQ(false, success) << "hasValue<wchar_t>(\"1float\")";
    success = d.hasValue<wchar_t>("2int");
    EXPECT_EQ(false, success) << "hasValue<wchar_t>(\"2int\")";
    {
        wchar_t value;
        success = d.getValue("1int", value);
        ASSERT_EQ(true, success) << "success getValue<wchar_t>";
        EXPECT_EQ(wchar_t(1), value)
            << "correct getValue<wchar_t>";
    }

    success = d.hasValue<short>("1int");
    ASSERT_EQ(true, success) << "hasValue<short>(\"1int\")";
    success = d.hasValue<short>("1uint");
    EXPECT_EQ(false, success) << "hasValue<short>(\"1uint\")";
    success = d.hasValue<short>("1float");
    EXPECT_EQ(false, success) << "hasValue<short>(\"1float\")";
    success = d.hasValue<short>("2int");
    EXPECT_EQ(false, success) << "hasValue<short>(\"2int\")";
    {
        short value;
        success = d.getValue("1int", value);
        ASSERT_EQ(true, success) << "success getValue<short>";
        EXPECT_EQ(short(1), value)
            << "correct getValue<short>";
    }

    success = d.hasValue<unsigned short>("1int");
    EXPECT_EQ(false, success) << "hasValue<unsigned short>(\"1int\")";
    success = d.hasValue<unsigned short>("1uint");
    ASSERT_EQ(true, success) << "hasValue<unsigned short>(\"1uint\")";
    success = d.hasValue<unsigned short>("1float");
    EXPECT_EQ(false, success) << "hasValue<unsigned short>(\"1float\")";
    success = d.hasValue<unsigned short>("2int");
    EXPECT_EQ(false, success) << "hasValue<unsigned short>(\"2int\")";
    {
        unsigned short value;
        success = d.getValue("1uint", value);
        ASSERT_EQ(true, success) << "success getValue<unsigned short>";
        EXPECT_EQ(static_cast<unsigned short>(1), value)
            << "correct getValue<unsigned short>";
    }

    success = d.hasValue<int>("1int");
    ASSERT_EQ(true, success) << "hasValue<int>(\"1int\")";
    success = d.hasValue<int>("1uint");
    EXPECT_EQ(false, success) << "hasValue<int>(\"1uint\")";
    success = d.hasValue<int>("1float");
    EXPECT_EQ(false, success) << "hasValue<int>(\"1float\")";
    success = d.hasValue<int>("2int");
    EXPECT_EQ(false, success) << "hasValue<int>(\"2int\")";
    {
        int value;
        success = d.getValue("1int", value);
        ASSERT_EQ(true, success) << "success getValue<int>";
        EXPECT_EQ(int(1), value)
            << "correct getValue<int>";
    }

    success = d.hasValue<unsigned int>("1int");
    EXPECT_EQ(false, success) << "hasValue<unsigned int>(\"1int\")";
    success = d.hasValue<unsigned int>("1uint");
    ASSERT_EQ(true, success) << "hasValue<unsigned int>(\"1uint\")";
    success = d.hasValue<unsigned int>("1float");
    EXPECT_EQ(false, success) << "hasValue<unsigned int>(\"1float\")";
    success = d.hasValue<unsigned int>("2int");
    EXPECT_EQ(false, success) << "hasValue<unsigned int>(\"2int\")";
    {
        unsigned int value;
        success = d.getValue("1uint", value);
        ASSERT_EQ(true, success) << "success getValue<unsigned int>";
        EXPECT_EQ(static_cast<unsigned int>(1), value)
            << "correct getValue<unsigned int>";
    }

    success = d.hasValue<long long>("1int");
    ASSERT_EQ(true, success) << "hasValue<long long>(\"1int\")";
    success = d.hasValue<long long>("1uint");
    EXPECT_EQ(false, success) << "hasValue<long long>(\"1uint\")";
    success = d.hasValue<long long>("1float");
    EXPECT_EQ(false, success) << "hasValue<long long>(\"1float\")";
    success = d.hasValue<long long>("2int");
    EXPECT_EQ(false, success) << "hasValue<long long>(\"2int\")";
    {
        long long value;
        success = d.getValue("1int", value);
        ASSERT_EQ(true, success) << "success getValue<long long>";
        EXPECT_EQ(static_cast<long long>(1), value)
            << "correct getValue<long long>";
    }

    success = d.hasValue<unsigned long long>("1int");
    EXPECT_EQ(false, success) << "hasValue<unsigned long long>(\"1int\")";
    success = d.hasValue<unsigned long long>("1uint");
    ASSERT_EQ(true, success) << "hasValue<unsigned long long>(\"1uint\")";
    success = d.hasValue<unsigned long long>("1float");
    EXPECT_EQ(false, success) << "hasValue<unsigned long long>(\"1float\")";
    success = d.hasValue<unsigned long long>("2int");
    EXPECT_EQ(false, success) << "hasValue<unsigned long long>(\"2int\")";
    {
        unsigned long long value;
        success = d.getValue("1uint", value);
        ASSERT_EQ(true, success) << "success getValue<unsigned long long>";
        EXPECT_EQ(static_cast<unsigned long long>(1), value)
            << "correct getValue<unsigned long long>";
    }

    success = d.hasValue<float>("1int");
    EXPECT_EQ(false, success) << "hasValue<float>(\"1int\")";
    success = d.hasValue<float>("1uint");
    EXPECT_EQ(false, success) << "hasValue<float>(\"1uint\")";
    success = d.hasValue<float>("1float");
    ASSERT_EQ(true, success) << "hasValue<float>(\"1float\")";
    success = d.hasValue<float>("2int");
    EXPECT_EQ(false, success) << "hasValue<float>(\"2int\")";
    {
        float value;
        success = d.getValue("1float", value);
        ASSERT_EQ(true, success) << "success getValue<float>";
        EXPECT_EQ(float(1), value)
            << "correct getValue<float>";
    }

    success = d.hasValue<double>("1int");
    EXPECT_EQ(false, success) << "hasValue<double>(\"1int\")";
    success = d.hasValue<double>("1uint");
    EXPECT_EQ(false, success) << "hasValue<double>(\"1uint\")";
    success = d.hasValue<double>("1float");
    ASSERT_EQ(true, success) << "hasValue<double>(\"1float\")";
    success = d.hasValue<double>("2int");
    EXPECT_EQ(false, success) << "hasValue<double>(\"2int\")";
    {
        double value;
        success = d.getValue("1float", value);
        ASSERT_EQ(true, success) << "success getValue<double>";
        EXPECT_EQ(double(1), value)
            << "correct getValue<double>";
    }

    using glm::vec2;

    success = d.hasValue<vec2>("1float");
    EXPECT_EQ(false, success) << "hasValue<vec2>(\"1float\")";
    success = d.hasValue<vec2>("2int");
    EXPECT_EQ(false, success) << "hasValue<vec2>(\"2int\")";
    success = d.hasValue<vec2>("2uint");
    EXPECT_EQ(false, success) << "hasValue<vec2>(\"2uint\")";
    success = d.hasValue<vec2>("2float");
    ASSERT_EQ(true, success) << "hasValue<vec2>(\"2float\")";
    {
        vec2 value;
        success = d.getValue("2float", value);
        ASSERT_EQ(true, success) << "success getValue<vec2>";
        EXPECT_EQ(vec2(1.f, 2.f), value)
            << "correct getValue<vec2>";
    }

    using glm::dvec2;

    success = d.hasValue<dvec2>("1float");
    EXPECT_EQ(false, success) << "hasValue<dvec2>(\"1float\")";
    success = d.hasValue<dvec2>("2int");
    EXPECT_EQ(false, success) << "hasValue<dvec2>(\"2int\")";
    success = d.hasValue<dvec2>("2uint");
    EXPECT_EQ(false, success) << "hasValue<dvec2>(\"2uint\")";
    success = d.hasValue<dvec2>("2float");
    ASSERT_EQ(true, success) << "hasValue<dvec2>(\"2float\")";
    {
        dvec2 value;
        success = d.getValue("2float", value);
        ASSERT_EQ(true, success) << "success getValue<dvec2>";
        EXPECT_EQ(dvec2(1.0, 2.0), value)
            << "correct getValue<dvec2>";
    }

    using glm::ivec2;

    success = d.hasValue<ivec2>("1float");
    EXPECT_EQ(false, success) << "hasValue<ivec2>(\"1float\")";
    success = d.hasValue<ivec2>("2int");
    ASSERT_EQ(true, success) << "hasValue<ivec2>(\"2int\")";
    success = d.hasValue<ivec2>("2uint");
    EXPECT_EQ(false, success) << "hasValue<ivec2>(\"2uint\")";
    success = d.hasValue<ivec2>("2float");
    EXPECT_EQ(false, success) << "hasValue<ivec2>(\"2float\")";
    {
        ivec2 value;
        success = d.getValue("2int", value);
        ASSERT_EQ(true, success) << "success getValue<ivec2>";
        EXPECT_EQ(ivec2(1, 2), value)
            << "correct getValue<ivec2>";
    }

    using glm::uvec2;

    success = d.hasValue<uvec2>("1float");
    EXPECT_EQ(false, success) << "hasValue<uvec2>(\"1float\")";
    success = d.hasValue<uvec2>("2int");
    EXPECT_EQ(false, success) << "hasValue<uvec2>(\"2int\")";
    success = d.hasValue<uvec2>("2uint");
    ASSERT_EQ(true, success) << "hasValue<uvec2>(\"2uint\")";
    success = d.hasValue<uvec2>("2float");
    EXPECT_EQ(false, success) << "hasValue<uvec2>(\"2float\")";
    {
        uvec2 value;
        success = d.getValue("2uint", value);
        ASSERT_EQ(true, success) << "success getValue<uvec2>";
        EXPECT_EQ(uvec2(1, 2), value)
            << "correct getValue<uvec2>";
    }

    using glm::bvec2;

    success = d.hasValue<bvec2>("1float");
    EXPECT_EQ(false, success) << "hasValue<bvec2>(\"1float\")";
    success = d.hasValue<bvec2>("2int");
    ASSERT_EQ(true, success) << "hasValue<bvec2>(\"2int\")";
    success = d.hasValue<bvec2>("2uint");
    EXPECT_EQ(false, success) << "hasValue<bvec2>(\"2uint\")";
    success = d.hasValue<bvec2>("2float");
    EXPECT_EQ(false, success) << "hasValue<bvec2>(\"2float\")";
    {
        bvec2 value;
        success = d.getValue("2int", value);
        ASSERT_EQ(true, success) << "success getValue<bvec2>";
        EXPECT_EQ(bvec2(true, true), value)
            << "correct getValue<bvec2>";
    }

    using glm::vec3;

    success = d.hasValue<vec3>("1float");
    EXPECT_EQ(false, success) << "hasValue<vec3>(\"1float\")";
    success = d.hasValue<vec3>("3int");
    EXPECT_EQ(false, success) << "hasValue<vec3>(\"3int\")";
    success = d.hasValue<vec3>("3uint");
    EXPECT_EQ(false, success) << "hasValue<vec3>(\"3uint\")";
    success = d.hasValue<vec3>("3float");
    ASSERT_EQ(true, success) << "hasValue<vec3>(\"3float\")";
    {
        vec3 value;
        success = d.getValue("3float", value);
        ASSERT_EQ(true, success) << "success getValue<vec3>";
        EXPECT_EQ(vec3(1.f, 2.f, 3.f), value)
            << "correct getValue<vec3>";
    }

    using glm::dvec3;

    success = d.hasValue<dvec3>("1float");
    EXPECT_EQ(false, success) << "hasValue<dvec3>(\"1float\")";
    success = d.hasValue<dvec3>("3int");
    EXPECT_EQ(false, success) << "hasValue<dvec3>(\"3int\")";
    success = d.hasValue<dvec3>("3uint");
    EXPECT_EQ(false, success) << "hasValue<dvec3>(\"3uint\")";
    success = d.hasValue<dvec3>("3float");
    ASSERT_EQ(true, success) << "hasValue<dvec3>(\"3float\")";
    {
        dvec3 value;
        success = d.getValue("3float", value);
        ASSERT_EQ(true, success) << "success getValue<dvec3>";
        EXPECT_EQ(dvec3(1.0, 2.0, 3.0), value)
            << "correct getValue<dvec3>";
    }

    using glm::ivec3;

    success = d.hasValue<ivec3>("1float");
    EXPECT_EQ(false, success) << "hasValue<ivec3>(\"1float\")";
    success = d.hasValue<ivec3>("3int");
    ASSERT_EQ(true, success) << "hasValue<ivec3>(\"3int\")";
    success = d.hasValue<ivec3>("3uint");
    EXPECT_EQ(false, success) << "hasValue<ivec3>(\"3uint\")";
    success = d.hasValue<ivec3>("3float");
    EXPECT_EQ(false, success) << "hasValue<ivec3>(\"3float\")";
    {
        ivec3 value;
        success = d.getValue("3int", value);
        ASSERT_EQ(true, success) << "success getValue<ivec3>";
        EXPECT_EQ(ivec3(1, 2, 3), value)
            << "correct getValue<ivec3>";
    }

    using glm::uvec3;

    success = d.hasValue<uvec3>("1float");
    EXPECT_EQ(false, success) << "hasValue<uvec3>(\"1float\")";
    success = d.hasValue<uvec3>("3int");
    EXPECT_EQ(false, success) << "hasValue<uvec3>(\"3int\")";
    success = d.hasValue<uvec3>("3uint");
    ASSERT_EQ(true, success) << "hasValue<uvec3>(\"3uint\")";
    success = d.hasValue<uvec3>("3float");
    EXPECT_EQ(false, success) << "hasValue<uvec3>(\"3float\")";
    {
        uvec3 value;
        success = d.getValue("3uint", value);
        ASSERT_EQ(true, success) << "success getValue<uvec3>";
        EXPECT_EQ(uvec3(1, 2, 3), value)
            << "correct getValue<uvec3>";
    }

    using glm::bvec3;

    success = d.hasValue<bvec3>("1float");
    EXPECT_EQ(false, success) << "hasValue<bvec3>(\"1float\")";
    success = d.hasValue<bvec3>("3int");
    ASSERT_EQ(true, success) << "hasValue<bvec3>(\"3int\")";
    success = d.hasValue<bvec3>("3uint");
    EXPECT_EQ(false, success) << "hasValue<bvec3>(\"3uint\")";
    success = d.hasValue<bvec3>("3float");
    EXPECT_EQ(false, success) << "hasValue<bvec3>(\"3float\")";
    {
        bvec3 value;
        success = d.getValue("3int", value);
        ASSERT_EQ(true, success) << "success getValue<bvec3>";
        EXPECT_EQ(bvec3(true, true, true), value)
            << "correct getValue<bvec3>";
    }

    using glm::vec4;

    success = d.hasValue<vec4>("1float");
    EXPECT_EQ(false, success) << "hasValue<vec4>(\"1float\")";
    success = d.hasValue<vec4>("4int");
    EXPECT_EQ(false, success) << "hasValue<vec4>(\"4int\")";
    success = d.hasValue<vec4>("4uint");
    EXPECT_EQ(false, success) << "hasValue<vec4>(\"4uint\")";
    success = d.hasValue<vec4>("4float");
    ASSERT_EQ(true, success) << "hasValue<vec4>(\"4float\")";
    {
        vec4 value;
        success = d.getValue("4float", value);
        ASSERT_EQ(true, success) << "success getValue<vec4>";
        EXPECT_EQ(vec4(1.f, 2.f, 3.f, 4.f), value)
            << "correct getValue<vec4>";
    }

    using glm::dvec4;

    success = d.hasValue<dvec4>("1float");
    EXPECT_EQ(false, success) << "hasValue<dvec4>(\"1float\")";
    success = d.hasValue<dvec4>("4int");
    EXPECT_EQ(false, success) << "hasValue<dvec4>(\"4int\")";
    success = d.hasValue<dvec4>("4uint");
    EXPECT_EQ(false, success) << "hasValue<dvec4>(\"4uint\")";
    success = d.hasValue<dvec4>("4float");
    ASSERT_EQ(true, success) << "hasValue<dvec4>(\"4float\")";
    {
        dvec4 value;
        success = d.getValue("4float", value);
        ASSERT_EQ(true, success) << "success getValue<dvec4>";
        EXPECT_EQ(dvec4(1.0, 2.0, 3.0, 4.0), value)
            << "correct getValue<dvec4>";
    }

    using glm::ivec4;

    success = d.hasValue<ivec4>("1float");
    EXPECT_EQ(false, success) << "hasValue<ivec4>(\"1float\")";
    success = d.hasValue<ivec4>("4int");
    ASSERT_EQ(true, success) << "hasValue<ivec4>(\"4int\")";
    success = d.hasValue<ivec4>("4uint");
    EXPECT_EQ(false, success) << "hasValue<ivec4>(\"4uint\")";
    success = d.hasValue<ivec4>("4float");
    EXPECT_EQ(false, success) << "hasValue<ivec4>(\"4float\")";
    {
        ivec4 value;
        success = d.getValue("4int", value);
        ASSERT_EQ(true, success) << "success getValue<ivec4>";
        EXPECT_EQ(ivec4(1, 2, 3, 4), value)
            << "correct getValue<ivec4>";
    }

    using glm::uvec4;

    success = d.hasValue<uvec4>("1float");
    EXPECT_EQ(false, success) << "hasValue<uvec4>(\"1float\")";
    success = d.hasValue<uvec4>("4int");
    EXPECT_EQ(false, success) << "hasValue<uvec4>(\"4int\")";
    success = d.hasValue<uvec4>("4uint");
    ASSERT_EQ(true, success) << "hasValue<uvec4>(\"4uint\")";
    success = d.hasValue<uvec4>("4float");
    EXPECT_EQ(false, success) << "hasValue<uvec4>(\"4float\")";
    {
        uvec4 value;
        success = d.getValue("4uint", value);
        ASSERT_EQ(true, success) << "success getValue<uvec4>";
        EXPECT_EQ(uvec4(1, 2, 3, 4), value)
            << "correct getValue<uvec4>";
    }

    using glm::bvec4;

    success = d.hasValue<bvec4>("1float");
    EXPECT_EQ(false, success) << "hasValue<bvec4>(\"1float\")";
    success = d.hasValue<bvec4>("4int");
    ASSERT_EQ(true, success) << "hasValue<bvec4>(\"4int\")";
    success = d.hasValue<bvec4>("4uint");
    EXPECT_EQ(false, success) << "hasValue<bvec4>(\"4uint\")";
    success = d.hasValue<bvec4>("4float");
    EXPECT_EQ(false, success) << "hasValue<bvec4>(\"4float\")";
    {
        bvec4 value;
        success = d.getValue("4int", value);
        ASSERT_EQ(true, success) << "success getValue<bvec4>";
        EXPECT_EQ(bvec4(true, true, true, true), value)
            << "correct getValue<bvec4>";
    }

    using glm::mat2x2;

    success = d.hasValue<mat2x2>("1float");
    EXPECT_EQ(false, success) << "hasValue<mat2x2>(\"1float\")";
    success = d.hasValue<mat2x2>("4int");
    EXPECT_EQ(false, success) << "hasValue<mat2x2>(\"4int\")";
    success = d.hasValue<mat2x2>("4uint");
    EXPECT_EQ(false, success) << "hasValue<mat2x2>(\"4uint\")";
    success = d.hasValue<mat2x2>("4float");
    ASSERT_EQ(true, success) << "hasValue<mat2x2>(\"4float\")";
    {
        mat2x2 value;
        success = d.getValue("4float", value);
        ASSERT_EQ(true, success) << "success getValue<mat2x2>";
        EXPECT_EQ(mat2x2(1.f, 2.f, 3.f, 4.f), value)
            << "correct getValue<mat2x2>";
    }

    using glm::mat2x3;

    success = d.hasValue<mat2x3>("1float");
    EXPECT_EQ(false, success) << "hasValue<mat2x3>(\"1float\")";
    success = d.hasValue<mat2x3>("6int");
    EXPECT_EQ(false, success) << "hasValue<mat2x3>(\"6int\")";
    success = d.hasValue<mat2x3>("6uint");
    EXPECT_EQ(false, success) << "hasValue<mat2x3>(\"6uint\")";
    success = d.hasValue<mat2x3>("6float");
    ASSERT_EQ(true, success) << "hasValue<mat2x3>(\"6float\")";
    {
        mat2x3 value;
        success = d.getValue("6float", value);
        ASSERT_EQ(true, success) << "success getValue<mat2x3>";
        EXPECT_EQ(mat2x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f), value)
            << "correct getValue<mat2x3>";
    }

    using glm::mat2x4;

    success = d.hasValue<mat2x4>("1float");
    EXPECT_EQ(false, success) << "hasValue<mat2x4>(\"1float\")";
    success = d.hasValue<mat2x4>("8int");
    EXPECT_EQ(false, success) << "hasValue<mat2x4>(\"8int\")";
    success = d.hasValue<mat2x4>("8uint");
    EXPECT_EQ(false, success) << "hasValue<mat2x4>(\"8uint\")";
    success = d.hasValue<mat2x4>("8float");
    ASSERT_EQ(true, success) << "hasValue<mat2x4>(\"8float\")";
    {
        mat2x4 value;
        success = d.getValue("8float", value);
        ASSERT_EQ(true, success) << "success getValue<mat2x4>";
        EXPECT_EQ(mat2x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f), value)
            << "correct getValue<mat2x4>";
    }

    using glm::mat3x2;

    success = d.hasValue<mat3x2>("1float");
    EXPECT_EQ(false, success) << "hasValue<mat2x2>(\"1float\")";
    success = d.hasValue<mat3x2>("6int");
    EXPECT_EQ(false, success) << "hasValue<mat3x2>(\"6int\")";
    success = d.hasValue<mat3x2>("6uint");
    EXPECT_EQ(false, success) << "hasValue<mat3x2>(\"6uint\")";
    success = d.hasValue<mat3x2>("6float");
    ASSERT_EQ(true, success) << "hasValue<mat3x2>(\"6float\")";
    {
        mat3x2 value;
        success = d.getValue("6float", value);
        ASSERT_EQ(true, success) << "success getValue<mat3x2>";
        EXPECT_EQ(mat3x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f), value)
            << "correct getValue<mat3x2>";
    }

    using glm::mat3x3;

    success = d.hasValue<mat3x3>("1float");
    EXPECT_EQ(false, success) << "hasValue<mat3x3>(\"1float\")";
    success = d.hasValue<mat3x3>("9int");
    EXPECT_EQ(false, success) << "hasValue<mat3x3>(\"9int\")";
    success = d.hasValue<mat3x3>("9uint");
    EXPECT_EQ(false, success) << "hasValue<mat3x3>(\"9uint\")";
    success = d.hasValue<mat3x3>("9float");
    ASSERT_EQ(true, success) << "hasValue<mat3x3>(\"9float\")";
    {
        mat3x3 value;
        success = d.getValue("9float", value);
        ASSERT_EQ(true, success) << "success getValue<mat3x3>";
        EXPECT_EQ(mat3x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f), value)
            << "correct getValue<mat3x2>";
    }

    using glm::mat3x4;

    success = d.hasValue<mat3x4>("1float");
    EXPECT_EQ(false, success) << "hasValue<mat3x4>(\"1float\")";
    success = d.hasValue<mat3x4>("12int");
    EXPECT_EQ(false, success) << "hasValue<mat3x4>(\"12int\")";
    success = d.hasValue<mat3x4>("12uint");
    EXPECT_EQ(false, success) << "hasValue<mat3x4>(\"12uint\")";
    success = d.hasValue<mat3x4>("12float");
    ASSERT_EQ(true, success) << "hasValue<mat3x4>(\"12float\")";
    {
        mat3x4 value;
        mat3x4 s = mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f);
        success = d.getValue("12float", value);
        ASSERT_EQ(true, success) << "success getValue<mat3x4>";
        EXPECT_EQ(mat3x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f),
                  value)
              << "correct getValue<mat3x4>";
    }

    using glm::mat4x2;

    success = d.hasValue<mat4x2>("1float");
    EXPECT_EQ(false, success) << "hasValue<mat4x2>(\"1float\")";
    success = d.hasValue<mat4x2>("8int");
    EXPECT_EQ(false, success) << "hasValue<mat4x2>(\"8int\")";
    success = d.hasValue<mat4x2>("8uint");
    EXPECT_EQ(false, success) << "hasValue<mat4x2>(\"8uint\")";
    success = d.hasValue<mat4x2>("8float");
    ASSERT_EQ(true, success) << "hasValue<mat4x2>(\"8float\")";
    {
        mat4x2 value;
        success = d.getValue("8float", value);
        ASSERT_EQ(true, success) << "success getValue<mat4x2>";
        EXPECT_EQ(mat4x2(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f), value)
            << "correct getValue<mat4x2>";
    }

    using glm::mat4x3;

    success = d.hasValue<mat4x3>("1float");
    EXPECT_EQ(false, success) << "hasValue<mat4x3>(\"1float\")";
    success = d.hasValue<mat4x3>("12int");
    EXPECT_EQ(false, success) << "hasValue<mat4x3>(\"12int\")";
    success = d.hasValue<mat4x3>("12uint");
    EXPECT_EQ(false, success) << "hasValue<mat4x3>(\"12uint\")";
    success = d.hasValue<mat4x3>("12float");
    ASSERT_EQ(true, success) << "hasValue<mat4x3>(\"12float\")";
    {
        mat4x3 value;
        success = d.getValue("12float", value);
        ASSERT_EQ(true, success) << "success getValue<mat4x3>";
        EXPECT_EQ(mat4x3(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f),
                  value)
              << "correct getValue<mat4x3>";
    }

    using glm::mat4x4;

    success = d.hasValue<mat4x4>("1float");
    EXPECT_EQ(false, success) << "hasValue<mat4x4>(\"1float\")";
    success = d.hasValue<mat4x4>("16int");
    EXPECT_EQ(false, success) << "hasValue<mat4x4>(\"16int\")";
    success = d.hasValue<mat4x4>("16uint");
    EXPECT_EQ(false, success) << "hasValue<mat4x4>(\"16uint\")";
    success = d.hasValue<mat4x4>("16float");
    ASSERT_EQ(true, success) << "hasValue<mat4x4>(\"16float\")";
    {
        mat4x4 value;
        success = d.getValue("16float", value);
        ASSERT_EQ(true, success) << "success getValue<mat4x4>";
        EXPECT_EQ(mat4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f,
                         13.f, 14.f, 15.f, 16.f),
                  value)
              << "correct getValue<mat4x4>";
    }

    using glm::dmat2x2;

    success = d.hasValue<dmat2x2>("1float");
    EXPECT_EQ(false, success) << "hasValue<dmat2x2>(\"1float\")";
    success = d.hasValue<dmat2x2>("4int");
    EXPECT_EQ(false, success) << "hasValue<dmat2x2>(\"4int\")";
    success = d.hasValue<dmat2x2>("4uint");
    EXPECT_EQ(false, success) << "hasValue<dmat2x2>(\"4uint\")";
    success = d.hasValue<dmat2x2>("4float");
    ASSERT_EQ(true, success) << "hasValue<dmat2x2>(\"4float\")";
    {
        dmat2x2 value;
        success = d.getValue("4float", value);
        ASSERT_EQ(true, success) << "success getValue<dmat2x2>";
        EXPECT_EQ(dmat2x2(1.0, 2.0, 3.0, 4.0), value) << "correct getValue<dmat2x2>";
    }

    using glm::dmat2x3;

    success = d.hasValue<dmat2x3>("1float");
    EXPECT_EQ(false, success) << "hasValue<dmat2x3>(\"1float\")";
    success = d.hasValue<dmat2x3>("6int");
    EXPECT_EQ(false, success) << "hasValue<dmat2x3>(\"6int\")";
    success = d.hasValue<dmat2x3>("6uint");
    EXPECT_EQ(false, success) << "hasValue<dmat2x3>(\"6uint\")";
    success = d.hasValue<dmat2x3>("6float");
    ASSERT_EQ(true, success) << "hasValue<dmat2x3>(\"6float\")";
    {
        dmat2x3 value;
        success = d.getValue("6float", value);
        ASSERT_EQ(true, success) << "success getValue<dmat2x3>";
        EXPECT_EQ(dmat2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0), value)
              << "correct getValue<dmat2x3>";
    }

    using glm::dmat2x4;

    success = d.hasValue<dmat2x4>("1float");
    EXPECT_EQ(false, success) << "hasValue<dmat2x4>(\"1float\")";
    success = d.hasValue<dmat2x4>("8int");
    EXPECT_EQ(false, success) << "hasValue<dmat2x4>(\"8int\")";
    success = d.hasValue<mat2x4>("8uint");
    EXPECT_EQ(false, success) << "hasValue<dmat2x4>(\"8uint\")";
    success = d.hasValue<dmat2x4>("8float");
    ASSERT_EQ(true, success) << "hasValue<dmat2x4>(\"8float\")";
    {
        dmat2x4 value;
        success = d.getValue("8float", value);
        ASSERT_EQ(true, success) << "success getValue<dmat2x4>";
        EXPECT_EQ(dmat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0), value)
            << "correct getValue<dmat2x4>";
    }

    using glm::dmat3x2;

    success = d.hasValue<dmat3x2>("1float");
    EXPECT_EQ(false, success) << "hasValue<dmat2x2>(\"1float\")";
    success = d.hasValue<dmat3x2>("6int");
    EXPECT_EQ(false, success) << "hasValue<dmat3x2>(\"6int\")";
    success = d.hasValue<dmat3x2>("6uint");
    EXPECT_EQ(false, success) << "hasValue<dmat3x2>(\"6uint\")";
    success = d.hasValue<dmat3x2>("6float");
    ASSERT_EQ(true, success) << "hasValue<dmat3x2>(\"6float\")";
    {
        dmat3x2 value;
        success = d.getValue("6float", value);
        ASSERT_EQ(true, success) << "success getValue<dmat3x2>";
        EXPECT_EQ(dmat3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0), value)
            << "correct getValue<dmat3x2>";
    }

    using glm::dmat3x3;

    success = d.hasValue<dmat3x3>("1float");
    EXPECT_EQ(false, success) << "hasValue<dmat3x3>(\"1float\")";
    success = d.hasValue<dmat3x3>("9int");
    EXPECT_EQ(false, success) << "hasValue<dmat3x3>(\"9int\")";
    success = d.hasValue<dmat3x3>("9uint");
    EXPECT_EQ(false, success) << "hasValue<dmat3x3>(\"9uint\")";
    success = d.hasValue<dmat3x3>("9float");
    ASSERT_EQ(true, success) << "hasValue<dmat3x3>(\"9float\")";
    {
        dmat3x3 value;
        success = d.getValue("9float", value);
        ASSERT_EQ(true, success) << "success getValue<dmat3x3>";
        EXPECT_EQ(dmat3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0), value)
            << "correct getValue<dmat3x3>";
    }

    using glm::dmat3x4;

    success = d.hasValue<dmat3x4>("1float");
    EXPECT_EQ(false, success) << "hasValue<dmat3x4>(\"1float\")";
    success = d.hasValue<dmat3x4>("12int");
    EXPECT_EQ(false, success) << "hasValue<dmat3x4>(\"12int\")";
    success = d.hasValue<dmat3x4>("12uint");
    EXPECT_EQ(false, success) << "hasValue<dmat3x4>(\"12uint\")";
    success = d.hasValue<dmat3x4>("12float");
    ASSERT_EQ(true, success) << "hasValue<dmat3x4>(\"12float\")";
    {
        dmat3x4 value;
        success = d.getValue("12float", value);
        ASSERT_EQ(true, success) << "success getValue<dmat3x4>";
        EXPECT_EQ(dmat3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0),
                  value)
              << "correct getValue<dmat3x4>";
    }

    using glm::dmat4x2;

    success = d.hasValue<dmat4x2>("1float");
    EXPECT_EQ(false, success) << "hasValue<dmat4x2>(\"1float\")";
    success = d.hasValue<dmat4x2>("8int");
    EXPECT_EQ(false, success) << "hasValue<dmat4x2>(\"8int\")";
    success = d.hasValue<dmat4x2>("8uint");
    EXPECT_EQ(false, success) << "hasValue<dmat4x2>(\"8uint\")";
    success = d.hasValue<dmat4x2>("8float");
    ASSERT_EQ(true, success) << "hasValue<dmat4x2>(\"8float\")";
    {
        dmat4x2 value;
        success = d.getValue("8float", value);
        ASSERT_EQ(true, success) << "success getValue<dmat4x2>";
        EXPECT_EQ(dmat4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0),
            value)
            << "correct getValue<dmat4x2>";
    }

    using glm::dmat4x3;

    success = d.hasValue<dmat4x3>("1float");
    EXPECT_EQ(false, success) << "hasValue<dmat4x3>(\"1float\")";
    success = d.hasValue<dmat4x3>("12int");
    EXPECT_EQ(false, success) << "hasValue<dmat4x3>(\"12int\")";
    success = d.hasValue<dmat4x3>("12uint");
    EXPECT_EQ(false, success) << "hasValue<dmat4x3>(\"12uint\")";
    success = d.hasValue<dmat4x3>("12float");
    ASSERT_EQ(true, success) << "hasValue<dmat4x3>(\"12float\")";
    {
        dmat4x3 value;
        success = d.getValue("12float", value);
        ASSERT_EQ(true, success) << "success getValue<dmat4x3>";
        EXPECT_EQ(dmat4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0),
            value)
            << "correct getValue<dmat4x3>";
    }

    using glm::dmat4x4;

    success = d.hasValue<dmat4x4>("1float");
    EXPECT_EQ(false, success) << "hasValue<dmat4x4>(\"1float\")";
    success = d.hasValue<dmat4x4>("16int");
    EXPECT_EQ(false, success) << "hasValue<dmat4x4>(\"16int\")";
    success = d.hasValue<dmat4x4>("16uint");
    EXPECT_EQ(false, success) << "hasValue<dmat4x4>(\"16uint\")";
    success = d.hasValue<dmat4x4>("16float");
    ASSERT_EQ(true, success) << "hasValue<dmat4x4>(\"16float\")";
    {
        dmat4x4 value;
        success = d.getValue("16float", value);
        ASSERT_EQ(true, success) << "success getValue<dmat4x4>";
        EXPECT_EQ(dmat4x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0,
                          13.0, 14.0, 15.0, 16.0),
                  value)
              << "correct getValue<dmat4x4>";
    }
}

TEST_F(DictionaryTest, RecursiveAccessHasValue) {
	ghoul::Dictionary d = { { "a", 1 } };
	ghoul::Dictionary e = d;
	ghoul::Dictionary f = d;
	ghoul::Dictionary g = d;
	f.setValue("g", g);
	e.setValue("f", f);
	d.setValue("e", e);

	ASSERT_EQ(true, d.hasValue<int>("a")) << "a";
	ASSERT_EQ(true, d.hasValue<ghoul::Dictionary>("e")) << "e";
	ASSERT_EQ(true, d.hasValue<int>("e.a")) << "e.a";
	ASSERT_EQ(true, d.hasValue<ghoul::Dictionary>("e.f")) << "e.f";
	ASSERT_EQ(true, d.hasValue<int>("e.f.a")) << "e.f.a";
	ASSERT_EQ(true, d.hasValue<ghoul::Dictionary>("e.f.g")) << "e.f.g";
	ASSERT_EQ(true, d.hasValue<int>("e.f.g.a")) << "e.f.g.a";

	ASSERT_EQ(false, d.hasValue<int>("e.g")) << "e.g";
}

TEST_F(DictionaryTest, RecursiveAccessGetValue) {
	ghoul::Dictionary d = { { "a", 1 } };
	ghoul::Dictionary e = d;
	ghoul::Dictionary f = d;
	ghoul::Dictionary g = d;
	f.setValue("g", g);
	e.setValue("f", f);
	d.setValue("e", e);

	int value;
	ghoul::Dictionary dictValue;
	bool success;
	success = d.getValue("a", value);
	ASSERT_EQ(true, success) << "success 'a'";
	ASSERT_EQ(1, value) << "value 'a'";
	success = d.getValue("e", dictValue);
	ASSERT_EQ(true, success) << "success 'e'";
	
	success = d.getValue("e.a", value);
	ASSERT_EQ(true, success) << "success 'e.a'";
	ASSERT_EQ(1, value) << "value 'e.a'";
	success = d.getValue("e.f", dictValue);
	ASSERT_EQ(true, success) << "success 'e.f'";

	success = d.getValue("e.f.a", value);
	ASSERT_EQ(true, success) << "success 'e.f.a'";
	ASSERT_EQ(1, value) << "value 'e.f.a'";
	success = d.getValue("e.f.g", dictValue);
	ASSERT_EQ(true, success) << "success 'e.f.g'";

	success = d.getValue("e.f.g.a", value);
	ASSERT_EQ(true, success) << "success 'e.f.g.a'";
	ASSERT_EQ(1, value) << "value 'e.f.g.a'";


	// false values
	success = d.getValue("e.g", value);
	ASSERT_EQ(false, success) << "success 'e.g'";
}

TEST_F(DictionaryTest, RecursiveAccessSetValue) {
	ghoul::Dictionary d = { { "a", 1 } };
	ghoul::Dictionary e = d;
	ghoul::Dictionary f = d;
	ghoul::Dictionary g = d;
	f.setValue("g", g);
	e.setValue("f", f);
	d.setValue("e", e);

	int value;
	bool success;
	success = d.getValue("e.f.g.a", value);
	ASSERT_EQ(true, success) << "success 'e.f.g.a'";
	ASSERT_EQ(1, value) << "value 'e.f.g.a'";

	success = d.setValue("e.f.g.b", 2);
	ASSERT_EQ(true, success) << "success set 'e.f.g.b'";
	success = d.getValue("e.f.g.b", value);
	ASSERT_EQ(true, success) << "success 'e.f.g.b'";
	ASSERT_EQ(2, value) << "value 'e.f.g.b'";

	// false values
	success = d.setValue("e.g.a", 1);
	ASSERT_EQ(false, success) << "success 'e.g.a'";
}
