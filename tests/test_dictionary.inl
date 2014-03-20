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
#include <glm/glm.hpp>
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
		_d = new ghoul::Dictionary({
			{ "bool", false },
			{ "char", char(0) },
			{ "signed char", static_cast<signed char>(0) },
			{ "unsigned char", static_cast<unsigned char>(0) },
			{ "wchar_t", wchar_t(0) },
			{ "short", short(0) },
			{ "unsigned short", static_cast<unsigned short>(0) },
			{ "int", int(0) },
			{ "unsigned int", static_cast<unsigned int>(0) },
			{ "long long", static_cast<long long>(0) },
			{ "unsigned long long", static_cast<unsigned long long>(0) },
			{ "float", float(0) },
			{ "double", double(0) },
			{ "long double", static_cast<long double>(0) },
			{ "dictionary", dict },
			{ "vec2", glm::vec2(0) },
			{ "dvec2", glm::dvec2(0) },
			{ "ivec2", glm::ivec2(0) },
			{ "uvec2", glm::uvec2(0) },
			{ "bvec2", glm::bvec2(false) },
			{ "vec3", glm::vec3(0) },
			{ "dvec3", glm::dvec3(0) },
			{ "ivec3", glm::ivec3(0) },
			{ "uvec3", glm::uvec3(0) },
			{ "bvec3", glm::bvec3(false) },
			{ "vec4", glm::vec4(0) },
			{ "dvec4", glm::dvec4(0) },
			{ "ivec4", glm::ivec4(0) },
			{ "uvec4", glm::uvec4(0) },
			{ "bvec4", glm::bvec4(false) },
			{ "mat2x2", glm::mat2x2(0) },
			{ "mat2x3", glm::mat2x3(0) },
			{ "mat2x4", glm::mat2x4(0) },
			{ "mat3x2", glm::mat3x2(0) },
			{ "mat3x3", glm::mat3x3(0) },
			{ "mat3x4", glm::mat3x4(0) },
			{ "mat4x2", glm::mat4x2(0) },
			{ "mat4x3", glm::mat4x3(0) },
			{ "mat4x4", glm::mat4x4(0) },
			{ "dmat2x2", glm::dmat2x2(0) },
			{ "dmat2x3", glm::dmat2x3(0) },
			{ "dmat2x4", glm::dmat2x4(0) },
			{ "dmat3x2", glm::dmat3x2(0) },
			{ "dmat3x3", glm::dmat3x3(0) },
			{ "dmat3x4", glm::dmat3x4(0) },
			{ "dmat4x2", glm::dmat4x2(0) },
			{ "dmat4x3", glm::dmat4x3(0) },
			{ "dmat4x4", glm::dmat4x4(0) }
		});
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
		EXPECT_EQ(false, value) << "value 'bool'";
	}
	{
		char value;
		const bool success = _d->getValue("char", value);
		EXPECT_EQ(true, success) << "success 'char'";
		EXPECT_EQ(char(0), value) << "value 'char'";
	}
	{
		signed char value;
		const bool success = _d->getValue("signed char", value);
		EXPECT_EQ(true, success) << "success 'signed char'";
		EXPECT_EQ(static_cast<signed char>(0), value) << "value 'signed char'";
	}
	{
		unsigned char value;
		const bool success = _d->getValue("unsigned char", value);
		EXPECT_EQ(true, success) << "success 'unsigned char'";
		EXPECT_EQ(static_cast<unsigned char>(0), value) << "value 'unsigned char'";
	}
	{
		wchar_t value;
		const bool success = _d->getValue("wchar_t", value);
		EXPECT_EQ(true, success) << "success 'wchar_t'";
		EXPECT_EQ(wchar_t(0), value) << "value 'wchar_t'";
	}
	{
		short value;
		const bool success = _d->getValue("short", value);
		EXPECT_EQ(true, success) << "success 'short'";
		EXPECT_EQ(short(0), value) << "value 'short'";
	}
	{
		unsigned short value;
		const bool success = _d->getValue("unsigned short", value);
		EXPECT_EQ(true, success) << "success 'unsigned short'";
		EXPECT_EQ(static_cast<unsigned short>(0), value) << "value 'unsigned short'";
	}
	{
		int value;
		const bool success = _d->getValue("int", value);
		EXPECT_EQ(true, success) << "success 'int'";
		EXPECT_EQ(int(0), value) << "value 'int'";
	}
	{
		unsigned int value;
		const bool success = _d->getValue("unsigned int", value);
		EXPECT_EQ(true, success) << "success 'unsigned int'";
		EXPECT_EQ(static_cast<unsigned int>(0), value) << "value 'unsigned int'";
	}
	{
		long long value;
		const bool success = _d->getValue("long long", value);
		EXPECT_EQ(true, success) << "success 'long long'";
		EXPECT_EQ(static_cast<long long>(0), value) << "value 'long long'";
	}
	{
		unsigned long long value;
		const bool success = _d->getValue("unsigned long long", value);
		EXPECT_EQ(true, success) << "success 'unsigned long long'";
		EXPECT_EQ(static_cast<unsigned long long>(0), value) << "value 'unsigned long long'";
	}
	{
		float value;
		const bool success = _d->getValue("float", value);
		EXPECT_EQ(true, success) << "success 'float'";
		EXPECT_EQ(float(0), value) << "value 'float'";
	}
	{
		double value;
		const bool success = _d->getValue("double", value);
		EXPECT_EQ(true, success) << "success 'double'";
		EXPECT_EQ(double(0), value) << "value 'double'";
	}
	{
		long double value;
		const bool success = _d->getValue("long double", value);
		EXPECT_EQ(true, success) << "success 'long double'";
		EXPECT_EQ(static_cast<long double>(0), value) << "value 'long double'";
	}
	{
		glm::vec2 value;
		const bool success = _d->getValue("vec2", value);
		EXPECT_EQ(true, success) << "success 'glm::vec2'";
		EXPECT_EQ(glm::vec2(0), value) << "value 'glm::vec2'";
	}
	{
		glm::dvec2 value;
		const bool success = _d->getValue("dvec2", value);
		EXPECT_EQ(true, success) << "success 'glm::dvec2'";
		EXPECT_EQ(glm::dvec2(0), value) << "value 'glm::dvec2'";
	}
	{
		glm::ivec2 value;
		const bool success = _d->getValue("ivec2", value);
		EXPECT_EQ(true, success) << "success 'glm::ivec2'";
		EXPECT_EQ(glm::ivec2(0), value) << "value 'glm::ivec2'";
	}
	{
		glm::uvec2 value;
		const bool success = _d->getValue("uvec2", value);
		EXPECT_EQ(true, success) << "success 'glm::uvec2'";
		EXPECT_EQ(glm::uvec2(0), value) << "value 'glm::uvec2'";
	}
	{
		glm::bvec2 value;
		const bool success = _d->getValue("bvec2", value);
		EXPECT_EQ(true, success) << "success 'glm::bvec2'";
		EXPECT_EQ(glm::bvec2(false), value) << "value 'glm::bvec2'";
	}
	{
		glm::vec3 value;
		const bool success = _d->getValue("vec3", value);
		EXPECT_EQ(true, success) << "success 'glm::vec3'";
		EXPECT_EQ(glm::vec3(0), value) << "value 'glm::vec3'";
	}
	{
		glm::dvec3 value;
		const bool success = _d->getValue("dvec3", value);
		EXPECT_EQ(true, success) << "success 'glm::dvec3'";
		EXPECT_EQ(glm::dvec3(0), value) << "value 'glm::dvec3'";
	}
	{
		glm::ivec3 value;
		const bool success = _d->getValue("ivec3", value);
		EXPECT_EQ(true, success) << "success 'glm::ivec3'";
		EXPECT_EQ(glm::ivec3(0), value) << "value 'glm::ivec3'";
	}
	{
		glm::uvec3 value;
		const bool success = _d->getValue("uvec3", value);
		EXPECT_EQ(true, success) << "success 'glm::uvec3'";
		EXPECT_EQ(glm::uvec3(0), value) << "value 'glm::uvec3'";
	}
	{
		glm::bvec3 value;
		const bool success = _d->getValue("bvec3", value);
		EXPECT_EQ(true, success) << "success 'glm::bvec3'";
		EXPECT_EQ(glm::bvec3(false), value) << "value 'glm::bvec3'";
	}
	{
		glm::vec4 value;
		const bool success = _d->getValue("vec4", value);
		EXPECT_EQ(true, success) << "success 'glm::vec4'";
		EXPECT_EQ(glm::vec4(0), value) << "value 'glm::vec4'";
	}
	{
		glm::dvec4 value;
		const bool success = _d->getValue("dvec4", value);
		EXPECT_EQ(true, success) << "success 'glm::dvec4'";
		EXPECT_EQ(glm::dvec4(0), value) << "value 'glm::dvec4'";
	}
	{
		glm::ivec4 value;
		const bool success = _d->getValue("ivec4", value);
		EXPECT_EQ(true, success) << "success 'glm::ivec4'";
		EXPECT_EQ(glm::ivec4(0), value) << "value 'glm::ivec4'";
	}
	{
		glm::uvec4 value;
		const bool success = _d->getValue("uvec4", value);
		EXPECT_EQ(true, success) << "success 'glm::uvec4'";
		EXPECT_EQ(glm::uvec4(0), value) << "value 'glm::uvec4'";
	}
	{
		glm::bvec4 value;
		const bool success = _d->getValue("bvec4", value);
		EXPECT_EQ(true, success) << "success 'glm::bvec4'";
		EXPECT_EQ(glm::bvec4(false), value) << "value 'glm::bvec4'";
	}
	{
		glm::mat2x2 value;
		const bool success = _d->getValue("mat2x2", value);
		EXPECT_EQ(true, success) << "success 'glm::mat2x2'";
		EXPECT_EQ(glm::mat2x2(0), value) << "value 'glm::mat2x2'";
	}
	{
		glm::mat2x3 value;
		const bool success = _d->getValue("mat2x3", value);
		EXPECT_EQ(true, success) << "success 'glm::mat2x3'";
		EXPECT_EQ(glm::mat2x3(0), value) << "value 'glm::mat2x3'";
	}
	{
		glm::mat2x4 value;
		const bool success = _d->getValue("mat2x4", value);
		EXPECT_EQ(true, success) << "success 'glm::mat2x4'";
		EXPECT_EQ(glm::mat2x4(0), value) << "value 'glm::mat2x4'";
	}
	{
		glm::mat3x2 value;
		const bool success = _d->getValue("mat3x2", value);
		EXPECT_EQ(true, success) << "success 'glm::mat3x2'";
		EXPECT_EQ(glm::mat3x2(0), value) << "value 'glm::mat3x2'";
	}
	{
		glm::mat3x3 value;
		const bool success = _d->getValue("mat3x3", value);
		EXPECT_EQ(true, success) << "success 'glm::mat3x3'";
		EXPECT_EQ(glm::mat3x3(0), value) << "value 'glm::mat3x3'";
	}
	{
		glm::mat3x4 value;
		const bool success = _d->getValue("mat3x4", value);
		EXPECT_EQ(true, success) << "success 'glm::mat3x4'";
		EXPECT_EQ(glm::mat3x4(0), value) << "value 'glm::mat3x4'";
	}
	{
		glm::mat4x2 value;
		const bool success = _d->getValue("mat4x2", value);
		EXPECT_EQ(true, success) << "success 'glm::mat4x2'";
		EXPECT_EQ(glm::mat4x2(0), value) << "value 'glm::mat4x2'";
	}
	{
		glm::mat4x3 value;
		const bool success = _d->getValue("mat4x3", value);
		EXPECT_EQ(true, success) << "success 'glm::mat4x3'";
		EXPECT_EQ(glm::mat4x3(0), value) << "value 'glm::mat4x3'";
	}
	{
		glm::mat4x4 value;
		const bool success = _d->getValue("mat4x4", value);
		EXPECT_EQ(true, success) << "success 'glm::mat4x4'";
		EXPECT_EQ(glm::mat4x4(0), value) << "value 'glm::mat4x4'";
	}
	{
		glm::dmat2x2 value;
		const bool success = _d->getValue("dmat2x2", value);
		EXPECT_EQ(true, success) << "success 'glm::dmat2x2'";
		EXPECT_EQ(glm::dmat2x2(0), value) << "value 'glm::dmat2x2'";
	}
	{
		glm::dmat2x3 value;
		const bool success = _d->getValue("dmat2x3", value);
		EXPECT_EQ(true, success) << "success 'glm::dmat2x3'";
		EXPECT_EQ(glm::dmat2x3(0), value) << "value 'glm::dmat2x3'";
	}
	{
		glm::dmat2x4 value;
		const bool success = _d->getValue("dmat2x4", value);
		EXPECT_EQ(true, success) << "success 'glm::dmat2x4'";
		EXPECT_EQ(glm::dmat2x4(0), value) << "value 'glm::dmat2x4'";
	}
	{
		glm::dmat3x2 value;
		const bool success = _d->getValue("dmat3x2", value);
		EXPECT_EQ(true, success) << "success 'glm::dmat3x2'";
		EXPECT_EQ(glm::dmat3x2(0), value) << "value 'glm::dmat3x2'";
	}
	{
		glm::dmat3x3 value;
		const bool success = _d->getValue("dmat3x3", value);
		EXPECT_EQ(true, success) << "success 'glm::dmat3x3'";
		EXPECT_EQ(glm::dmat3x3(0), value) << "value 'glm::dmat3x3'";
	}
	{
		glm::dmat3x4 value;
		const bool success = _d->getValue("dmat3x4", value);
		EXPECT_EQ(true, success) << "success 'glm::dmat3x4'";
		EXPECT_EQ(glm::dmat3x4(0), value) << "value 'glm::dmat3x4'";
	}
	{
		glm::dmat4x2 value;
		const bool success = _d->getValue("dmat4x2", value);
		EXPECT_EQ(true, success) << "success 'glm::dmat4x2'";
		EXPECT_EQ(glm::dmat4x2(0), value) << "value 'glm::dmat4x2'";
	}
	{
		glm::dmat4x3 value;
		const bool success = _d->getValue("dmat4x3", value);
		EXPECT_EQ(true, success) << "success 'glm::dmat4x3'";
		EXPECT_EQ(glm::dmat4x3(0), value) << "value 'glm::dmat4x3'";
	}
	{
		glm::dmat4x4 value;
		const bool success = _d->getValue("dmat4x4", value);
		EXPECT_EQ(true, success) << "success 'glm::dmat4x4'";
		EXPECT_EQ(glm::dmat4x4(0), value) << "value 'glm::dmat4x4'";
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
