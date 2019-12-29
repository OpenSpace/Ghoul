/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2019                                                               *
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

#include "catch2/catch.hpp"

#include <ghoul/misc/buffer.h>

TEST_CASE("Buffer: String", "[buffer]") {
    const std::string s1 = "first";
    const std::string s2 = "second";
    std::string s4, s5, s6;

    ghoul::Buffer b;
    b.serialize(s1);
    b.serialize(s2);
    b.serialize("third");

    b.deserialize(s4);
    b.deserialize(s5);
    b.deserialize(s6);

    REQUIRE(s1 == s4);
    REQUIRE(s2 == s5);
    REQUIRE(s6 == "third");
}

TEST_CASE("Buffer: MixedTypes", "[buffer]") {
    const std::string s1 = "string";
    const int i1 = 42;
    const double d1 = 123.456;
    const unsigned long long u1 = 123456789;

    ghoul::Buffer b;
    b.serialize(s1);
    b.serialize(i1);
    b.serialize(d1);
    b.serialize(u1);

    std::string s2;
    b.deserialize(s2);
    int i2;
    b.deserialize(i2);
    double d2;
    b.deserialize(d2);
    unsigned long long u2;
    b.deserialize(u2);

    REQUIRE(s1 == s2);
    REQUIRE(i1 == i2);
    REQUIRE(d1 == d2);
    REQUIRE(u1 == u2);
}

TEST_CASE("Buffer: Copy", "[buffer]") {
    const std::string s1 = "string";
    const int i1 = 42;
    const double d1 = 123.456;
    const unsigned long long u1 = 123456789;

    ghoul::Buffer b;
    b.serialize(s1);
    b.serialize(i1);
    b.serialize(d1);
    b.serialize(u1);

    // copy
    ghoul::Buffer b2 = b;
    b.reset();

    std::string s2;
    b2.deserialize(s2);
    int i2;
    b2.deserialize(i2);
    double d2;
    b2.deserialize(d2);
    unsigned long long u2;
    b2.deserialize(u2);

    REQUIRE(s1 == s2);
    REQUIRE(i1 == i2);
    REQUIRE(d1 == d2);
    REQUIRE(u1 == u2);
}

TEST_CASE("Buffer: Move", "[buffer]") {
    const std::string s1 = "string";
    const int i1 = 42;
    const double d1 = 123.456;
    const unsigned long long u1 = 123456789;

    ghoul::Buffer b;
    b.serialize(s1);
    b.serialize(i1);
    b.serialize(d1);
    b.serialize(u1);

    // move
    ghoul::Buffer b2 = std::move(b);
    b.reset();

    std::string s2;
    b2.deserialize(s2);
    int i2;
    b2.deserialize(i2);
    double d2;
    b2.deserialize(d2);
    unsigned long long u2;
    b2.deserialize(u2);

    REQUIRE(s1 == s2);
    REQUIRE(i1 == i2);
    REQUIRE(d1 == d2);
    REQUIRE(u1 == u2);
}

TEST_CASE("Buffer: Store", "[buffer]") {
    const std::string s1 = "string";
    const int i1 = 42;
    const double d1 = 123.456;
    const unsigned long long u1 = 123456789;

    ghoul::Buffer b;
    b.serialize(s1);
    b.serialize(i1);
    b.serialize(d1);
    b.serialize(u1);
    b.write("binary.bin");

    ghoul::Buffer b2;
    b2.read("binary.bin");

    std::string s2;
    b2.deserialize(s2);
    int i2;
    b2.deserialize(i2);
    double d2;
    b2.deserialize(d2);
    unsigned long long u2;
    b2.deserialize(u2);

    REQUIRE(s1 == s2);
    REQUIRE(i1 == i2);
    REQUIRE(d1 == d2);
    REQUIRE(u1 == u2);
}

TEST_CASE("Buffer: Store Compress", "[buffer]") {
    const std::string s1 = "string";
    const int i1 = 42;
    const double d1 = 123.456;
    const unsigned long long u1 = 123456789;

    ghoul::Buffer b;
    b.serialize(s1);
    b.serialize(i1);
    b.serialize(d1);
    b.serialize(u1);
    b.write("binary.bin", ghoul::Buffer::Compress::Yes);

    ghoul::Buffer b2;
    b2.read("binary.bin");

    std::string s2;
    b2.deserialize(s2);
    int i2;
    b2.deserialize(i2);
    double d2;
    b2.deserialize(d2);
    unsigned long long u2;
    b2.deserialize(u2);

    REQUIRE(s1 == s2);
    REQUIRE(i1 == i2);
    REQUIRE(d1 == d2);
    REQUIRE(u1 == u2);
}

TEST_CASE("Buffer: Capacity", "[buffer]") {
    const std::string s1 = "string";
    const int i1 = 42;
    const double d1 = 123.456;
    const unsigned long long u1 = 123456789;

    ghoul::Buffer b(0);
    b.serialize(s1);
    b.serialize(i1);
    b.serialize(d1);
    b.serialize(u1);

    std::string s2;
    b.deserialize(s2);
    int i2;
    b.deserialize(i2);
    double d2;
    b.deserialize(d2);
    unsigned long long u2;
    b.deserialize(u2);

    REQUIRE(s1 == s2);
    REQUIRE(i1 == i2);
    REQUIRE(d1 == d2);
    REQUIRE(u1 == u2);
}

TEST_CASE("Buffer: Vector", "[buffer]") {
    std::vector<float> fv;
    fv.push_back(1.5);
    fv.push_back(2.5);
    fv.push_back(3.5);
    fv.push_back(4.5);
    fv.push_back(5.5);

    std::vector<std::string> sv;
    sv.push_back("first");
    sv.push_back("second");
    sv.push_back("third");

    ghoul::Buffer b;
    b.serialize(fv);
    b.serialize(sv);
    std::vector<float> fv2;
    b.deserialize(fv2);
    std::vector<std::string> sv2;
    b.deserialize(sv2);

    REQUIRE(fv == fv2);
    REQUIRE(sv == sv2);
}
