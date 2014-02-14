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

#include "gtest/gtest.h"
#include <ghoul/misc/dictionary.h>
#include <glm/glm.hpp>

/*
Test checklist:
--- getValue
---  basic types
---  advanced types
--- setValues
---  basic types
---  advanced types
--- nested dictionaries
--- timing
*/

class ConfigurationManagerTest : public testing::Test {
protected:
    ConfigurationManagerTest() {
        _d = new ghoul::Dictionary;
    }

    ~ConfigurationManagerTest() {
        if (_d) {
            delete _d;
            _d = nullptr;
        }
    }

    void resetManager() {
        delete _d;
        _d = new ghoul::Dictionary;
    }

    ghoul::Dictionary* _d;
};

TEST_F(ConfigurationManagerTest, EmptyTest) {
    EXPECT_EQ(0, _d->size());
}

TEST_F(ConfigurationManagerTest, BasicTypes) {
    _d->setValue("")
}