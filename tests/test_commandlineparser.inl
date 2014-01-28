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

#include <ghoul/cmdparser/cmdparser>

/*
Test checklist:
--- SingleCommand, MultipleCommand
---     1-4 arguments
---     different types
---     same types
---     invalid types
---     calling once
---     calling multiple times
--- Multiple commands in the same command-line result
--- Variable orders should produce the same result
--- Unknown commands
--- Collection of unknown commands with known cmomands interspersed
--- Error messages when unknown commands are allowed but no receiving vector is provided
    (and vice versa)
*/

class CommandlineParserTest : public testing::Test {
protected:
    CommandlineParserTest() {
        _p = new ghoul::cmdparser::CommandlineParser;
    }

    ~CommandlineParserTest() {
    }

    void resetParser() {
        delete _p;
        _p = new ghoul::cmdparser::CommandlineParser;
    }

    ghoul::cmdparser::CommandlineParser* _p;
};

TEST_F(CommandlineParserTest, SingleZeroCommandArguments) {
    bool v = false;
    ghoul::cmdparser::SingleCommandZeroArguments* c = 
        new ghoul::cmdparser::SingleCommandZeroArguments(&v, "-zero");
    _p->addCommand(c);

    char* argv[] = {
        "tests",
        "-zero"
    };

    _p->setCommandLine(2, argv);
    _p->execute();
    EXPECT_EQ(true, v);
}

TEST_F(CommandlineParserTest, SingleCommandOneArgumentBool) {
    // boolean
    bool v = true;
    ghoul::cmdparser::SingleCommand<bool>* c =
        new ghoul::cmdparser::SingleCommand<bool>(&v, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ(false, v) << "0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ(true, v) << "1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandOneArgumentInt) {
    // int
    int v = 0;
    ghoul::cmdparser::SingleCommand<int>* c =
        new ghoul::cmdparser::SingleCommand<int>(&v, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "1"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ(1, v) << "1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ(0, v) << "0";
    }
}

TEST_F(CommandlineParserTest, SingleCommandOneArgumentString) {
    // string
    std::string v = "";
    ghoul::cmdparser::SingleCommand<std::string>* c =
        new ghoul::cmdparser::SingleCommand<std::string>(&v, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "foo"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ("foo", v) << "foo";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "bar"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ("bar", v) << "bar";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsBoolBool) {
    // bool-bool
    bool v1 = true;
    bool v2 = true;
    ghoul::cmdparser::SingleCommand<bool, bool>* c =
        new ghoul::cmdparser::SingleCommand<bool, bool>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 0";
        ASSERT_EQ(false, v2) << "0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 0";
        ASSERT_EQ(false, v2) << "1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 1";
        ASSERT_EQ(true, v2) << "0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 1";
        ASSERT_EQ(true, v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsIntInt) {
    // int-int
    int v1 = 1;
    int v2 = 1;
    ghoul::cmdparser::SingleCommand<int, int>* c =
        new ghoul::cmdparser::SingleCommand<int, int>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1) << "0 0";
        ASSERT_EQ(0, v2) << "0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1) << "1 0";
        ASSERT_EQ(0, v2) << "1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1) << "0 1";
        ASSERT_EQ(1, v2) << "0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1) << "1 1";
        ASSERT_EQ(1, v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsStringString) {
    // int-int
    std::string v1 = "";
    std::string v2 = "";
    ghoul::cmdparser::SingleCommand<std::string, std::string>* c =
        new ghoul::cmdparser::SingleCommand<std::string, std::string>(&v1, &v2, 
        "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("0", v1) << "0 0";
        ASSERT_EQ("0", v2) << "0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("1", v1) << "1 0";
        ASSERT_EQ("0", v2) << "1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("0", v1) << "0 1";
        ASSERT_EQ("1", v2) << "0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("1", v1) << "1 1";
        ASSERT_EQ("1", v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsBoolInt) {
    // bool-int
    bool v1 = true;
    int v2 = 1;
    ghoul::cmdparser::SingleCommand<bool, int>* c =
        new ghoul::cmdparser::SingleCommand<bool, int>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 0";
        ASSERT_EQ(0, v2) << "0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 0";
        ASSERT_EQ(0, v2) << "1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 1";
        ASSERT_EQ(1, v2) << "0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 1";
        ASSERT_EQ(1, v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsIntBool) {
    // bool-int
    int v1 = 1;
    bool v2 = true;
    ghoul::cmdparser::SingleCommand<int, bool>* c =
        new ghoul::cmdparser::SingleCommand<int, bool>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1) << "0 0";
        ASSERT_EQ(false, v2) << "0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1) << "1 0";
        ASSERT_EQ(false, v2) << "1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1) << "0 1";
        ASSERT_EQ(true, v2) << "0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1) << "1 1";
        ASSERT_EQ(true, v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsIntString) {
    // bool-int
    int v1 = 1;
    std::string v2 = "";
    ghoul::cmdparser::SingleCommand<int, std::string>* c =
        new ghoul::cmdparser::SingleCommand<int, std::string>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1) << "0 0";
        ASSERT_EQ("0", v2) << "0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1) << "1 0";
        ASSERT_EQ("0", v2) << "1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1) << "0 1";
        ASSERT_EQ("1", v2) << "0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1) << "1 1";
        ASSERT_EQ("1", v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsStringInt) {
    // bool-int
    std::string v1 = "";
    int v2 = 1;
    ghoul::cmdparser::SingleCommand<std::string, int>* c =
        new ghoul::cmdparser::SingleCommand<std::string, int>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("0", v1) << "0 0";
        ASSERT_EQ(0, v2) << "0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("1", v1) << "1 0";
        ASSERT_EQ(0, v2) << "1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("0", v1) << "0 1";
        ASSERT_EQ(1, v2) << "0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("1", v1) << "1 1";
        ASSERT_EQ(1, v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandThreeArgumentsBoolIntString) {
    bool v1 = true;
    int v2 = 1;
    std::string v3 = "";
    ghoul::cmdparser::SingleCommand<bool, int, std::string>* c =
        new ghoul::cmdparser::SingleCommand<bool, int, std::string>(
                                                        &v1, &v2, &v3, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 0 0";
        ASSERT_EQ(0, v2) << "0 0 0";
        ASSERT_EQ("0", v3) << "0 0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 0 0";
        ASSERT_EQ(0, v2) << "1 0 0";
        ASSERT_EQ("0", v3) << "1 0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "0"

        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 1 0";
        ASSERT_EQ(1, v2) << "0 1 0";
        ASSERT_EQ("0", v3) << "0 1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "0"

        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 1 0";
        ASSERT_EQ(1, v2) << "1 1 0";
        ASSERT_EQ("0", v3) << "1 1 0";
    }

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 0 1";
        ASSERT_EQ(0, v2) << "0 0 1";
        ASSERT_EQ("1", v3) << "0 0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 0 1";
        ASSERT_EQ(0, v2) << "1 0 1";
        ASSERT_EQ("1", v3) << "1 0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "1"

        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 1 1";
        ASSERT_EQ(1, v2) << "0 1 1";
        ASSERT_EQ("1", v3) << "0 1 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "1"

        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 1 1";
        ASSERT_EQ(1, v2) << "1 1 1";
        ASSERT_EQ("1", v3) << "1 1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandFourArgumentsBoolIntStringFloat) {
    bool v1 = true;
    int v2 = 1;
    std::string v3 = "";
    float v4 = 1.f;
    ghoul::cmdparser::SingleCommand<bool, int, std::string, float>* c =
        new ghoul::cmdparser::SingleCommand<bool, int, std::string, float>(
        &v1, &v2, &v3, &v4, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 0 0 0";
        ASSERT_EQ(0, v2) << "0 0 0 0";
        ASSERT_EQ("0", v3) << "0 0 0 0";
        ASSERT_EQ(0.f, v4) << "0 0 0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 0 0 0";
        ASSERT_EQ(0, v2) << "1 0 0 0";
        ASSERT_EQ("0", v3) << "1 0 0 0";
        ASSERT_EQ(0.f, v4) << "1 0 0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 1 0 0";
        ASSERT_EQ(1, v2) << "0 1 0 0";
        ASSERT_EQ("0", v3) << "0 1 0 0";
        ASSERT_EQ(0.f, v4) << "0 1 0 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 1 0 0";
        ASSERT_EQ(1, v2) << "1 1 0 0";
        ASSERT_EQ("0", v3) << "1 1 0 0";
        ASSERT_EQ(0.f, v4) << "1 1 0 0";
    }

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 0 1 0";
        ASSERT_EQ(0, v2) << "0 0 1 0";
        ASSERT_EQ("1", v3) << "0 0 1 0";
        ASSERT_EQ(0.f, v4) << "0 0 1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 0 1 0";
        ASSERT_EQ(0, v2) << "1 0 1 0";
        ASSERT_EQ("1", v3) << "1 0 1 0";
        ASSERT_EQ(0.f, v4) << "1 0 1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 1 1 0";
        ASSERT_EQ(1, v2) << "0 1 1 0";
        ASSERT_EQ("1", v3) << "0 1 1 0";
        ASSERT_EQ(0.f, v4) << "0 1 1 0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 1 1 0";
        ASSERT_EQ(1, v2) << "1 1 1 0";
        ASSERT_EQ("1", v3) << "1 1 1 0";
        ASSERT_EQ(0.f, v4) << "1 1 1 0";
    }

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 0 0 1";
        ASSERT_EQ(0, v2) << "0 0 0 1";
        ASSERT_EQ("0", v3) << "0 0 0 1";
        ASSERT_EQ(1.f, v4) << "0 0 0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 0 0 1";
        ASSERT_EQ(0, v2) << "1 0 0 1";
        ASSERT_EQ("0", v3) << "1 0 0 1";
        ASSERT_EQ(1.f, v4) << "1 0 0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 1 0 1";
        ASSERT_EQ(1, v2) << "0 1 0 1";
        ASSERT_EQ("0", v3) << "0 1 0 1";
        ASSERT_EQ(1.f, v4) << "0 1 0 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 1 0 1";
        ASSERT_EQ(1, v2) << "1 1 0 1";
        ASSERT_EQ("0", v3) << "1 1 0 1";
        ASSERT_EQ(1.f, v4) << "1 1 0 1";
    }

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 0 1 1";
        ASSERT_EQ(0, v2) << "0 0 1 1";
        ASSERT_EQ("1", v3) << "0 0 1 1";
        ASSERT_EQ(1.f, v4) << "0 0 1 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 0 1 1";
        ASSERT_EQ(0, v2) << "1 0 1 1";
        ASSERT_EQ("1", v3) << "1 0 1 1";
        ASSERT_EQ(1.f, v4) << "1 0 1 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1) << "0 1 1 1";
        ASSERT_EQ(1, v2) << "0 1 1 1";
        ASSERT_EQ("1", v3) << "0 1 1 1";
        ASSERT_EQ(1.f, v4) << "0 1 1 1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1) << "1 1 1 1";
        ASSERT_EQ(1, v2) << "1 1 1 1";
        ASSERT_EQ("1", v3) << "1 1 1 1";
        ASSERT_EQ(1.f, v4) << "1 1 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleZeroCommandArguments) {
    int v = 0;
    ghoul::cmdparser::MultipleCommandZeroArguments* c = 
        new ghoul::cmdparser::MultipleCommandZeroArguments(&v, "-zero");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-zero",
            "-zero"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        EXPECT_EQ(2, v) << "2x zero";
    }

    v = 0;
    {
        char* argv[] = {
            "tests",
            "-zero",
            "-zero",
            "-zero",
            "-zero",
            "-zero",
            "-zero"
        };

        _p->setCommandLine(7, argv);
        _p->execute();
        EXPECT_EQ(6, v) << "6x zero";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandOneArgumentBool) {
    // boolean
    std::vector<bool> v;
    ghoul::cmdparser::MultipleCommand<bool>* c =
        new ghoul::cmdparser::MultipleCommand<bool>(&v, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ(1, v.size()) << "0";
        ASSERT_EQ(false, v[0]) << "0";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ(1, v.size()) << "1";
        ASSERT_EQ(true, v[0]) << "1";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "-single",
            "1"
        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(2, v.size()) << "0 1";
        ASSERT_EQ(false, v[0]) << "0 1";
        ASSERT_EQ(true, v[1]) << "0 1";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "-single",
            "0"
        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(2, v.size()) << "1 1";
        ASSERT_EQ(true, v[0]) << "1 1";
        ASSERT_EQ(false, v[1]) << "1 1";
    }

}
/*
TEST_F(CommandlineParserTest, MultipleCommandOneArgumentInt) {
    // int
    int v = 0;
    ghoul::cmdparser::MultipleCommand<int>* c =
        new ghoul::cmdparser::MultipleCommand<int>(&v, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "1"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ(1, v);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ(0, v);
    }
}

TEST_F(CommandlineParserTest, MultipleCommandOneArgumentString) {
    // string
    std::string v = "";
    ghoul::cmdparser::MultipleCommand<std::string>* c =
        new ghoul::cmdparser::MultipleCommand<std::string>(&v, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "foo"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ("foo", v);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "bar"
        };

        _p->setCommandLine(3, argv);
        _p->execute();
        ASSERT_EQ("bar", v);
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsBoolBool) {
    // bool-bool
    bool v1 = true;
    bool v2 = true;
    ghoul::cmdparser::MultipleCommand<bool, bool>* c =
        new ghoul::cmdparser::MultipleCommand<bool, bool>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(false, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(false, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(true, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(true, v2);
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsIntInt) {
    // int-int
    int v1 = 1;
    int v2 = 1;
    ghoul::cmdparser::MultipleCommand<int, int>* c =
        new ghoul::cmdparser::MultipleCommand<int, int>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1);
        ASSERT_EQ(0, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1);
        ASSERT_EQ(0, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1);
        ASSERT_EQ(1, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1);
        ASSERT_EQ(1, v2);
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsStringString) {
    // int-int
    std::string v1 = "";
    std::string v2 = "";
    ghoul::cmdparser::MultipleCommand<std::string, std::string>* c =
        new ghoul::cmdparser::MultipleCommand<std::string, std::string>(&v1, &v2, 
        "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("0", v1);
        ASSERT_EQ("0", v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("1", v1);
        ASSERT_EQ("0", v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("0", v1);
        ASSERT_EQ("1", v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("1", v1);
        ASSERT_EQ("1", v2);
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsBoolInt) {
    // bool-int
    bool v1 = true;
    int v2 = 1;
    ghoul::cmdparser::MultipleCommand<bool, int>* c =
        new ghoul::cmdparser::MultipleCommand<bool, int>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(0, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(0, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(1, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(1, v2);
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsIntBool) {
    // bool-int
    int v1 = 1;
    bool v2 = true;
    ghoul::cmdparser::MultipleCommand<int, bool>* c =
        new ghoul::cmdparser::MultipleCommand<int, bool>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1);
        ASSERT_EQ(false, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1);
        ASSERT_EQ(false, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1);
        ASSERT_EQ(true, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1);
        ASSERT_EQ(true, v2);
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsIntString) {
    // bool-int
    int v1 = 1;
    std::string v2 = "";
    ghoul::cmdparser::MultipleCommand<int, std::string>* c =
        new ghoul::cmdparser::MultipleCommand<int, std::string>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1);
        ASSERT_EQ("0", v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1);
        ASSERT_EQ("0", v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(0, v1);
        ASSERT_EQ("1", v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ(1, v1);
        ASSERT_EQ("1", v2);
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsStringInt) {
    // bool-int
    std::string v1 = "";
    int v2 = 1;
    ghoul::cmdparser::MultipleCommand<std::string, int>* c =
        new ghoul::cmdparser::MultipleCommand<std::string, int>(&v1, &v2, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("0", v1);
        ASSERT_EQ(0, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("1", v1);
        ASSERT_EQ(0, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("0", v1);
        ASSERT_EQ(1, v2);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        _p->execute();
        ASSERT_EQ("1", v1);
        ASSERT_EQ(1, v2);
    }
}

TEST_F(CommandlineParserTest, MultipleCommandThreeArgumentsBoolIntString) {
    bool v1 = true;
    int v2 = 1;
    std::string v3 = "";
    ghoul::cmdparser::MultipleCommand<bool, int, std::string>* c =
        new ghoul::cmdparser::MultipleCommand<bool, int, std::string>(
        &v1, &v2, &v3, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("0", v3);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("0", v3);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "0"

        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("0", v3);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "0"

        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("0", v3);
    }

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("1", v3);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("1", v3);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "1"

        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("1", v3);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "1"

        };

        _p->setCommandLine(5, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("1", v3);
    }
}

TEST_F(CommandlineParserTest, MultipleCommandFourArgumentsBoolIntStringFloat) {
    bool v1 = true;
    int v2 = 1;
    std::string v3 = "";
    float v4 = 1.f;
    ghoul::cmdparser::MultipleCommand<bool, int, std::string, float>* c =
        new ghoul::cmdparser::MultipleCommand<bool, int, std::string, float>(
        &v1, &v2, &v3, &v4, "-single");
    _p->addCommand(c);

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("0", v3);
        ASSERT_EQ(0.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("0", v3);
        ASSERT_EQ(0.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("0", v3);
        ASSERT_EQ(0.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("0", v3);
        ASSERT_EQ(0.f, v4);
    }

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("1", v3);
        ASSERT_EQ(0.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("1", v3);
        ASSERT_EQ(0.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("1", v3);
        ASSERT_EQ(0.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "0"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("1", v3);
        ASSERT_EQ(0.f, v4);
    }

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("0", v3);
        ASSERT_EQ(1.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("0", v3);
        ASSERT_EQ(1.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("0", v3);
        ASSERT_EQ(1.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("0", v3);
        ASSERT_EQ(1.f, v4);
    }

    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("1", v3);
        ASSERT_EQ(1.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(0, v2);
        ASSERT_EQ("1", v3);
        ASSERT_EQ(1.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(false, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("1", v3);
        ASSERT_EQ(1.f, v4);
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(6, argv);
        _p->execute();
        ASSERT_EQ(true, v1);
        ASSERT_EQ(1, v2);
        ASSERT_EQ("1", v3);
        ASSERT_EQ(1.f, v4);
    }


}*/