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

#include <ghoul/cmdparser/commandlineparser.h>
#include <ghoul/cmdparser/singlecommand.h>
#include <ghoul/cmdparser/multiplecommand.h>

#include <memory>

 /*
  * Test checklist:
  * +++ SingleCommand, MultipleCommand
  * +++     1-4 arguments
  * +++     different types
  * +++     same types
  * +++     calling once
  * +++     calling multiple times
  * +++ Multiple commands in the same command-line result
  * +++ Variable orders should produce the same result
  * +++ Unknown commands
  * +++ Collection of unknown commands with known commands interspersed
  * +++ Error messages when unknown commands are allowed but no receiving vector is
  *     provided (and vice versa)
  */

TEST_CASE("Unknown Commands Unhandled", "[commandlineparser]") {
    ghoul::cmdparser::CommandlineParser p;

    std::vector<std::string> argv = {
        "tests",
        "-cmd1",
        "arg",
        "-cmd2",
        "arg2"
    };

    p.setCommandLine(argv);
    REQUIRE_THROWS_AS(
        p.execute(),
        ghoul::cmdparser::CommandlineParser::CommandlineException
    );
}

TEST_CASE("Unknown Commands Handled Correctly", "[commandlineparser]") {
    ghoul::cmdparser::CommandlineParser p;

    std::vector<std::string> argv = {
        "tests",
        "-cmd1",
        "arg",
        "-cmd2",
        "arg2"
    };

    p.setAllowUnknownCommands(
        ghoul::cmdparser::CommandlineParser::AllowUnknownCommands::Yes
    );
    p.setCommandLine(argv);
    REQUIRE_NOTHROW(p.execute());
}

TEST_CASE("Unknown Commands Interspersed", "[commandlineparser]") {
    ghoul::cmdparser::CommandlineParser p;

    std::vector<std::string> argv = {
        "tests",
        "-cmd1",
        "arg",
        "-cmd2",
        "arg2",
        "arg3",
        "-cmd3",
        "arg4"
    };
    std::string v1;
    std::string v2;
    using T = ghoul::cmdparser::SingleCommand<std::string, std::string>;
    p.addCommand(std::make_unique<T>(v1, v2, "-cmd2"));

    using ghoul::cmdparser::CommandlineParser;
    p.setAllowUnknownCommands(CommandlineParser::AllowUnknownCommands::Yes);
    const std::vector<std::string>& arguments = p.setCommandLine(argv);

    REQUIRE_NOTHROW(p.execute());

    REQUIRE(arguments.size() == 4);
    REQUIRE(arguments.at(0) == "-cmd1");
    REQUIRE(arguments.at(1) == "arg");
    REQUIRE(arguments.at(2) == "-cmd3");
    REQUIRE(arguments.at(3) == "arg4");
    REQUIRE(v1 == "arg2");
    REQUIRE(v2 == "arg3");
}

TEST_CASE("Single Zero Command Arguments", "[commandlineparser]") {
    ghoul::cmdparser::CommandlineParser p;

    bool v = false;
    using T = ghoul::cmdparser::SingleCommandZeroArguments;
    p.addCommand(std::make_unique<T>(v, "-zero"));

    std::vector<std::string> argv = {
        "tests",
        "-zero"
    };

    p.setCommandLine(argv);
    REQUIRE_NOTHROW(p.execute());
    REQUIRE(v);
}

TEST_CASE("Single Command One Argument Bool", "[commandlineparser]") {
    ghoul::cmdparser::CommandlineParser p;

    // boolean
    bool v = true;
    using T = ghoul::cmdparser::SingleCommand<bool>;
    p.addCommand(std::make_unique<T>(v, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0"
        };

        p.setCommandLine(argv);
        REQUIRE_NOTHROW(p.execute());
        REQUIRE_FALSE(v);
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1"
        };

        p.setCommandLine(argv);
        REQUIRE_NOTHROW(p.execute());
        REQUIRE(v);
    }
}

TEST_CASE("Single Command Called Multiple Times", "[commandlineparser]") {
    ghoul::cmdparser::CommandlineParser p;

    // boolean
    bool v = false;
    using T = ghoul::cmdparser::SingleCommand<bool>;
    p.addCommand(std::make_unique<T>(v, "-single"));

    std::vector<std::string> argv = {
        "tests",
        "-single",
        "0",
        "-single",
        "0"
    };

    p.setCommandLine(argv);
    REQUIRE_THROWS_AS(
        p.execute(),
        ghoul::cmdparser::CommandlineParser::CommandlineException
    );
}

TEST_CASE("Multiple Commands Permutation", "[commandlineparser]") {
    ghoul::cmdparser::CommandlineParser p;

    int v1 = 0;
    int v2 = 0;
    int v3 = 0;

    using T = ghoul::cmdparser::SingleCommand<int>;

    p.addCommand(std::make_unique<T>(v1, "-cmd1"));
    p.addCommand(std::make_unique<T>(v2, "-cmd2"));
    p.addCommand(std::make_unique<T>(v3, "-cmd3"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-cmd1",
            "1",
            "-cmd2",
            "2",
            "-cmd3",
            "3"
        };

        p.setCommandLine(argv);
        REQUIRE_NOTHROW(p.execute());
        REQUIRE(v1 == 1);
        REQUIRE(v2 == 2);
        REQUIRE(v3 == 3);
    }

    {
        v1 = 0;
        v2 = 0;
        v3 = 0;

        std::vector<std::string> argv = {
            "tests",
            "-cmd2",
            "2",
            "-cmd1",
            "1",
            "-cmd3",
            "3"
        };

        p.setCommandLine(argv);
        REQUIRE_NOTHROW(p.execute());
        REQUIRE(v1 == 1);
        REQUIRE(v2 == 2);
        REQUIRE(v3 == 3);
    }

    {
        v1 = 0;
        v2 = 0;
        v3 = 0;

        std::vector<std::string> argv = {
            "tests",
            "-cmd3",
            "3",
            "-cmd2",
            "2",
            "-cmd1",
            "1"
        };

        p.setCommandLine(argv);
        REQUIRE_NOTHROW(p.execute());
        REQUIRE(v1 == 1);
        REQUIRE(v2 == 2);
        REQUIRE(v3 == 3);
    }

    {
        v1 = 0;
        v2 = 0;
        v3 = 0;

        std::vector<std::string> argv = {
            "tests",
            "-cmd3",
            "3",
            "-cmd1",
            "1",
            "-cmd2",
            "2"
        };

        p.setCommandLine(argv);
        REQUIRE_NOTHROW(p.execute());
        REQUIRE(v1 == 1);
        REQUIRE(v2 == 2);
        REQUIRE(v3 == 3);
    }
}

TEST_CASE("", "[commandlineparser]") {
    ghoul::cmdparser::CommandlineParser p;
}

TEST_CASE("", "[commandlineparser]") {
    ghoul::cmdparser::CommandlineParser p;
}


#if 0

TEST_F(CommandlineParserTest, SingleCommandOneArgumentInt) {
    // int
    int v = 0;
    using T = ghoul::cmdparser::SingleCommand<int>;
    _p->addCommand(std::make_unique<T>(v, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v) << "1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(0, v) << "0";
    }
}

TEST_F(CommandlineParserTest, SingleCommandOneArgumentString) {
    // string
    std::string v = "";
    using T = ghoul::cmdparser::SingleCommand<std::string>;
    _p->addCommand(std::make_unique<T>(v, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "foo"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ("foo", v) << "foo";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "bar"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ("bar", v) << "bar";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsBoolBool) {
    // bool-bool
    bool v1 = true;
    bool v2 = true;
    using T = ghoul::cmdparser::SingleCommand<bool, bool>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 0";
        EXPECT_EQ(false, v2) << "0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 0";
        EXPECT_EQ(false, v2) << "1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 1";
        EXPECT_EQ(true, v2) << "0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 1";
        EXPECT_EQ(true, v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsIntInt) {
    // int-int
    int v1 = 1;
    int v2 = 1;
    using T = ghoul::cmdparser::SingleCommand<int, int>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(0, v1) << "0 0";
        EXPECT_EQ(0, v2) << "0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1) << "1 0";
        EXPECT_EQ(0, v2) << "1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(0, v1) << "0 1";
        EXPECT_EQ(1, v2) << "0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1) << "1 1";
        EXPECT_EQ(1, v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsStringString) {
    // int-int
    std::string v1 = "";
    std::string v2 = "";
    using T = ghoul::cmdparser::SingleCommand<std::string, std::string>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ("0", v1) << "0 0";
        EXPECT_EQ("0", v2) << "0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ("1", v1) << "1 0";
        EXPECT_EQ("0", v2) << "1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ("0", v1) << "0 1";
        EXPECT_EQ("1", v2) << "0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ("1", v1) << "1 1";
        EXPECT_EQ("1", v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsBoolInt) {
    // bool-int
    bool v1 = true;
    int v2 = 1;
    using T = ghoul::cmdparser::SingleCommand<bool, int>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 0";
        EXPECT_EQ(0, v2) << "0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 0";
        EXPECT_EQ(0, v2) << "1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 1";
        EXPECT_EQ(1, v2) << "0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 1";
        EXPECT_EQ(1, v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsIntBool) {
    // bool-int
    int v1 = 1;
    bool v2 = true;
    using T = ghoul::cmdparser::SingleCommand<int, bool>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(0, v1) << "0 0";
        EXPECT_EQ(false, v2) << "0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1) << "1 0";
        EXPECT_EQ(false, v2) << "1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(0, v1) << "0 1";
        EXPECT_EQ(true, v2) << "0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1) << "1 1";
        EXPECT_EQ(true, v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsIntString) {
    // bool-int
    int v1 = 1;
    std::string v2 = "";
    using T = ghoul::cmdparser::SingleCommand<int, std::string>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(0, v1) << "0 0";
        EXPECT_EQ("0", v2) << "0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1) << "1 0";
        EXPECT_EQ("0", v2) << "1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(0, v1) << "0 1";
        EXPECT_EQ("1", v2) << "0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1) << "1 1";
        EXPECT_EQ("1", v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandTwoArgumentsStringInt) {
    // bool-int
    std::string v1 = "";
    int v2 = 1;
    using T = ghoul::cmdparser::SingleCommand<std::string, int>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ("0", v1) << "0 0";
        EXPECT_EQ(0, v2) << "0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ("1", v1) << "1 0";
        EXPECT_EQ(0, v2) << "1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ("0", v1) << "0 1";
        EXPECT_EQ(1, v2) << "0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ("1", v1) << "1 1";
        EXPECT_EQ(1, v2) << "1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandThreeArgumentsBoolIntString) {
    bool v1 = true;
    int v2 = 1;
    std::string v3 = "";
    using T = ghoul::cmdparser::SingleCommand<bool, int, std::string>;
    _p->addCommand(std::make_unique<T>(v1, v2, v3, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 0 0";
        EXPECT_EQ(0, v2) << "0 0 0";
        EXPECT_EQ("0", v3) << "0 0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 0 0";
        EXPECT_EQ(0, v2) << "1 0 0";
        EXPECT_EQ("0", v3) << "1 0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 1 0";
        EXPECT_EQ(1, v2) << "0 1 0";
        EXPECT_EQ("0", v3) << "0 1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 1 0";
        EXPECT_EQ(1, v2) << "1 1 0";
        EXPECT_EQ("0", v3) << "1 1 0";
    }

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 0 1";
        EXPECT_EQ(0, v2) << "0 0 1";
        EXPECT_EQ("1", v3) << "0 0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 0 1";
        EXPECT_EQ(0, v2) << "1 0 1";
        EXPECT_EQ("1", v3) << "1 0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 1 1";
        EXPECT_EQ(1, v2) << "0 1 1";
        EXPECT_EQ("1", v3) << "0 1 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 1 1";
        EXPECT_EQ(1, v2) << "1 1 1";
        EXPECT_EQ("1", v3) << "1 1 1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandFourArgumentsBoolIntStringFloat) {
    bool v1 = true;
    int v2 = 1;
    std::string v3 = "";
    float v4 = 1.f;

    using T = ghoul::cmdparser::SingleCommand<bool, int, std::string, float>;
    _p->addCommand(std::make_unique<T>(v1, v2, v3, v4, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 0 0 0";
        EXPECT_EQ(0, v2) << "0 0 0 0";
        EXPECT_EQ("0", v3) << "0 0 0 0";
        EXPECT_EQ(0.f, v4) << "0 0 0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 0 0 0";
        EXPECT_EQ(0, v2) << "1 0 0 0";
        EXPECT_EQ("0", v3) << "1 0 0 0";
        EXPECT_EQ(0.f, v4) << "1 0 0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 1 0 0";
        EXPECT_EQ(1, v2) << "0 1 0 0";
        EXPECT_EQ("0", v3) << "0 1 0 0";
        EXPECT_EQ(0.f, v4) << "0 1 0 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 1 0 0";
        EXPECT_EQ(1, v2) << "1 1 0 0";
        EXPECT_EQ("0", v3) << "1 1 0 0";
        EXPECT_EQ(0.f, v4) << "1 1 0 0";
    }

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 0 1 0";
        EXPECT_EQ(0, v2) << "0 0 1 0";
        EXPECT_EQ("1", v3) << "0 0 1 0";
        EXPECT_EQ(0.f, v4) << "0 0 1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 0 1 0";
        EXPECT_EQ(0, v2) << "1 0 1 0";
        EXPECT_EQ("1", v3) << "1 0 1 0";
        EXPECT_EQ(0.f, v4) << "1 0 1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 1 1 0";
        EXPECT_EQ(1, v2) << "0 1 1 0";
        EXPECT_EQ("1", v3) << "0 1 1 0";
        EXPECT_EQ(0.f, v4) << "0 1 1 0";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 1 1 0";
        EXPECT_EQ(1, v2) << "1 1 1 0";
        EXPECT_EQ("1", v3) << "1 1 1 0";
        EXPECT_EQ(0.f, v4) << "1 1 1 0";
    }

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 0 0 1";
        EXPECT_EQ(0, v2) << "0 0 0 1";
        EXPECT_EQ("0", v3) << "0 0 0 1";
        EXPECT_EQ(1.f, v4) << "0 0 0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 0 0 1";
        EXPECT_EQ(0, v2) << "1 0 0 1";
        EXPECT_EQ("0", v3) << "1 0 0 1";
        EXPECT_EQ(1.f, v4) << "1 0 0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 1 0 1";
        EXPECT_EQ(1, v2) << "0 1 0 1";
        EXPECT_EQ("0", v3) << "0 1 0 1";
        EXPECT_EQ(1.f, v4) << "0 1 0 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 1 0 1";
        EXPECT_EQ(1, v2) << "1 1 0 1";
        EXPECT_EQ("0", v3) << "1 1 0 1";
        EXPECT_EQ(1.f, v4) << "1 1 0 1";
    }

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 0 1 1";
        EXPECT_EQ(0, v2) << "0 0 1 1";
        EXPECT_EQ("1", v3) << "0 0 1 1";
        EXPECT_EQ(1.f, v4) << "0 0 1 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 0 1 1";
        EXPECT_EQ(0, v2) << "1 0 1 1";
        EXPECT_EQ("1", v3) << "1 0 1 1";
        EXPECT_EQ(1.f, v4) << "1 0 1 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(false, v1) << "0 1 1 1";
        EXPECT_EQ(1, v2) << "0 1 1 1";
        EXPECT_EQ("1", v3) << "0 1 1 1";
        EXPECT_EQ(1.f, v4) << "0 1 1 1";
    }
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(true, v1) << "1 1 1 1";
        EXPECT_EQ(1, v2) << "1 1 1 1";
        EXPECT_EQ("1", v3) << "1 1 1 1";
        EXPECT_EQ(1.f, v4) << "1 1 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleZeroCommandArguments) {
    int v = 0;
    using T = ghoul::cmdparser::MultipleCommandZeroArguments;
    _p->addCommand(std::make_unique<T>(v, "-zero"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-zero",
            "-zero"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v) << "2x zero";
    }

    v = 0;
    {
        std::vector<std::string> argv = {
            "tests",
            "-zero",
            "-zero",
            "-zero",
            "-zero",
            "-zero",
            "-zero"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(6, v) << "6x zero";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandOneArgumentBool) {
    std::vector<bool> v;
    using T = ghoul::cmdparser::MultipleCommand<bool>;
    _p->addCommand(std::make_unique<T>(v, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v.size()) << "0";
        EXPECT_EQ(false, v[0]) << "0";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v.size()) << "1";
        EXPECT_EQ(true, v[0]) << "1";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "-single",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "0 0";
        EXPECT_EQ(false, v[0]) << "0 0";
        EXPECT_EQ(false, v[1]) << "0 0";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "-single",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "1 0";
        EXPECT_EQ(true, v[0]) << "1 0";
        EXPECT_EQ(false, v[1]) << "1 0";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "-single",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "0 1";
        EXPECT_EQ(false, v[0]) << "0 1";
        EXPECT_EQ(true, v[1]) << "0 1";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "-single",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "1 1";
        EXPECT_EQ(true, v[0]) << "1 1";
        EXPECT_EQ(false, v[1]) << "1 1";
    }

}

TEST_F(CommandlineParserTest, MultipleCommandOneArgumentInt) {
    std::vector<int> v;
    using T = ghoul::cmdparser::MultipleCommand<int>;
    _p->addCommand(std::make_unique<T>(v, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v.size()) << "1";
        EXPECT_EQ(1, v[0]) << "1";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v.size()) << "0";
        EXPECT_EQ(0, v[0]) << "0";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "-single",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "1 0";
        EXPECT_EQ(1, v[0]) << "1 0";
        EXPECT_EQ(0, v[1]) << "1 0";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "-single",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "0 0";
        EXPECT_EQ(0, v[0]) << "0 0";
        EXPECT_EQ(0, v[1]) << "0 0";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "-single",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "1 1";
        EXPECT_EQ(1, v[0]) << "1 1";
        EXPECT_EQ(1, v[1]) << "1 1";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "-single",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "0 1";
        EXPECT_EQ(0, v[0]) << "0 1";
        EXPECT_EQ(2, v.size()) << "0 1";
        EXPECT_EQ(1, v[1]) << "0 1";
    }
}


TEST_F(CommandlineParserTest, MultipleCommandOneArgumentString) {
    std::vector<std::string> v;
    using T = ghoul::cmdparser::MultipleCommand<std::string>;
    _p->addCommand(std::make_unique<T>(v, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "foo"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v.size()) << "foo";
        EXPECT_EQ("foo", v[0]) << "foo";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "bar"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v.size()) << "bar";
        EXPECT_EQ("bar", v[0]) << "bar";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "foo",
            "-single",
            "foo"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "foo foo";
        EXPECT_EQ("foo", v[0]) << "foo foo";
        EXPECT_EQ("foo", v[1]) << "foo foo";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "bar",
            "-single",
            "foo"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "bar foo";
        EXPECT_EQ("bar", v[0]) << "bar foo";
        EXPECT_EQ("foo", v[1]) << "bar foo";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "foo",
            "-single",
            "bar"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "foo bar";
        EXPECT_EQ("foo", v[0]) << "foo bar";
        EXPECT_EQ("bar", v[1]) << "foo bar";
    }
    v.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "bar",
            "-single",
            "bar"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v.size()) << "bar bar";
        EXPECT_EQ("bar", v[0]) << "bar bar";
        EXPECT_EQ("bar", v[1]) << "bar bar";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsBoolBool) {
    // bool-bool
    std::vector<bool> v1;
    std::vector<bool> v2;
    using T = ghoul::cmdparser::MultipleCommand<bool, bool>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));
    
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0";
        EXPECT_EQ(1, v2.size()) << "0 0";
        EXPECT_EQ(false, v1[0]) << "0 0";
        EXPECT_EQ(false, v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0";
        EXPECT_EQ(1, v2.size()) << "1 0";
        EXPECT_EQ(true, v1[0]) << "1 0";
        EXPECT_EQ(false, v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1";
        EXPECT_EQ(1, v2.size()) << "0 1";
        EXPECT_EQ(false, v1[0]) << "0 1";
        EXPECT_EQ(true, v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1";
        EXPECT_EQ(1, v2.size()) << "1 1";
        EXPECT_EQ(true, v1[0]) << "1 1";
        EXPECT_EQ(true, v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 | 0 0";
        EXPECT_EQ(false, v1[0]) << "0 0 | 0 0";
        EXPECT_EQ(false, v2[0]) << "0 0 | 0 0";
        EXPECT_EQ(false, v1[1]) << "0 0 | 0 0";
        EXPECT_EQ(false, v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 | 0 0";
        EXPECT_EQ(true, v1[0]) << "1 0 | 0 0";
        EXPECT_EQ(false, v2[0]) << "1 0 | 0 0";
        EXPECT_EQ(false, v1[1]) << "1 0 | 0 0";
        EXPECT_EQ(false, v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 | 0 0";
        EXPECT_EQ(false, v1[0]) << "0 1 | 0 0";
        EXPECT_EQ(true, v2[0]) << "0 1 | 0 0";
        EXPECT_EQ(false, v1[1]) << "0 1 | 0 0";
        EXPECT_EQ(false, v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 | 0 0";
        EXPECT_EQ(true, v1[0]) << "1 1 | 0 0";
        EXPECT_EQ(true, v2[0]) << "1 1 | 0 0";
        EXPECT_EQ(false, v1[1]) << "1 1 | 0 0";
        EXPECT_EQ(false, v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 | 1 1";
        EXPECT_EQ(false, v1[0]) << "0 0 | 1 1";
        EXPECT_EQ(false, v2[0]) << "0 0 | 1 1";
        EXPECT_EQ(true, v1[1]) << "0 0 | 1 1";
        EXPECT_EQ(true, v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 | 1 1";
        EXPECT_EQ(true, v1[0]) << "1 0 | 1 1";
        EXPECT_EQ(false, v2[0]) << "1 0 | 1 1";
        EXPECT_EQ(true, v1[1]) << "1 0 | 1 1";
        EXPECT_EQ(true, v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 | 1 1";
        EXPECT_EQ(false, v1[0]) << "0 1 | 1 1";
        EXPECT_EQ(true, v2[0]) << "0 1 | 1 1";
        EXPECT_EQ(true, v1[1]) << "0 1 | 1 1";
        EXPECT_EQ(true, v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 | 1 1";
        EXPECT_EQ(true, v1[0]) << "1 1 | 1 1";
        EXPECT_EQ(true, v2[0]) << "1 1 | 1 1";
        EXPECT_EQ(true, v1[1]) << "1 1 | 1 1";
        EXPECT_EQ(true, v2[1]) << "1 1 | 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsIntInt) {
    std::vector<int> v1;
    std::vector<int> v2;

    using T = ghoul::cmdparser::MultipleCommand<int, int>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0";
        EXPECT_EQ(1, v2.size()) << "0 0";
        EXPECT_EQ(0, v1[0]) << "0 0";
        EXPECT_EQ(0, v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0";
        EXPECT_EQ(1, v2.size()) << "1 0";
        EXPECT_EQ(1, v1[0]) << "1 0";
        EXPECT_EQ(0, v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1";
        EXPECT_EQ(1, v2.size()) << "0 1";
        EXPECT_EQ(0, v1[0]) << "0 1";
        EXPECT_EQ(1, v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1";
        EXPECT_EQ(1, v2.size()) << "1 1";
        EXPECT_EQ(1, v1[0]) << "1 1";
        EXPECT_EQ(1, v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 | 0 0";
        EXPECT_EQ(0, v1[0]) << "0 0 | 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 | 0 0";
        EXPECT_EQ(0, v1[1]) << "0 0 | 0 0";
        EXPECT_EQ(0, v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 | 0 0";
        EXPECT_EQ(1, v1[0]) << "1 0 | 0 0";
        EXPECT_EQ(0, v2[0]) << "1 0 | 0 0";
        EXPECT_EQ(0, v1[1]) << "1 0 | 0 0";
        EXPECT_EQ(0, v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 | 0 0";
        EXPECT_EQ(0, v1[0]) << "0 1 | 0 0";
        EXPECT_EQ(1, v2[0]) << "0 1 | 0 0";
        EXPECT_EQ(0, v1[1]) << "0 1 | 0 0";
        EXPECT_EQ(0, v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 | 0 0";
        EXPECT_EQ(1, v1[0]) << "1 1 | 0 0";
        EXPECT_EQ(1, v2[0]) << "1 1 | 0 0";
        EXPECT_EQ(0, v1[1]) << "1 1 | 0 0";
        EXPECT_EQ(0, v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 | 1 1";
        EXPECT_EQ(0, v1[0]) << "0 0 | 1 1";
        EXPECT_EQ(0, v2[0]) << "0 0 | 1 1";
        EXPECT_EQ(1, v1[1]) << "0 0 | 1 1";
        EXPECT_EQ(1, v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 | 1 1";
        EXPECT_EQ(1, v1[0]) << "1 0 | 1 1";
        EXPECT_EQ(0, v2[0]) << "1 0 | 1 1";
        EXPECT_EQ(1, v1[1]) << "1 0 | 1 1";
        EXPECT_EQ(1, v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 | 1 1";
        EXPECT_EQ(0, v1[0]) << "0 1 | 1 1";
        EXPECT_EQ(1, v2[0]) << "0 1 | 1 1";
        EXPECT_EQ(1, v1[1]) << "0 1 | 1 1";
        EXPECT_EQ(1, v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 | 1 1";
        EXPECT_EQ(1, v1[0]) << "1 1 | 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 | 1 1";
        EXPECT_EQ(1, v1[1]) << "1 1 | 1 1";
        EXPECT_EQ(1, v2[1]) << "1 1 | 1 1";
    }
}


TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsStringString) {
    // int-int
    std::vector<std::string> v1;
    std::vector<std::string> v2;

    using T = ghoul::cmdparser::MultipleCommand<std::string, std::string>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0";
        EXPECT_EQ(1, v2.size()) << "0 0";
        EXPECT_EQ("0", v1[0]) << "0 0";
        EXPECT_EQ("0", v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0";
        EXPECT_EQ(1, v2.size()) << "1 0";
        EXPECT_EQ("1", v1[0]) << "1 0";
        EXPECT_EQ("0", v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1";
        EXPECT_EQ(1, v2.size()) << "0 1";
        EXPECT_EQ("0", v1[0]) << "0 1";
        EXPECT_EQ("1", v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1";
        EXPECT_EQ(1, v2.size()) << "1 1";
        EXPECT_EQ("1", v1[0]) << "1 1";
        EXPECT_EQ("1", v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 | 0 0";
        EXPECT_EQ("0", v1[0]) << "0 0 | 0 0";
        EXPECT_EQ("0", v2[0]) << "0 0 | 0 0";
        EXPECT_EQ("0", v1[1]) << "0 0 | 0 0";
        EXPECT_EQ("0", v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 | 0 0";
        EXPECT_EQ("1", v1[0]) << "1 0 | 0 0";
        EXPECT_EQ("0", v2[0]) << "1 0 | 0 0";
        EXPECT_EQ("0", v1[1]) << "1 0 | 0 0";
        EXPECT_EQ("0", v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 | 0 0";
        EXPECT_EQ("0", v1[0]) << "0 1 | 0 0";
        EXPECT_EQ("1", v2[0]) << "0 1 | 0 0";
        EXPECT_EQ("0", v1[1]) << "0 1 | 0 0";
        EXPECT_EQ("0", v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 | 0 0";
        EXPECT_EQ("1", v1[0]) << "1 1 | 0 0";
        EXPECT_EQ("1", v2[0]) << "1 1 | 0 0";
        EXPECT_EQ("0", v1[1]) << "1 1 | 0 0";
        EXPECT_EQ("0", v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 | 1 1";
        EXPECT_EQ("0", v1[0]) << "0 0 | 1 1";
        EXPECT_EQ("0", v2[0]) << "0 0 | 1 1";
        EXPECT_EQ("1", v1[1]) << "0 0 | 1 1";
        EXPECT_EQ("1", v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 | 1 1";
        EXPECT_EQ("1", v1[0]) << "1 0 | 1 1";
        EXPECT_EQ("0", v2[0]) << "1 0 | 1 1";
        EXPECT_EQ("1", v1[1]) << "1 0 | 1 1";
        EXPECT_EQ("1", v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 | 1 1";
        EXPECT_EQ("0", v1[0]) << "0 1 | 1 1";
        EXPECT_EQ("1", v2[0]) << "0 1 | 1 1";
        EXPECT_EQ("1", v1[1]) << "0 1 | 1 1";
        EXPECT_EQ("1", v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 | 1 1";
        EXPECT_EQ("1", v1[0]) << "1 1 | 1 1";
        EXPECT_EQ("1", v2[0]) << "1 1 | 1 1";
        EXPECT_EQ("1", v1[1]) << "1 1 | 1 1";
        EXPECT_EQ("1", v2[1]) << "1 1 | 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsBoolInt) {
    std::vector<bool> v1;
    std::vector<int> v2;

    using T = ghoul::cmdparser::MultipleCommand<bool, int>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0";
        EXPECT_EQ(1, v2.size()) << "0 0";
        EXPECT_EQ(false, v1[0]) << "0 0";
        EXPECT_EQ(0, v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0";
        EXPECT_EQ(1, v2.size()) << "1 0";
        EXPECT_EQ(true, v1[0]) << "1 0";
        EXPECT_EQ(0, v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1";
        EXPECT_EQ(1, v2.size()) << "0 1";
        EXPECT_EQ(false, v1[0]) << "0 1";
        EXPECT_EQ(1, v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1";
        EXPECT_EQ(1, v2.size()) << "1 1";
        EXPECT_EQ(true, v1[0]) << "1 1";
        EXPECT_EQ(1, v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 | 0 0";
        EXPECT_EQ(false, v1[0]) << "0 0 | 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 | 0 0";
        EXPECT_EQ(false, v1[1]) << "0 0 | 0 0";
        EXPECT_EQ(0, v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 | 0 0";
        EXPECT_EQ(true, v1[0]) << "1 0 | 0 0";
        EXPECT_EQ(0, v2[0]) << "1 0 | 0 0";
        EXPECT_EQ(false, v1[1]) << "1 0 | 0 0";
        EXPECT_EQ(0, v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 | 0 0";
        EXPECT_EQ(false, v1[0]) << "0 1 | 0 0";
        EXPECT_EQ(1, v2[0]) << "0 1 | 0 0";
        EXPECT_EQ(false, v1[1]) << "0 1 | 0 0";
        EXPECT_EQ(0, v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 | 0 0";
        EXPECT_EQ(true, v1[0]) << "1 1 | 0 0";
        EXPECT_EQ(1, v2[0]) << "1 1 | 0 0";
        EXPECT_EQ(false, v1[1]) << "1 1 | 0 0";
        EXPECT_EQ(0, v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 | 1 1";
        EXPECT_EQ(false, v1[0]) << "0 0 | 1 1";
        EXPECT_EQ(0, v2[0]) << "0 0 | 1 1";
        EXPECT_EQ(true, v1[1]) << "0 0 | 1 1";
        EXPECT_EQ(1, v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 | 1 1";
        EXPECT_EQ(true, v1[0]) << "1 0 | 1 1";
        EXPECT_EQ(0, v2[0]) << "1 0 | 1 1";
        EXPECT_EQ(true, v1[1]) << "1 0 | 1 1";
        EXPECT_EQ(1, v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 | 1 1";
        EXPECT_EQ(false, v1[0]) << "0 1 | 1 1";
        EXPECT_EQ(1, v2[0]) << "0 1 | 1 1";
        EXPECT_EQ(true, v1[1]) << "0 1 | 1 1";
        EXPECT_EQ(1, v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 | 1 1";
        EXPECT_EQ(true, v1[0]) << "1 1 | 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 | 1 1";
        EXPECT_EQ(true, v1[1]) << "1 1 | 1 1";
        EXPECT_EQ(1, v2[1]) << "1 1 | 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsIntBool) {
    std::vector<int> v1;
    std::vector<bool> v2;

    using T = ghoul::cmdparser::MultipleCommand<int, bool>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0";
        EXPECT_EQ(1, v2.size()) << "0 0";
        EXPECT_EQ(0, v1[0]) << "0 0";
        EXPECT_EQ(false, v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0";
        EXPECT_EQ(1, v2.size()) << "1 0";
        EXPECT_EQ(1, v1[0]) << "1 0";
        EXPECT_EQ(false, v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1";
        EXPECT_EQ(1, v2.size()) << "0 1";
        EXPECT_EQ(0, v1[0]) << "0 1";
        EXPECT_EQ(true, v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1";
        EXPECT_EQ(1, v2.size()) << "1 1";
        EXPECT_EQ(1, v1[0]) << "1 1";
        EXPECT_EQ(true, v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 | 0 0";
        EXPECT_EQ(0, v1[0]) << "0 0 | 0 0";
        EXPECT_EQ(false, v2[0]) << "0 0 | 0 0";
        EXPECT_EQ(0, v1[1]) << "0 0 | 0 0";
        EXPECT_EQ(false, v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 | 0 0";
        EXPECT_EQ(1, v1[0]) << "1 0 | 0 0";
        EXPECT_EQ(false, v2[0]) << "1 0 | 0 0";
        EXPECT_EQ(0, v1[1]) << "1 0 | 0 0";
        EXPECT_EQ(false, v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 | 0 0";
        EXPECT_EQ(0, v1[0]) << "0 1 | 0 0";
        EXPECT_EQ(true, v2[0]) << "0 1 | 0 0";
        EXPECT_EQ(0, v1[1]) << "0 1 | 0 0";
        EXPECT_EQ(false, v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 | 0 0";
        EXPECT_EQ(1, v1[0]) << "1 1 | 0 0";
        EXPECT_EQ(true, v2[0]) << "1 1 | 0 0";
        EXPECT_EQ(0, v1[1]) << "1 1 | 0 0";
        EXPECT_EQ(false, v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 | 1 1";
        EXPECT_EQ(0, v1[0]) << "0 0 | 1 1";
        EXPECT_EQ(false, v2[0]) << "0 0 | 1 1";
        EXPECT_EQ(1, v1[1]) << "0 0 | 1 1";
        EXPECT_EQ(true, v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 | 1 1";
        EXPECT_EQ(1, v1[0]) << "1 0 | 1 1";
        EXPECT_EQ(false, v2[0]) << "1 0 | 1 1";
        EXPECT_EQ(1, v1[1]) << "1 0 | 1 1";
        EXPECT_EQ(true, v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 | 1 1";
        EXPECT_EQ(0, v1[0]) << "0 1 | 1 1";
        EXPECT_EQ(true, v2[0]) << "0 1 | 1 1";
        EXPECT_EQ(1, v1[1]) << "0 1 | 1 1";
        EXPECT_EQ(true, v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 | 1 1";
        EXPECT_EQ(1, v1[0]) << "1 1 | 1 1";
        EXPECT_EQ(true, v2[0]) << "1 1 | 1 1";
        EXPECT_EQ(1, v1[1]) << "1 1 | 1 1";
        EXPECT_EQ(true, v2[1]) << "1 1 | 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsIntString) {
    std::vector<int> v1;
    std::vector<std::string> v2;

    using T = ghoul::cmdparser::MultipleCommand<int, std::string>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0";
        EXPECT_EQ(1, v2.size()) << "0 0";
        EXPECT_EQ(0, v1[0]) << "0 0";
        EXPECT_EQ("0", v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0";
        EXPECT_EQ(1, v2.size()) << "1 0";
        EXPECT_EQ(1, v1[0]) << "1 0";
        EXPECT_EQ("0", v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1";
        EXPECT_EQ(1, v2.size()) << "0 1";
        EXPECT_EQ(0, v1[0]) << "0 1";
        EXPECT_EQ("1", v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1";
        EXPECT_EQ(1, v2.size()) << "1 1";
        EXPECT_EQ(1, v1[0]) << "1 1";
        EXPECT_EQ("1", v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 | 0 0";
        EXPECT_EQ(0, v1[0]) << "0 0 | 0 0";
        EXPECT_EQ("0", v2[0]) << "0 0 | 0 0";
        EXPECT_EQ(0, v1[1]) << "0 0 | 0 0";
        EXPECT_EQ("0", v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 | 0 0";
        EXPECT_EQ(1, v1[0]) << "1 0 | 0 0";
        EXPECT_EQ("0", v2[0]) << "1 0 | 0 0";
        EXPECT_EQ(0, v1[1]) << "1 0 | 0 0";
        EXPECT_EQ("0", v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 | 0 0";
        EXPECT_EQ(0, v1[0]) << "0 1 | 0 0";
        EXPECT_EQ("1", v2[0]) << "0 1 | 0 0";
        EXPECT_EQ(0, v1[1]) << "0 1 | 0 0";
        EXPECT_EQ("0", v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 | 0 0";
        EXPECT_EQ(1, v1[0]) << "1 1 | 0 0";
        EXPECT_EQ("1", v2[0]) << "1 1 | 0 0";
        EXPECT_EQ(0, v1[1]) << "1 1 | 0 0";
        EXPECT_EQ("0", v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 | 1 1";
        EXPECT_EQ(0, v1[0]) << "0 0 | 1 1";
        EXPECT_EQ("0", v2[0]) << "0 0 | 1 1";
        EXPECT_EQ(1, v1[1]) << "0 0 | 1 1";
        EXPECT_EQ("1", v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 | 1 1";
        EXPECT_EQ(1, v1[0]) << "1 0 | 1 1";
        EXPECT_EQ("0", v2[0]) << "1 0 | 1 1";
        EXPECT_EQ(1, v1[1]) << "1 0 | 1 1";
        EXPECT_EQ("1", v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 | 1 1";
        EXPECT_EQ(0, v1[0]) << "0 1 | 1 1";
        EXPECT_EQ("1", v2[0]) << "0 1 | 1 1";
        EXPECT_EQ(1, v1[1]) << "0 1 | 1 1";
        EXPECT_EQ("1", v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 | 1 1";
        EXPECT_EQ(1, v1[0]) << "1 1 | 1 1";
        EXPECT_EQ("1", v2[0]) << "1 1 | 1 1";
        EXPECT_EQ(1, v1[1]) << "1 1 | 1 1";
        EXPECT_EQ("1", v2[1]) << "1 1 | 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsStringInt) {
    std::vector<std::string> v1;
    std::vector<int> v2;

    using T = ghoul::cmdparser::MultipleCommand<std::string, int>;
    _p->addCommand(std::make_unique<T>(v1, v2, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0";
        EXPECT_EQ(1, v2.size()) << "0 0";
        EXPECT_EQ("0", v1[0]) << "0 0";
        EXPECT_EQ(0, v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0";
        EXPECT_EQ(1, v2.size()) << "1 0";
        EXPECT_EQ("1", v1[0]) << "1 0";
        EXPECT_EQ(0, v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1";
        EXPECT_EQ(1, v2.size()) << "0 1";
        EXPECT_EQ("0", v1[0]) << "0 1";
        EXPECT_EQ(1, v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1";
        EXPECT_EQ(1, v2.size()) << "1 1";
        EXPECT_EQ("1", v1[0]) << "1 1";
        EXPECT_EQ(1, v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 | 0 0";
        EXPECT_EQ("0", v1[0]) << "0 0 | 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 | 0 0";
        EXPECT_EQ("0", v1[1]) << "0 0 | 0 0";
        EXPECT_EQ(0, v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 | 0 0";
        EXPECT_EQ("1", v1[0]) << "1 0 | 0 0";
        EXPECT_EQ(0, v2[0]) << "1 0 | 0 0";
        EXPECT_EQ("0", v1[1]) << "1 0 | 0 0";
        EXPECT_EQ(0, v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 | 0 0";
        EXPECT_EQ("0", v1[0]) << "0 1 | 0 0";
        EXPECT_EQ(1, v2[0]) << "0 1 | 0 0";
        EXPECT_EQ("0", v1[1]) << "0 1 | 0 0";
        EXPECT_EQ(0, v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 | 0 0";
        EXPECT_EQ("1", v1[0]) << "1 1 | 0 0";
        EXPECT_EQ(1, v2[0]) << "1 1 | 0 0";
        EXPECT_EQ("0", v1[1]) << "1 1 | 0 0";
        EXPECT_EQ(0, v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 | 1 1";
        EXPECT_EQ("0", v1[0]) << "0 0 | 1 1";
        EXPECT_EQ(0, v2[0]) << "0 0 | 1 1";
        EXPECT_EQ("1", v1[1]) << "0 0 | 1 1";
        EXPECT_EQ(1, v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 | 1 1";
        EXPECT_EQ("1", v1[0]) << "1 0 | 1 1";
        EXPECT_EQ(0, v2[0]) << "1 0 | 1 1";
        EXPECT_EQ("1", v1[1]) << "1 0 | 1 1";
        EXPECT_EQ(1, v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 | 1 1";
        EXPECT_EQ("0", v1[0]) << "0 1 | 1 1";
        EXPECT_EQ(1, v2[0]) << "0 1 | 1 1";
        EXPECT_EQ("1", v1[1]) << "0 1 | 1 1";
        EXPECT_EQ(1, v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 | 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 | 1 1";
        EXPECT_EQ("1", v1[0]) << "1 1 | 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 | 1 1";
        EXPECT_EQ("1", v1[1]) << "1 1 | 1 1";
        EXPECT_EQ(1, v2[1]) << "1 1 | 1 1";
    }}

TEST_F(CommandlineParserTest, MultipleCommandThreeArgumentsBoolIntString) {
    std::vector<bool> v1;
    std::vector<int> v2;
    std::vector<std::string> v3;

    using T = ghoul::cmdparser::MultipleCommand<bool, int, std::string>;
    _p->addCommand(std::make_unique<T>(v1, v2, v3, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0 0";
        EXPECT_EQ(1, v2.size()) << "0 0 0";
        EXPECT_EQ(1, v3.size()) << "0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 0";
        EXPECT_EQ("0", v3[0]) << "0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0 0";
        EXPECT_EQ(1, v2.size()) << "1 0 0";
        EXPECT_EQ(1, v3.size()) << "1 0 0";
        EXPECT_EQ(true, v1[0]);
        EXPECT_EQ(0, v2[0]);
        EXPECT_EQ("0", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1 0";
        EXPECT_EQ(1, v2.size()) << "0 1 0";
        EXPECT_EQ(1, v3.size()) << "0 1 0";
        EXPECT_EQ(false, v1[0]);
        EXPECT_EQ(1, v2[0]);
        EXPECT_EQ("0", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1 0";
        EXPECT_EQ(1, v2.size()) << "1 1 0";
        EXPECT_EQ(1, v3.size()) << "1 1 0";
        EXPECT_EQ(true, v1[0]);
        EXPECT_EQ(1, v2[0]);
        EXPECT_EQ("0", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0 1";
        EXPECT_EQ(1, v2.size()) << "0 0 1";
        EXPECT_EQ(1, v3.size()) << "0 0 1";
        EXPECT_EQ(false, v1[0]);
        EXPECT_EQ(0, v2[0]);
        EXPECT_EQ("1", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0 1";
        EXPECT_EQ(1, v2.size()) << "1 0 1";
        EXPECT_EQ(1, v3.size()) << "1 0 1";
        EXPECT_EQ(true, v1[0]);
        EXPECT_EQ(0, v2[0]);
        EXPECT_EQ("1", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1 1";
        EXPECT_EQ(1, v2.size()) << "0 1 1";
        EXPECT_EQ(1, v3.size()) << "0 1 1";
        EXPECT_EQ(false, v1[0]);
        EXPECT_EQ(1, v2[0]);
        EXPECT_EQ("1", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1 1";
        EXPECT_EQ(1, v2.size()) << "1 1 1";
        EXPECT_EQ(1, v3.size()) << "1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 1";
        EXPECT_EQ("1", v3[0]) << "1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "-single",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 0 | 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 0 | 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 0 0 | 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 0 0 | 0 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 0 | 0 0 0";
        EXPECT_EQ("0", v3[0]) << "0 0 0 | 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 0 0 | 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 0 0 | 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 0 0 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "-single",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 0 | 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 0 | 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 0 0 | 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 0 0 | 0 0 0";
        EXPECT_EQ(0, v2[0]) << "1 0 0 | 0 0 0";
        EXPECT_EQ("0", v3[0]) << "1 0 0 | 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 0 0 | 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 0 0 | 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 0 0 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "-single",
            "0",
            "0",
            "0"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 0 | 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 0 | 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 1 0 | 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 1 0 | 0 0 0";
        EXPECT_EQ(1, v2[0]) << "0 1 0 | 0 0 0";
        EXPECT_EQ("0", v3[0]) << "0 1 0 | 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 1 0 | 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 1 0 | 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 1 0 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "-single",
            "0",
            "0",
            "0"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 0 | 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 0 | 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 1 0 | 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 1 0 | 0 0 0";
        EXPECT_EQ(1, v2[0]) << "1 1 0 | 0 0 0";
        EXPECT_EQ("0", v3[0]) << "1 1 0 | 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 1 0 | 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 1 0 | 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 1 0 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "-single",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 1 | 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 1 | 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 0 1 | 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 0 1 | 0 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 1 | 0 0 0";
        EXPECT_EQ("1", v3[0]) << "0 0 1 | 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 0 1 | 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 0 1 | 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 0 1 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "-single",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 1 | 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 1 | 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 0 1 | 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 0 1 | 0 0 0";
        EXPECT_EQ(0, v2[0]) << "1 0 1 | 0 0 0";
        EXPECT_EQ("1", v3[0]) << "1 0 1 | 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 0 1 | 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 0 1 | 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 0 1 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "-single",
            "0",
            "0",
            "0"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 1 | 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 1 | 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 1 1 | 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 1 1 | 0 0 0";
        EXPECT_EQ(1, v2[0]) << "0 1 1 | 0 0 0";
        EXPECT_EQ("1", v3[0]) << "0 1 1 | 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 1 1 | 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 1 1 | 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 1 1 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "-single",
            "0",
            "0",
            "0"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 1 | 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 1 | 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 1 1 | 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 1 1 | 0 0 0";
        EXPECT_EQ(1, v2[0]) << "1 1 1 | 0 0 0";
        EXPECT_EQ("1", v3[0]) << "1 1 1 | 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 1 1 | 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 1 1 | 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 1 1 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "-single",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 0 | 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 0 | 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 0 0 | 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 0 0 | 1 1 1";
        EXPECT_EQ(0, v2[0]) << "0 0 0 | 1 1 1";
        EXPECT_EQ("0", v3[0]) << "0 0 0 | 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 0 0 | 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 0 0 | 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 0 0 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "-single",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 0 | 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 0 | 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 0 0 | 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 0 0 | 1 1 1";
        EXPECT_EQ(0, v2[0]) << "1 0 0 | 1 1 1";
        EXPECT_EQ("0", v3[0]) << "1 0 0 | 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 0 0 | 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 0 0 | 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 0 0 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "-single",
            "1",
            "1",
            "1"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 0 | 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 0 | 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 1 0 | 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 1 0 | 1 1 1";
        EXPECT_EQ(1, v2[0]) << "0 1 0 | 1 1 1";
        EXPECT_EQ("0", v3[0]) << "0 1 0 | 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 1 0 | 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 1 0 | 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 1 0 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "-single",
            "1",
            "1",
            "1"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 0 | 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 0 | 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 1 0 | 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 1 0 | 1 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 0 | 1 1 1";
        EXPECT_EQ("0", v3[0]) << "1 1 0 | 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 1 0 | 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 1 0 | 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 1 0 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "-single",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 1 | 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 1 | 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 0 1 | 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 0 1 | 1 1 1";
        EXPECT_EQ(0, v2[0]) << "0 0 1 | 1 1 1";
        EXPECT_EQ("1", v3[0]) << "0 0 1 | 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 0 1 | 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 0 1 | 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 0 1 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "-single",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 1 | 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 1 | 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 0 1 | 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 0 1 | 1 1 1";
        EXPECT_EQ(0, v2[0]) << "1 0 1 | 1 1 1";
        EXPECT_EQ("1", v3[0]) << "1 0 1 | 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 0 1 | 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 0 1 | 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 0 1 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "-single",
            "1",
            "1",
            "1"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 1 | 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 1 | 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 1 1 | 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 1 1 | 1 1 1";
        EXPECT_EQ(1, v2[0]) << "0 1 1 | 1 1 1";
        EXPECT_EQ("1", v3[0]) << "0 1 1 | 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 1 1 | 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 1 1 | 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 1 1 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "-single",
            "1",
            "1",
            "1"

        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 1 | 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 1 | 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 1 1 | 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 1 1 | 1 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 1 | 1 1 1";
        EXPECT_EQ("1", v3[0]) << "1 1 1 | 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 1 1 | 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 1 1 | 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 1 1 | 1 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandFourArgumentsBoolIntStringFloat) {
    std::vector<bool> v1;
    std::vector<int> v2;
    std::vector<std::string> v3;
    std::vector<float> v4;

    using T = ghoul::cmdparser::MultipleCommand<bool, int, std::string, float>;
    _p->addCommand(std::make_unique<T>(v1, v2, v3, v4, "-single"));

    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0 0 0";
        EXPECT_EQ(1, v2.size()) << "0 0 0 0";
        EXPECT_EQ(1, v3.size()) << "0 0 0 0";
        EXPECT_EQ(1, v4.size()) << "0 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 0 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 0 0";
        EXPECT_EQ("0", v3[0]) << "0 0 0 0";
        EXPECT_EQ(0.f, v4[0]) << "0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0 0 0";
        EXPECT_EQ(1, v2.size()) << "1 0 0 0";
        EXPECT_EQ(1, v3.size()) << "1 0 0 0";
        EXPECT_EQ(1, v4.size()) << "1 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 0 0 0";
        EXPECT_EQ(0, v2[0]) << "1 0 0 0";
        EXPECT_EQ("0", v3[0]) << "1 0 0 0";
        EXPECT_EQ(0.f, v4[0]) << "1 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1 0 0";
        EXPECT_EQ(1, v2.size()) << "0 1 0 0";
        EXPECT_EQ(1, v3.size()) << "0 1 0 0";
        EXPECT_EQ(1, v4.size()) << "0 1 0 0";
        EXPECT_EQ(false, v1[0]) << "0 1 0 0";
        EXPECT_EQ(1, v2[0]) << "0 1 0 0";
        EXPECT_EQ("0", v3[0]) << "0 1 0 0";
        EXPECT_EQ(0.f, v4[0]) << "0 1 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1 0 0";
        EXPECT_EQ(1, v2.size()) << "1 1 0 0";
        EXPECT_EQ(1, v3.size()) << "1 1 0 0";
        EXPECT_EQ(1, v4.size()) << "1 1 0 0";
        EXPECT_EQ(true, v1[0]) << "1 1 0 0";
        EXPECT_EQ(1, v2[0]) << "1 1 0 0";
        EXPECT_EQ("0", v3[0]) << "1 1 0 0";
        EXPECT_EQ(0.f, v4[0]) << "1 1 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0 1 0";
        EXPECT_EQ(1, v2.size()) << "0 0 1 0";
        EXPECT_EQ(1, v3.size()) << "0 0 1 0";
        EXPECT_EQ(1, v4.size()) << "0 0 1 0";
        EXPECT_EQ(false, v1[0]) << "0 0 1 0";
        EXPECT_EQ(0, v2[0]) << "0 0 1 0";
        EXPECT_EQ("1", v3[0]) << "0 0 1 0";
        EXPECT_EQ(0.f, v4[0]) << "0 0 1 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0 1 0";
        EXPECT_EQ(1, v2.size()) << "1 0 1 0";
        EXPECT_EQ(1, v3.size()) << "1 0 1 0";
        EXPECT_EQ(1, v4.size()) << "1 0 1 0";
        EXPECT_EQ(true, v1[0]) << "1 0 1 0";
        EXPECT_EQ(0, v2[0]) << "1 0 1 0";
        EXPECT_EQ("1", v3[0]) << "1 0 1 0";
        EXPECT_EQ(0.f, v4[0]) << "1 0 1 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1 1 0";
        EXPECT_EQ(1, v2.size()) << "0 1 1 0";
        EXPECT_EQ(1, v3.size()) << "0 1 1 0";
        EXPECT_EQ(1, v4.size()) << "0 1 1 0";
        EXPECT_EQ(false, v1[0]) << "0 1 1 0";
        EXPECT_EQ(1, v2[0]) << "0 1 1 0";
        EXPECT_EQ("1", v3[0]) << "0 1 1 0";
        EXPECT_EQ(0.f, v4[0]) << "0 1 1 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1 1 0";
        EXPECT_EQ(1, v2.size()) << "1 1 1 0";
        EXPECT_EQ(1, v3.size()) << "1 1 1 0";
        EXPECT_EQ(1, v4.size()) << "1 1 1 0";
        EXPECT_EQ(true, v1[0]) << "1 1 1 0";
        EXPECT_EQ(1, v2[0]) << "1 1 1 0";
        EXPECT_EQ("1", v3[0]) << "1 1 1 0";
        EXPECT_EQ(0.f, v4[0]) << "1 1 1 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0 0 1";
        EXPECT_EQ(1, v2.size()) << "0 0 0 1";
        EXPECT_EQ(1, v3.size()) << "0 0 0 1";
        EXPECT_EQ(1, v4.size()) << "0 0 0 1";
        EXPECT_EQ(false, v1[0]) << "0 0 0 1";
        EXPECT_EQ(0, v2[0]) << "0 0 0 1";
        EXPECT_EQ("0", v3[0]) << "0 0 0 1";
        EXPECT_EQ(1.f, v4[0]) << "0 0 0 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0 0 1";
        EXPECT_EQ(1, v2.size()) << "1 0 0 1";
        EXPECT_EQ(1, v3.size()) << "1 0 0 1";
        EXPECT_EQ(1, v4.size()) << "1 0 0 1";
        EXPECT_EQ(true, v1[0]) << "1 0 0 1";
        EXPECT_EQ(0, v2[0]) << "1 0 0 1";
        EXPECT_EQ("0", v3[0]) << "1 0 0 1";
        EXPECT_EQ(1.f, v4[0]) << "1 0 0 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1 0 1";
        EXPECT_EQ(1, v2.size()) << "0 1 0 1";
        EXPECT_EQ(1, v3.size()) << "0 1 0 1";
        EXPECT_EQ(1, v4.size()) << "0 1 0 1";
        EXPECT_EQ(false, v1[0]) << "0 1 0 1";
        EXPECT_EQ(1, v2[0]) << "0 1 0 1";
        EXPECT_EQ("0", v3[0]) << "0 1 0 1";
        EXPECT_EQ(1.f, v4[0]) << "0 1 0 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1 0 1";
        EXPECT_EQ(1, v2.size()) << "1 1 0 1";
        EXPECT_EQ(1, v3.size()) << "1 1 0 1";
        EXPECT_EQ(1, v4.size()) << "1 1 0 1";
        EXPECT_EQ(true, v1[0]) << "1 1 0 1";
        EXPECT_EQ(1, v2[0]) << "1 1 0 1";
        EXPECT_EQ("0", v3[0]) << "1 1 0 1";
        EXPECT_EQ(1.f, v4[0]) << "1 1 0 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 0 1 1";
        EXPECT_EQ(1, v2.size()) << "0 0 1 1";
        EXPECT_EQ(1, v3.size()) << "0 0 1 1";
        EXPECT_EQ(1, v4.size()) << "0 0 1 1";
        EXPECT_EQ(false, v1[0]) << "0 0 1 1";
        EXPECT_EQ(0, v2[0]) << "0 0 1 1";
        EXPECT_EQ("1", v3[0]) << "0 0 1 1";
        EXPECT_EQ(1.f, v4[0]) << "0 0 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 0 1 1";
        EXPECT_EQ(1, v2.size()) << "1 0 1 1";
        EXPECT_EQ(1, v3.size()) << "1 0 1 1";
        EXPECT_EQ(1, v4.size()) << "1 0 1 1";
        EXPECT_EQ(true, v1[0]) << "1 0 1 1";
        EXPECT_EQ(0, v2[0]) << "1 0 1 1";
        EXPECT_EQ("1", v3[0]) << "1 0 1 1";
        EXPECT_EQ(1.f, v4[0]) << "1 0 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "0 1 1 1";
        EXPECT_EQ(1, v2.size()) << "0 1 1 1";
        EXPECT_EQ(1, v3.size()) << "0 1 1 1";
        EXPECT_EQ(1, v4.size()) << "0 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 1 1 1";
        EXPECT_EQ(1, v2[0]) << "0 1 1 1";
        EXPECT_EQ("1", v3[0]) << "0 1 1 1";
        EXPECT_EQ(1.f, v4[0]) << "0 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(1, v1.size()) << "1 1 1 1";
        EXPECT_EQ(1, v2.size()) << "1 1 1 1";
        EXPECT_EQ(1, v3.size()) << "1 1 1 1";
        EXPECT_EQ(1, v4.size()) << "1 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 1 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 1 1";
        EXPECT_EQ("1", v3[0]) << "1 1 1 1";
        EXPECT_EQ(1.f, v4[0]) << "1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "0",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[0]) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[0]) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 0 0 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "0 0 0 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "0",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[0]) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[0]) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[0]) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 0 0 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "1 0 0 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "0",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ(1, v2[0]) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[0]) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[0]) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 1 0 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "0 1 0 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "0",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ(1, v2[0]) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[0]) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[0]) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 1 0 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "1 1 0 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "0",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ("1", v3[0]) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[0]) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 0 1 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "0 0 1 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "0",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[0]) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ("1", v3[0]) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[0]) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 0 1 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "1 0 1 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "0",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ(1, v2[0]) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ("1", v3[0]) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[0]) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 1 1 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "0 1 1 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "0",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ(1, v2[0]) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ("1", v3[0]) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[0]) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 1 1 0 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "1 1 1 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "1",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[0]) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ(1.f, v4[0]) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 0 0 1 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "0 0 0 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "1",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[0]) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[0]) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ(1.f, v4[0]) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 0 0 1 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "1 0 0 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "1",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ(1, v2[0]) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[0]) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ(1.f, v4[0]) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 1 0 1 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "0 1 0 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "1",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ(1, v2[0]) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[0]) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ(1.f, v4[0]) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 1 0 1 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "1 1 0 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "1",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[0]) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ("1", v3[0]) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ(1.f, v4[0]) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 0 1 1 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "0 0 1 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "1",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[0]) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ("1", v3[0]) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ(1.f, v4[0]) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 0 1 1 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "1 0 1 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "1",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[0]) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ(1, v2[0]) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ("1", v3[0]) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ(1.f, v4[0]) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "0 1 1 1 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "0 1 1 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "1",
            "-single",
            "0",
            "0",
            "0",
            "0"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v2.size()) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v3.size()) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ(2, v4.size()) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ(true, v1[0]) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ(1, v2[0]) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ("1", v3[0]) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ(1.f, v4[0]) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ(false, v1[1]) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ(0, v2[1]) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ("0", v3[1]) << "1 1 1 1 | 0 0 0 0";
        EXPECT_EQ(0.f, v4[1]) << "1 1 1 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "0",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ(0, v2[0]) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ("0", v3[0]) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ(0.f, v4[0]) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 0 0 0 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "0 0 0 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "0",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ(0, v2[0]) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ("0", v3[0]) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ(0.f, v4[0]) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 0 0 0 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "1 0 0 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "0",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[0]) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ("0", v3[0]) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ(0.f, v4[0]) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 1 0 0 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "0 1 0 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "0",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ("0", v3[0]) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ(0.f, v4[0]) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 1 0 0 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "1 1 0 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "0",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ(0, v2[0]) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[0]) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ(0.f, v4[0]) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 0 1 0 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "0 0 1 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "0",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ(0, v2[0]) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[0]) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ(0.f, v4[0]) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 0 1 0 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "1 0 1 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "0",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[0]) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[0]) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ(0.f, v4[0]) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 1 1 0 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "0 1 1 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "0",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[0]) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ(0.f, v4[0]) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 1 1 0 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "1 1 1 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "0",
            "1",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ(0, v2[0]) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ("0", v3[0]) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[0]) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 0 0 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "0 0 0 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "0",
            "1",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ(0, v2[0]) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ("0", v3[0]) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[0]) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 0 0 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "1 0 0 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "0",
            "1",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[0]) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ("0", v3[0]) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[0]) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 1 0 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "0 1 0 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "0",
            "1",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ("0", v3[0]) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[0]) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 1 0 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "1 1 0 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "0",
            "1",
            "1",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ(0, v2[0]) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[0]) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[0]) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 0 1 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "0 0 1 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "0",
            "1",
            "1",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ(0, v2[0]) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[0]) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[0]) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 0 1 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "1 0 1 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "0",
            "1",
            "1",
            "1",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ(false, v1[0]) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[0]) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[0]) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[0]) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "0 1 1 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "0 1 1 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        std::vector<std::string> argv = {
            "tests",
            "-single",
            "1",
            "1",
            "1",
            "1",
            "-single",
            "1",
            "1",
            "1",
            "1"
        };

        _p->setCommandLine(argv);
        ASSERT_NO_THROW(_p->execute());
        EXPECT_EQ(2, v1.size()) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v2.size()) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v3.size()) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ(2, v4.size()) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[0]) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[0]) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[0]) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[0]) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ(true, v1[1]) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ(1, v2[1]) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ("1", v3[1]) << "1 1 1 1 | 1 1 1 1";
        EXPECT_EQ(1.f, v4[1]) << "1 1 1 1 | 1 1 1 1";
    }
}

#ifdef __unix__
#pragma GCC diagnostic pop
#endif // __unix__

#endif 