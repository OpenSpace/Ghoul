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

#include <ghoul/cmdparser/cmdparser>

// GCC throws a lot of unnecessary
// warning: deprecated conversion from string constant to ‘char*’
// warnings
#ifdef __unix__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif //__unix__

/*
Test checklist:
+++ SingleCommand, MultipleCommand
+++     1-4 arguments
+++     different types
+++     same types
+++     calling once
+++     calling multiple times
+++ Multiple commands in the same command-line result
+++ Variable orders should produce the same result
+++ Unknown commands
+++ Collection of unknown commands with known commands interspersed
+++ Error messages when unknown commands are allowed but no receiving vector is provided
    (and vice versa)
*/

class CommandlineParserTest : public testing::Test {
protected:
    CommandlineParserTest() {
        _p = new ghoul::cmdparser::CommandlineParser;
    }

    ~CommandlineParserTest() {
    }

    void reset() {
        delete _p;
        _p = new ghoul::cmdparser::CommandlineParser;
    }

    ghoul::cmdparser::CommandlineParser* _p;
};

TEST_F(CommandlineParserTest, UnknownCommandsUnhandled) {
    char* argv[] = {
        "tests",
        "-cmd1",
        "arg",
        "-cmd2",
        "arg2"
    };

    _p->setCommandLine(5, argv);
    const bool res = _p->execute();
    ASSERT_EQ(false, res);
}

TEST_F(CommandlineParserTest, UnknownCommandsHandledWrongly1) {
    std::vector<std::string> arguments;
    char* argv[] = {
        "tests",
        "-cmd1",
        "arg",
        "-cmd2",
        "arg2"
    };

    //_p->setAllowUnknownCommands(true);
    _p->setCommandLine(5, argv, &arguments);
    const bool res = _p->execute();
    ASSERT_EQ(false, res);
}

TEST_F(CommandlineParserTest, UnknownCommandsHandledWrongly2) {
    //std::vector<std::string> arguments;
    char* argv[] = {
        "tests",
        "-cmd1",
        "arg",
        "-cmd2",
        "arg2"
    };

    _p->setAllowUnknownCommands(true);
    //_p->setCommandLine(5, argv, &arguments);
    _p->setCommandLine(5, argv);
    const bool res = _p->execute();
    ASSERT_EQ(false, res);
}

TEST_F(CommandlineParserTest, UnknownCommandsHandledCorrectly) {
    std::vector<std::string> arguments;
    char* argv[] = {
        "tests",
        "-cmd1",
        "arg",
        "-cmd2",
        "arg2"
    };

    _p->setAllowUnknownCommands(true);
    _p->setCommandLine(5, argv, &arguments);
    const bool res = _p->execute();
    ASSERT_EQ(true, res);
}

TEST_F(CommandlineParserTest, UnknownCommandsInterspersed) {
    std::vector<std::string> arguments;
    char* argv[] = {
        "tests",
        "-cmd1",
        "arg",
        "-cmd2",
        "arg2",
        "arg3",
        "-cmd3",
        "arg4"
    };
    std::string v1 = "";
    std::string v2 = "";
    ghoul::cmdparser::SingleCommand<std::string>* c =
        new ghoul::cmdparser::SingleCommand<std::string>(&v1, &v2, "-cmd2");
    _p->addCommand(c);

    _p->setAllowUnknownCommands(true);
    _p->setCommandLine(8, argv, &arguments);
    const bool res = _p->execute();
    ASSERT_EQ(true, res);
    ASSERT_EQ(4, arguments.size());
    ASSERT_EQ("-cmd1", arguments[0]);
    ASSERT_EQ("arg", arguments[1]);
    ASSERT_EQ("-cmd3", arguments[2]);
    ASSERT_EQ("arg4", arguments[3]);
    ASSERT_EQ("arg2", v1);
    ASSERT_EQ("arg3", v2);
}

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
    const bool res = _p->execute();
    ASSERT_EQ(true, res);
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
        ASSERT_EQ(false, v) << "0";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "1"
        };

        _p->setCommandLine(3, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
        ASSERT_EQ(true, v) << "1";
    }
}

TEST_F(CommandlineParserTest, SingleCommandCalledMultipleTimes) {
    // boolean
    bool v = false;
    ghoul::cmdparser::SingleCommand<bool>* c =
        new ghoul::cmdparser::SingleCommand<bool>(&v, "-single");
    _p->addCommand(c);

    char* argv[] = {
        "tests",
        "-single",
        "0",
        "-single",
        "0"
    };

    _p->setCommandLine(5, argv);
    const bool res = _p->execute();
    ASSERT_EQ(false, res);
}

TEST_F(CommandlineParserTest, MultipleCommandsPermutation) {
    int v1 = 0;
    int v2 = 0;
    int v3 = 0;
    ghoul::cmdparser::SingleCommand<int>* c1 =
        new ghoul::cmdparser::SingleCommand<int>(&v1, "-cmd1");
    ghoul::cmdparser::SingleCommand<int>* c2 =
        new ghoul::cmdparser::SingleCommand<int>(&v2, "-cmd2");
    ghoul::cmdparser::SingleCommand<int>* c3 =
        new ghoul::cmdparser::SingleCommand<int>(&v3, "-cmd3");
    _p->addCommand(c1);
    _p->addCommand(c2);
    _p->addCommand(c3);

    {
        char* argv[] = {
            "tests",
            "-cmd1",
            "1",
            "-cmd2",
            "2",
            "-cmd3",
            "3"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
        ASSERT_EQ(1, v1) << "cmd1 cmd2 cmd3";
        ASSERT_EQ(2, v2) << "cmd1 cmd2 cmd3";
        ASSERT_EQ(3, v3) << "cmd1 cmd2 cmd3";
    }
    v1 = 0;
    v2 = 0;
    v3 = 0;
    {
        char* argv[] = {
            "tests",
            "-cmd2",
            "2",
            "-cmd1",
            "1",
            "-cmd3",
            "3"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
        ASSERT_EQ(1, v1) << "cmd2 cmd1 cmd3";
        ASSERT_EQ(2, v2) << "cmd2 cmd1 cmd3";
        ASSERT_EQ(3, v3) << "cmd2 cmd1 cmd3";
    }
    v1 = 0;
    v2 = 0;
    v3 = 0;
    {
        char* argv[] = {
            "tests",
            "-cmd3",
            "3",
            "-cmd2",
            "2",
            "-cmd1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
        ASSERT_EQ(1, v1) << "cmd3 cmd2 cmd1";
        ASSERT_EQ(2, v2) << "cmd3 cmd2 cmd1";
        ASSERT_EQ(3, v3) << "cmd3 cmd2 cmd1";
    }
    v1 = 0;
    v2 = 0;
    v3 = 0;
    {
        char* argv[] = {
            "tests",
            "-cmd3",
            "3",
            "-cmd1",
            "1",
            "-cmd2",
            "2"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
        ASSERT_EQ(1, v1) << "cmd3 cmd1 cmd2";
        ASSERT_EQ(2, v2) << "cmd3 cmd1 cmd2";
        ASSERT_EQ(3, v3) << "cmd3 cmd1 cmd2";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
        ASSERT_EQ(1, v) << "1";
    }
    {
        char* argv[] = {
            "tests",
            "-single",
            "0"
        };

        _p->setCommandLine(3, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res);
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << " 1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "2x zero";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "6x zero";
        EXPECT_EQ(6, v) << "6x zero";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandOneArgumentBool) {
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1";
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
            "0"
        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
        ASSERT_EQ(2, v.size()) << "0 0";
        ASSERT_EQ(false, v[0]) << "0 0";
        ASSERT_EQ(false, v[1]) << "0 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
        ASSERT_EQ(2, v.size()) << "1 0";
        ASSERT_EQ(true, v[0]) << "1 0";
        ASSERT_EQ(false, v[1]) << "1 0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
        ASSERT_EQ(2, v.size()) << "1 1";
        ASSERT_EQ(true, v[0]) << "1 1";
        ASSERT_EQ(false, v[1]) << "1 1";
    }

}

TEST_F(CommandlineParserTest, MultipleCommandOneArgumentInt) {
    std::vector<int> v;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1";
        ASSERT_EQ(1, v.size()) << "1";
        ASSERT_EQ(1, v[0]) << "1";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0"
        };

        _p->setCommandLine(3, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0";
        ASSERT_EQ(1, v.size()) << "0";
        ASSERT_EQ(0, v[0]) << "0";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
        ASSERT_EQ(2, v.size()) << "1 0";
        ASSERT_EQ(1, v[0]) << "1 0";
        ASSERT_EQ(0, v[1]) << "1 0";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "-single",
            "0"
        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
        ASSERT_EQ(2, v.size()) << "0 0";
        ASSERT_EQ(0, v[0]) << "0 0";
        ASSERT_EQ(0, v[1]) << "0 0";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "-single",
            "1"
        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
        ASSERT_EQ(2, v.size()) << "1 1";
        ASSERT_EQ(1, v[0]) << "1 1";
        ASSERT_EQ(1, v[1]) << "1 1";
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
        ASSERT_EQ(2, v.size()) << "0 1";
        ASSERT_EQ(0, v[0]) << "0 1";
        ASSERT_EQ(2, v.size()) << "0 1";
        ASSERT_EQ(1, v[1]) << "0 1";
    }
}


TEST_F(CommandlineParserTest, MultipleCommandOneArgumentString) {
    std::vector<std::string> v;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "foo";
        ASSERT_EQ(1, v.size()) << "foo";
        ASSERT_EQ("foo", v[0]) << "foo";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "bar"
        };

        _p->setCommandLine(3, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "bar";
        ASSERT_EQ(1, v.size()) << "bar";
        ASSERT_EQ("bar", v[0]) << "bar";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "foo",
            "-single",
            "foo"
        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "foo foo";
        ASSERT_EQ(2, v.size()) << "foo foo";
        ASSERT_EQ("foo", v[0]) << "foo foo";
        ASSERT_EQ("foo", v[1]) << "foo foo";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "bar",
            "-single",
            "foo"
        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "bar foo";
        ASSERT_EQ(2, v.size()) << "bar foo";
        ASSERT_EQ("bar", v[0]) << "bar foo";
        ASSERT_EQ("foo", v[1]) << "bar foo";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "foo",
            "-single",
            "bar"
        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "foo bar";
        ASSERT_EQ(2, v.size()) << "foo bar";
        ASSERT_EQ("foo", v[0]) << "foo bar";
        ASSERT_EQ("bar", v[1]) << "foo bar";
    }
    v.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "bar",
            "-single",
            "bar"
        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "bar bar";
        ASSERT_EQ(2, v.size()) << "bar bar";
        ASSERT_EQ("bar", v[0]) << "bar bar";
        ASSERT_EQ("bar", v[1]) << "bar bar";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsBoolBool) {
    // bool-bool
    std::vector<bool> v1;
    std::vector<bool> v2;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
        ASSERT_EQ(1, v1.size()) << "0 0";
        ASSERT_EQ(1, v2.size()) << "0 0";
        ASSERT_EQ(false, v1[0]) << "0 0";
        ASSERT_EQ(false, v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
        ASSERT_EQ(1, v1.size()) << "1 0";
        ASSERT_EQ(1, v2.size()) << "1 0";
        ASSERT_EQ(true, v1[0]) << "1 0";
        ASSERT_EQ(false, v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
        ASSERT_EQ(1, v1.size()) << "0 1";
        ASSERT_EQ(1, v2.size()) << "0 1";
        ASSERT_EQ(false, v1[0]) << "0 1";
        ASSERT_EQ(true, v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
        ASSERT_EQ(1, v1.size()) << "1 1";
        ASSERT_EQ(1, v2.size()) << "1 1";
        ASSERT_EQ(true, v1[0]) << "1 1";
        ASSERT_EQ(true, v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 | 0 0";
        ASSERT_EQ(false, v1[0]) << "0 0 | 0 0";
        ASSERT_EQ(false, v2[0]) << "0 0 | 0 0";
        ASSERT_EQ(false, v1[1]) << "0 0 | 0 0";
        ASSERT_EQ(false, v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 | 0 0";
        ASSERT_EQ(true, v1[0]) << "1 0 | 0 0";
        ASSERT_EQ(false, v2[0]) << "1 0 | 0 0";
        ASSERT_EQ(false, v1[1]) << "1 0 | 0 0";
        ASSERT_EQ(false, v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 | 0 0";
        ASSERT_EQ(false, v1[0]) << "0 1 | 0 0";
        ASSERT_EQ(true, v2[0]) << "0 1 | 0 0";
        ASSERT_EQ(false, v1[1]) << "0 1 | 0 0";
        ASSERT_EQ(false, v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 | 0 0";
        ASSERT_EQ(true, v1[0]) << "1 1 | 0 0";
        ASSERT_EQ(true, v2[0]) << "1 1 | 0 0";
        ASSERT_EQ(false, v1[1]) << "1 1 | 0 0";
        ASSERT_EQ(false, v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 | 1 1";
        ASSERT_EQ(false, v1[0]) << "0 0 | 1 1";
        ASSERT_EQ(false, v2[0]) << "0 0 | 1 1";
        ASSERT_EQ(true, v1[1]) << "0 0 | 1 1";
        ASSERT_EQ(true, v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 | 1 1";
        ASSERT_EQ(true, v1[0]) << "1 0 | 1 1";
        ASSERT_EQ(false, v2[0]) << "1 0 | 1 1";
        ASSERT_EQ(true, v1[1]) << "1 0 | 1 1";
        ASSERT_EQ(true, v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 | 1 1";
        ASSERT_EQ(false, v1[0]) << "0 1 | 1 1";
        ASSERT_EQ(true, v2[0]) << "0 1 | 1 1";
        ASSERT_EQ(true, v1[1]) << "0 1 | 1 1";
        ASSERT_EQ(true, v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 | 1 1";
        ASSERT_EQ(true, v1[0]) << "1 1 | 1 1";
        ASSERT_EQ(true, v2[0]) << "1 1 | 1 1";
        ASSERT_EQ(true, v1[1]) << "1 1 | 1 1";
        ASSERT_EQ(true, v2[1]) << "1 1 | 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsIntInt) {
    std::vector<int> v1;
    std::vector<int> v2;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
        ASSERT_EQ(1, v1.size()) << "0 0";
        ASSERT_EQ(1, v2.size()) << "0 0";
        ASSERT_EQ(0, v1[0]) << "0 0";
        ASSERT_EQ(0, v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
        ASSERT_EQ(1, v1.size()) << "1 0";
        ASSERT_EQ(1, v2.size()) << "1 0";
        ASSERT_EQ(1, v1[0]) << "1 0";
        ASSERT_EQ(0, v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
        ASSERT_EQ(1, v1.size()) << "0 1";
        ASSERT_EQ(1, v2.size()) << "0 1";
        ASSERT_EQ(0, v1[0]) << "0 1";
        ASSERT_EQ(1, v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
        ASSERT_EQ(1, v1.size()) << "1 1";
        ASSERT_EQ(1, v2.size()) << "1 1";
        ASSERT_EQ(1, v1[0]) << "1 1";
        ASSERT_EQ(1, v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 | 0 0";
        ASSERT_EQ(0, v1[0]) << "0 0 | 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 | 0 0";
        ASSERT_EQ(0, v1[1]) << "0 0 | 0 0";
        ASSERT_EQ(0, v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 | 0 0";
        ASSERT_EQ(1, v1[0]) << "1 0 | 0 0";
        ASSERT_EQ(0, v2[0]) << "1 0 | 0 0";
        ASSERT_EQ(0, v1[1]) << "1 0 | 0 0";
        ASSERT_EQ(0, v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 | 0 0";
        ASSERT_EQ(0, v1[0]) << "0 1 | 0 0";
        ASSERT_EQ(1, v2[0]) << "0 1 | 0 0";
        ASSERT_EQ(0, v1[1]) << "0 1 | 0 0";
        ASSERT_EQ(0, v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 | 0 0";
        ASSERT_EQ(1, v1[0]) << "1 1 | 0 0";
        ASSERT_EQ(1, v2[0]) << "1 1 | 0 0";
        ASSERT_EQ(0, v1[1]) << "1 1 | 0 0";
        ASSERT_EQ(0, v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 | 1 1";
        ASSERT_EQ(0, v1[0]) << "0 0 | 1 1";
        ASSERT_EQ(0, v2[0]) << "0 0 | 1 1";
        ASSERT_EQ(1, v1[1]) << "0 0 | 1 1";
        ASSERT_EQ(1, v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 | 1 1";
        ASSERT_EQ(1, v1[0]) << "1 0 | 1 1";
        ASSERT_EQ(0, v2[0]) << "1 0 | 1 1";
        ASSERT_EQ(1, v1[1]) << "1 0 | 1 1";
        ASSERT_EQ(1, v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 | 1 1";
        ASSERT_EQ(0, v1[0]) << "0 1 | 1 1";
        ASSERT_EQ(1, v2[0]) << "0 1 | 1 1";
        ASSERT_EQ(1, v1[1]) << "0 1 | 1 1";
        ASSERT_EQ(1, v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 | 1 1";
        ASSERT_EQ(1, v1[0]) << "1 1 | 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 | 1 1";
        ASSERT_EQ(1, v1[1]) << "1 1 | 1 1";
        ASSERT_EQ(1, v2[1]) << "1 1 | 1 1";
    }
}


TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsStringString) {
    // int-int
    std::vector<std::string> v1;
    std::vector<std::string> v2;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
        ASSERT_EQ(1, v1.size()) << "0 0";
        ASSERT_EQ(1, v2.size()) << "0 0";
        ASSERT_EQ("0", v1[0]) << "0 0";
        ASSERT_EQ("0", v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
        ASSERT_EQ(1, v1.size()) << "1 0";
        ASSERT_EQ(1, v2.size()) << "1 0";
        ASSERT_EQ("1", v1[0]) << "1 0";
        ASSERT_EQ("0", v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
        ASSERT_EQ(1, v1.size()) << "0 1";
        ASSERT_EQ(1, v2.size()) << "0 1";
        ASSERT_EQ("0", v1[0]) << "0 1";
        ASSERT_EQ("1", v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
        ASSERT_EQ(1, v1.size()) << "1 1";
        ASSERT_EQ(1, v2.size()) << "1 1";
        ASSERT_EQ("1", v1[0]) << "1 1";
        ASSERT_EQ("1", v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 | 0 0";
        ASSERT_EQ("0", v1[0]) << "0 0 | 0 0";
        ASSERT_EQ("0", v2[0]) << "0 0 | 0 0";
        ASSERT_EQ("0", v1[1]) << "0 0 | 0 0";
        ASSERT_EQ("0", v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 | 0 0";
        ASSERT_EQ("1", v1[0]) << "1 0 | 0 0";
        ASSERT_EQ("0", v2[0]) << "1 0 | 0 0";
        ASSERT_EQ("0", v1[1]) << "1 0 | 0 0";
        ASSERT_EQ("0", v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 | 0 0";
        ASSERT_EQ("0", v1[0]) << "0 1 | 0 0";
        ASSERT_EQ("1", v2[0]) << "0 1 | 0 0";
        ASSERT_EQ("0", v1[1]) << "0 1 | 0 0";
        ASSERT_EQ("0", v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 | 0 0";
        ASSERT_EQ("1", v1[0]) << "1 1 | 0 0";
        ASSERT_EQ("1", v2[0]) << "1 1 | 0 0";
        ASSERT_EQ("0", v1[1]) << "1 1 | 0 0";
        ASSERT_EQ("0", v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 | 1 1";
        ASSERT_EQ("0", v1[0]) << "0 0 | 1 1";
        ASSERT_EQ("0", v2[0]) << "0 0 | 1 1";
        ASSERT_EQ("1", v1[1]) << "0 0 | 1 1";
        ASSERT_EQ("1", v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 | 1 1";
        ASSERT_EQ("1", v1[0]) << "1 0 | 1 1";
        ASSERT_EQ("0", v2[0]) << "1 0 | 1 1";
        ASSERT_EQ("1", v1[1]) << "1 0 | 1 1";
        ASSERT_EQ("1", v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 | 1 1";
        ASSERT_EQ("0", v1[0]) << "0 1 | 1 1";
        ASSERT_EQ("1", v2[0]) << "0 1 | 1 1";
        ASSERT_EQ("1", v1[1]) << "0 1 | 1 1";
        ASSERT_EQ("1", v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 | 1 1";
        ASSERT_EQ("1", v1[0]) << "1 1 | 1 1";
        ASSERT_EQ("1", v2[0]) << "1 1 | 1 1";
        ASSERT_EQ("1", v1[1]) << "1 1 | 1 1";
        ASSERT_EQ("1", v2[1]) << "1 1 | 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsBoolInt) {
    std::vector<bool> v1;
    std::vector<int> v2;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
        ASSERT_EQ(1, v1.size()) << "0 0";
        ASSERT_EQ(1, v2.size()) << "0 0";
        ASSERT_EQ(false, v1[0]) << "0 0";
        ASSERT_EQ(0, v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
        ASSERT_EQ(1, v1.size()) << "1 0";
        ASSERT_EQ(1, v2.size()) << "1 0";
        ASSERT_EQ(true, v1[0]) << "1 0";
        ASSERT_EQ(0, v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
        ASSERT_EQ(1, v1.size()) << "0 1";
        ASSERT_EQ(1, v2.size()) << "0 1";
        ASSERT_EQ(false, v1[0]) << "0 1";
        ASSERT_EQ(1, v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
        ASSERT_EQ(1, v1.size()) << "1 1";
        ASSERT_EQ(1, v2.size()) << "1 1";
        ASSERT_EQ(true, v1[0]) << "1 1";
        ASSERT_EQ(1, v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 | 0 0";
        ASSERT_EQ(false, v1[0]) << "0 0 | 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 | 0 0";
        ASSERT_EQ(false, v1[1]) << "0 0 | 0 0";
        ASSERT_EQ(0, v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 | 0 0";
        ASSERT_EQ(true, v1[0]) << "1 0 | 0 0";
        ASSERT_EQ(0, v2[0]) << "1 0 | 0 0";
        ASSERT_EQ(false, v1[1]) << "1 0 | 0 0";
        ASSERT_EQ(0, v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 | 0 0";
        ASSERT_EQ(false, v1[0]) << "0 1 | 0 0";
        ASSERT_EQ(1, v2[0]) << "0 1 | 0 0";
        ASSERT_EQ(false, v1[1]) << "0 1 | 0 0";
        ASSERT_EQ(0, v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 | 0 0";
        ASSERT_EQ(true, v1[0]) << "1 1 | 0 0";
        ASSERT_EQ(1, v2[0]) << "1 1 | 0 0";
        ASSERT_EQ(false, v1[1]) << "1 1 | 0 0";
        ASSERT_EQ(0, v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 | 1 1";
        ASSERT_EQ(false, v1[0]) << "0 0 | 1 1";
        ASSERT_EQ(0, v2[0]) << "0 0 | 1 1";
        ASSERT_EQ(true, v1[1]) << "0 0 | 1 1";
        ASSERT_EQ(1, v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 | 1 1";
        ASSERT_EQ(true, v1[0]) << "1 0 | 1 1";
        ASSERT_EQ(0, v2[0]) << "1 0 | 1 1";
        ASSERT_EQ(true, v1[1]) << "1 0 | 1 1";
        ASSERT_EQ(1, v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 | 1 1";
        ASSERT_EQ(false, v1[0]) << "0 1 | 1 1";
        ASSERT_EQ(1, v2[0]) << "0 1 | 1 1";
        ASSERT_EQ(true, v1[1]) << "0 1 | 1 1";
        ASSERT_EQ(1, v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 | 1 1";
        ASSERT_EQ(true, v1[0]) << "1 1 | 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 | 1 1";
        ASSERT_EQ(true, v1[1]) << "1 1 | 1 1";
        ASSERT_EQ(1, v2[1]) << "1 1 | 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsIntBool) {
    std::vector<int> v1;
    std::vector<bool> v2;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
        ASSERT_EQ(1, v1.size()) << "0 0";
        ASSERT_EQ(1, v2.size()) << "0 0";
        ASSERT_EQ(0, v1[0]) << "0 0";
        ASSERT_EQ(false, v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
        ASSERT_EQ(1, v1.size()) << "1 0";
        ASSERT_EQ(1, v2.size()) << "1 0";
        ASSERT_EQ(1, v1[0]) << "1 0";
        ASSERT_EQ(false, v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
        ASSERT_EQ(1, v1.size()) << "0 1";
        ASSERT_EQ(1, v2.size()) << "0 1";
        ASSERT_EQ(0, v1[0]) << "0 1";
        ASSERT_EQ(true, v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
        ASSERT_EQ(1, v1.size()) << "1 1";
        ASSERT_EQ(1, v2.size()) << "1 1";
        ASSERT_EQ(1, v1[0]) << "1 1";
        ASSERT_EQ(true, v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 | 0 0";
        ASSERT_EQ(0, v1[0]) << "0 0 | 0 0";
        ASSERT_EQ(false, v2[0]) << "0 0 | 0 0";
        ASSERT_EQ(0, v1[1]) << "0 0 | 0 0";
        ASSERT_EQ(false, v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 | 0 0";
        ASSERT_EQ(1, v1[0]) << "1 0 | 0 0";
        ASSERT_EQ(false, v2[0]) << "1 0 | 0 0";
        ASSERT_EQ(0, v1[1]) << "1 0 | 0 0";
        ASSERT_EQ(false, v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 | 0 0";
        ASSERT_EQ(0, v1[0]) << "0 1 | 0 0";
        ASSERT_EQ(true, v2[0]) << "0 1 | 0 0";
        ASSERT_EQ(0, v1[1]) << "0 1 | 0 0";
        ASSERT_EQ(false, v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 | 0 0";
        ASSERT_EQ(1, v1[0]) << "1 1 | 0 0";
        ASSERT_EQ(true, v2[0]) << "1 1 | 0 0";
        ASSERT_EQ(0, v1[1]) << "1 1 | 0 0";
        ASSERT_EQ(false, v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 | 1 1";
        ASSERT_EQ(0, v1[0]) << "0 0 | 1 1";
        ASSERT_EQ(false, v2[0]) << "0 0 | 1 1";
        ASSERT_EQ(1, v1[1]) << "0 0 | 1 1";
        ASSERT_EQ(true, v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 | 1 1";
        ASSERT_EQ(1, v1[0]) << "1 0 | 1 1";
        ASSERT_EQ(false, v2[0]) << "1 0 | 1 1";
        ASSERT_EQ(1, v1[1]) << "1 0 | 1 1";
        ASSERT_EQ(true, v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 | 1 1";
        ASSERT_EQ(0, v1[0]) << "0 1 | 1 1";
        ASSERT_EQ(true, v2[0]) << "0 1 | 1 1";
        ASSERT_EQ(1, v1[1]) << "0 1 | 1 1";
        ASSERT_EQ(true, v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 | 1 1";
        ASSERT_EQ(1, v1[0]) << "1 1 | 1 1";
        ASSERT_EQ(true, v2[0]) << "1 1 | 1 1";
        ASSERT_EQ(1, v1[1]) << "1 1 | 1 1";
        ASSERT_EQ(true, v2[1]) << "1 1 | 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsIntString) {
    std::vector<int> v1;
    std::vector<std::string> v2;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
        ASSERT_EQ(1, v1.size()) << "0 0";
        ASSERT_EQ(1, v2.size()) << "0 0";
        ASSERT_EQ(0, v1[0]) << "0 0";
        ASSERT_EQ("0", v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
        ASSERT_EQ(1, v1.size()) << "1 0";
        ASSERT_EQ(1, v2.size()) << "1 0";
        ASSERT_EQ(1, v1[0]) << "1 0";
        ASSERT_EQ("0", v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
        ASSERT_EQ(1, v1.size()) << "0 1";
        ASSERT_EQ(1, v2.size()) << "0 1";
        ASSERT_EQ(0, v1[0]) << "0 1";
        ASSERT_EQ("1", v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
        ASSERT_EQ(1, v1.size()) << "1 1";
        ASSERT_EQ(1, v2.size()) << "1 1";
        ASSERT_EQ(1, v1[0]) << "1 1";
        ASSERT_EQ("1", v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 | 0 0";
        ASSERT_EQ(0, v1[0]) << "0 0 | 0 0";
        ASSERT_EQ("0", v2[0]) << "0 0 | 0 0";
        ASSERT_EQ(0, v1[1]) << "0 0 | 0 0";
        ASSERT_EQ("0", v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 | 0 0";
        ASSERT_EQ(1, v1[0]) << "1 0 | 0 0";
        ASSERT_EQ("0", v2[0]) << "1 0 | 0 0";
        ASSERT_EQ(0, v1[1]) << "1 0 | 0 0";
        ASSERT_EQ("0", v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 | 0 0";
        ASSERT_EQ(0, v1[0]) << "0 1 | 0 0";
        ASSERT_EQ("1", v2[0]) << "0 1 | 0 0";
        ASSERT_EQ(0, v1[1]) << "0 1 | 0 0";
        ASSERT_EQ("0", v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 | 0 0";
        ASSERT_EQ(1, v1[0]) << "1 1 | 0 0";
        ASSERT_EQ("1", v2[0]) << "1 1 | 0 0";
        ASSERT_EQ(0, v1[1]) << "1 1 | 0 0";
        ASSERT_EQ("0", v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 | 1 1";
        ASSERT_EQ(0, v1[0]) << "0 0 | 1 1";
        ASSERT_EQ("0", v2[0]) << "0 0 | 1 1";
        ASSERT_EQ(1, v1[1]) << "0 0 | 1 1";
        ASSERT_EQ("1", v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 | 1 1";
        ASSERT_EQ(1, v1[0]) << "1 0 | 1 1";
        ASSERT_EQ("0", v2[0]) << "1 0 | 1 1";
        ASSERT_EQ(1, v1[1]) << "1 0 | 1 1";
        ASSERT_EQ("1", v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 | 1 1";
        ASSERT_EQ(0, v1[0]) << "0 1 | 1 1";
        ASSERT_EQ("1", v2[0]) << "0 1 | 1 1";
        ASSERT_EQ(1, v1[1]) << "0 1 | 1 1";
        ASSERT_EQ("1", v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 | 1 1";
        ASSERT_EQ(1, v1[0]) << "1 1 | 1 1";
        ASSERT_EQ("1", v2[0]) << "1 1 | 1 1";
        ASSERT_EQ(1, v1[1]) << "1 1 | 1 1";
        ASSERT_EQ("1", v2[1]) << "1 1 | 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandTwoArgumentsStringInt) {
    std::vector<std::string> v1;
    std::vector<int> v2;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0";
        ASSERT_EQ(1, v1.size()) << "0 0";
        ASSERT_EQ(1, v2.size()) << "0 0";
        ASSERT_EQ("0", v1[0]) << "0 0";
        ASSERT_EQ(0, v2[0]) << "0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0";
        ASSERT_EQ(1, v1.size()) << "1 0";
        ASSERT_EQ(1, v2.size()) << "1 0";
        ASSERT_EQ("1", v1[0]) << "1 0";
        ASSERT_EQ(0, v2[0]) << "1 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1";
        ASSERT_EQ(1, v1.size()) << "0 1";
        ASSERT_EQ(1, v2.size()) << "0 1";
        ASSERT_EQ("0", v1[0]) << "0 1";
        ASSERT_EQ(1, v2[0]) << "0 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(4, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1";
        ASSERT_EQ(1, v1.size()) << "1 1";
        ASSERT_EQ(1, v2.size()) << "1 1";
        ASSERT_EQ("1", v1[0]) << "1 1";
        ASSERT_EQ(1, v2[0]) << "1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 | 0 0";
        ASSERT_EQ("0", v1[0]) << "0 0 | 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 | 0 0";
        ASSERT_EQ("0", v1[1]) << "0 0 | 0 0";
        ASSERT_EQ(0, v2[1]) << "0 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 | 0 0";
        ASSERT_EQ("1", v1[0]) << "1 0 | 0 0";
        ASSERT_EQ(0, v2[0]) << "1 0 | 0 0";
        ASSERT_EQ("0", v1[1]) << "1 0 | 0 0";
        ASSERT_EQ(0, v2[1]) << "1 0 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 | 0 0";
        ASSERT_EQ("0", v1[0]) << "0 1 | 0 0";
        ASSERT_EQ(1, v2[0]) << "0 1 | 0 0";
        ASSERT_EQ("0", v1[1]) << "0 1 | 0 0";
        ASSERT_EQ(0, v2[1]) << "0 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "0",
            "0"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 | 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 | 0 0";
        ASSERT_EQ("1", v1[0]) << "1 1 | 0 0";
        ASSERT_EQ(1, v2[0]) << "1 1 | 0 0";
        ASSERT_EQ("0", v1[1]) << "1 1 | 0 0";
        ASSERT_EQ(0, v2[1]) << "1 1 | 0 0";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 | 1 1";
        ASSERT_EQ("0", v1[0]) << "0 0 | 1 1";
        ASSERT_EQ(0, v2[0]) << "0 0 | 1 1";
        ASSERT_EQ("1", v1[1]) << "0 0 | 1 1";
        ASSERT_EQ(1, v2[1]) << "0 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 | 1 1";
        ASSERT_EQ("1", v1[0]) << "1 0 | 1 1";
        ASSERT_EQ(0, v2[0]) << "1 0 | 1 1";
        ASSERT_EQ("1", v1[1]) << "1 0 | 1 1";
        ASSERT_EQ(1, v2[1]) << "1 0 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 | 1 1";
        ASSERT_EQ("0", v1[0]) << "0 1 | 1 1";
        ASSERT_EQ(1, v2[0]) << "0 1 | 1 1";
        ASSERT_EQ("1", v1[1]) << "0 1 | 1 1";
        ASSERT_EQ(1, v2[1]) << "0 1 | 1 1";
    }
    v1.clear();
    v2.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "-single",
            "1",
            "1"
        };

        _p->setCommandLine(7, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 | 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 | 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 | 1 1";
        ASSERT_EQ("1", v1[0]) << "1 1 | 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 | 1 1";
        ASSERT_EQ("1", v1[1]) << "1 1 | 1 1";
        ASSERT_EQ(1, v2[1]) << "1 1 | 1 1";
    }}

TEST_F(CommandlineParserTest, MultipleCommandThreeArgumentsBoolIntString) {
    std::vector<bool> v1;
    std::vector<int> v2;
    std::vector<std::string> v3;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0";
        ASSERT_EQ(1, v1.size()) << "0 0 0";
        ASSERT_EQ(1, v2.size()) << "0 0 0";
        ASSERT_EQ(1, v3.size()) << "0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 0";
        ASSERT_EQ("0", v3[0]) << "0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "0"
        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0";
        ASSERT_EQ(1, v1.size()) << "1 0 0";
        ASSERT_EQ(1, v2.size()) << "1 0 0";
        ASSERT_EQ(1, v3.size()) << "1 0 0";
        ASSERT_EQ(true, v1[0]);
        ASSERT_EQ(0, v2[0]);
        ASSERT_EQ("0", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "0"

        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0";
        ASSERT_EQ(1, v1.size()) << "0 1 0";
        ASSERT_EQ(1, v2.size()) << "0 1 0";
        ASSERT_EQ(1, v3.size()) << "0 1 0";
        ASSERT_EQ(false, v1[0]);
        ASSERT_EQ(1, v2[0]);
        ASSERT_EQ("0", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "0"

        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0";
        ASSERT_EQ(1, v1.size()) << "1 1 0";
        ASSERT_EQ(1, v2.size()) << "1 1 0";
        ASSERT_EQ(1, v3.size()) << "1 1 0";
        ASSERT_EQ(true, v1[0]);
        ASSERT_EQ(1, v2[0]);
        ASSERT_EQ("0", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "0",
            "1"
        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1";
        ASSERT_EQ(1, v1.size()) << "0 0 1";
        ASSERT_EQ(1, v2.size()) << "0 0 1";
        ASSERT_EQ(1, v3.size()) << "0 0 1";
        ASSERT_EQ(false, v1[0]);
        ASSERT_EQ(0, v2[0]);
        ASSERT_EQ("1", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "0",
            "1"
        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1";
        ASSERT_EQ(1, v1.size()) << "1 0 1";
        ASSERT_EQ(1, v2.size()) << "1 0 1";
        ASSERT_EQ(1, v3.size()) << "1 0 1";
        ASSERT_EQ(true, v1[0]);
        ASSERT_EQ(0, v2[0]);
        ASSERT_EQ("1", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "0",
            "1",
            "1"

        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1";
        ASSERT_EQ(1, v1.size()) << "0 1 1";
        ASSERT_EQ(1, v2.size()) << "0 1 1";
        ASSERT_EQ(1, v3.size()) << "0 1 1";
        ASSERT_EQ(false, v1[0]);
        ASSERT_EQ(1, v2[0]);
        ASSERT_EQ("1", v3[0]);
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
            "tests",
            "-single",
            "1",
            "1",
            "1"

        };

        _p->setCommandLine(5, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1";
        ASSERT_EQ(1, v1.size()) << "1 1 1";
        ASSERT_EQ(1, v2.size()) << "1 1 1";
        ASSERT_EQ(1, v3.size()) << "1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 1";
        ASSERT_EQ("1", v3[0]) << "1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0 | 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 0 | 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 0 | 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 0 0 | 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 0 0 | 0 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 0 | 0 0 0";
        ASSERT_EQ("0", v3[0]) << "0 0 0 | 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 0 0 | 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 0 0 | 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 0 0 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0 | 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 0 | 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 0 | 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 0 0 | 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 0 0 | 0 0 0";
        ASSERT_EQ(0, v2[0]) << "1 0 0 | 0 0 0";
        ASSERT_EQ("0", v3[0]) << "1 0 0 | 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 0 0 | 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 0 0 | 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 0 0 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0 | 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 0 | 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 0 | 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 1 0 | 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 1 0 | 0 0 0";
        ASSERT_EQ(1, v2[0]) << "0 1 0 | 0 0 0";
        ASSERT_EQ("0", v3[0]) << "0 1 0 | 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 1 0 | 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 1 0 | 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 1 0 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0 | 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 0 | 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 0 | 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 1 0 | 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 1 0 | 0 0 0";
        ASSERT_EQ(1, v2[0]) << "1 1 0 | 0 0 0";
        ASSERT_EQ("0", v3[0]) << "1 1 0 | 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 1 0 | 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 1 0 | 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 1 0 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1 | 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 1 | 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 1 | 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 0 1 | 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 0 1 | 0 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 1 | 0 0 0";
        ASSERT_EQ("1", v3[0]) << "0 0 1 | 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 0 1 | 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 0 1 | 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 0 1 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1 | 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 1 | 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 1 | 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 0 1 | 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 0 1 | 0 0 0";
        ASSERT_EQ(0, v2[0]) << "1 0 1 | 0 0 0";
        ASSERT_EQ("1", v3[0]) << "1 0 1 | 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 0 1 | 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 0 1 | 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 0 1 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1 | 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 1 | 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 1 | 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 1 1 | 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 1 1 | 0 0 0";
        ASSERT_EQ(1, v2[0]) << "0 1 1 | 0 0 0";
        ASSERT_EQ("1", v3[0]) << "0 1 1 | 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 1 1 | 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 1 1 | 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 1 1 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1 | 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 1 | 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 1 | 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 1 1 | 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 1 1 | 0 0 0";
        ASSERT_EQ(1, v2[0]) << "1 1 1 | 0 0 0";
        ASSERT_EQ("1", v3[0]) << "1 1 1 | 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 1 1 | 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 1 1 | 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 1 1 | 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0 | 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 0 | 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 0 | 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 0 0 | 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 0 0 | 1 1 1";
        ASSERT_EQ(0, v2[0]) << "0 0 0 | 1 1 1";
        ASSERT_EQ("0", v3[0]) << "0 0 0 | 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 0 0 | 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 0 0 | 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 0 0 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0 | 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 0 | 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 0 | 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 0 0 | 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 0 0 | 1 1 1";
        ASSERT_EQ(0, v2[0]) << "1 0 0 | 1 1 1";
        ASSERT_EQ("0", v3[0]) << "1 0 0 | 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 0 0 | 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 0 0 | 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 0 0 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0 | 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 0 | 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 0 | 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 1 0 | 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 1 0 | 1 1 1";
        ASSERT_EQ(1, v2[0]) << "0 1 0 | 1 1 1";
        ASSERT_EQ("0", v3[0]) << "0 1 0 | 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 1 0 | 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 1 0 | 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 1 0 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0 | 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 0 | 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 0 | 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 1 0 | 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 1 0 | 1 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 0 | 1 1 1";
        ASSERT_EQ("0", v3[0]) << "1 1 0 | 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 1 0 | 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 1 0 | 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 1 0 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1 | 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 1 | 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 1 | 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 0 1 | 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 0 1 | 1 1 1";
        ASSERT_EQ(0, v2[0]) << "0 0 1 | 1 1 1";
        ASSERT_EQ("1", v3[0]) << "0 0 1 | 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 0 1 | 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 0 1 | 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 0 1 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1 | 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 1 | 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 1 | 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 0 1 | 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 0 1 | 1 1 1";
        ASSERT_EQ(0, v2[0]) << "1 0 1 | 1 1 1";
        ASSERT_EQ("1", v3[0]) << "1 0 1 | 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 0 1 | 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 0 1 | 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 0 1 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1 | 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 1 | 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 1 | 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 1 1 | 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 1 1 | 1 1 1";
        ASSERT_EQ(1, v2[0]) << "0 1 1 | 1 1 1";
        ASSERT_EQ("1", v3[0]) << "0 1 1 | 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 1 1 | 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 1 1 | 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 1 1 | 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(9, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1 | 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 1 | 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 1 | 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 1 1 | 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 1 1 | 1 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 1 | 1 1 1";
        ASSERT_EQ("1", v3[0]) << "1 1 1 | 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 1 1 | 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 1 1 | 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 1 1 | 1 1 1";
    }
}

TEST_F(CommandlineParserTest, MultipleCommandFourArgumentsBoolIntStringFloat) {
    std::vector<bool> v1;
    std::vector<int> v2;
    std::vector<std::string> v3;
    std::vector<float> v4;
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0 0";
        ASSERT_EQ(1, v1.size()) << "0 0 0 0";
        ASSERT_EQ(1, v2.size()) << "0 0 0 0";
        ASSERT_EQ(1, v3.size()) << "0 0 0 0";
        ASSERT_EQ(1, v4.size()) << "0 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 0 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 0 0";
        ASSERT_EQ("0", v3[0]) << "0 0 0 0";
        ASSERT_EQ(0.f, v4[0]) << "0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0 0";
        ASSERT_EQ(1, v1.size()) << "1 0 0 0";
        ASSERT_EQ(1, v2.size()) << "1 0 0 0";
        ASSERT_EQ(1, v3.size()) << "1 0 0 0";
        ASSERT_EQ(1, v4.size()) << "1 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 0 0 0";
        ASSERT_EQ(0, v2[0]) << "1 0 0 0";
        ASSERT_EQ("0", v3[0]) << "1 0 0 0";
        ASSERT_EQ(0.f, v4[0]) << "1 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0 0";
        ASSERT_EQ(1, v1.size()) << "0 1 0 0";
        ASSERT_EQ(1, v2.size()) << "0 1 0 0";
        ASSERT_EQ(1, v3.size()) << "0 1 0 0";
        ASSERT_EQ(1, v4.size()) << "0 1 0 0";
        ASSERT_EQ(false, v1[0]) << "0 1 0 0";
        ASSERT_EQ(1, v2[0]) << "0 1 0 0";
        ASSERT_EQ("0", v3[0]) << "0 1 0 0";
        ASSERT_EQ(0.f, v4[0]) << "0 1 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0 0";
        ASSERT_EQ(1, v1.size()) << "1 1 0 0";
        ASSERT_EQ(1, v2.size()) << "1 1 0 0";
        ASSERT_EQ(1, v3.size()) << "1 1 0 0";
        ASSERT_EQ(1, v4.size()) << "1 1 0 0";
        ASSERT_EQ(true, v1[0]) << "1 1 0 0";
        ASSERT_EQ(1, v2[0]) << "1 1 0 0";
        ASSERT_EQ("0", v3[0]) << "1 1 0 0";
        ASSERT_EQ(0.f, v4[0]) << "1 1 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1 0";
        ASSERT_EQ(1, v1.size()) << "0 0 1 0";
        ASSERT_EQ(1, v2.size()) << "0 0 1 0";
        ASSERT_EQ(1, v3.size()) << "0 0 1 0";
        ASSERT_EQ(1, v4.size()) << "0 0 1 0";
        ASSERT_EQ(false, v1[0]) << "0 0 1 0";
        ASSERT_EQ(0, v2[0]) << "0 0 1 0";
        ASSERT_EQ("1", v3[0]) << "0 0 1 0";
        ASSERT_EQ(0.f, v4[0]) << "0 0 1 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1 0";
        ASSERT_EQ(1, v1.size()) << "1 0 1 0";
        ASSERT_EQ(1, v2.size()) << "1 0 1 0";
        ASSERT_EQ(1, v3.size()) << "1 0 1 0";
        ASSERT_EQ(1, v4.size()) << "1 0 1 0";
        ASSERT_EQ(true, v1[0]) << "1 0 1 0";
        ASSERT_EQ(0, v2[0]) << "1 0 1 0";
        ASSERT_EQ("1", v3[0]) << "1 0 1 0";
        ASSERT_EQ(0.f, v4[0]) << "1 0 1 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1 0";
        ASSERT_EQ(1, v1.size()) << "0 1 1 0";
        ASSERT_EQ(1, v2.size()) << "0 1 1 0";
        ASSERT_EQ(1, v3.size()) << "0 1 1 0";
        ASSERT_EQ(1, v4.size()) << "0 1 1 0";
        ASSERT_EQ(false, v1[0]) << "0 1 1 0";
        ASSERT_EQ(1, v2[0]) << "0 1 1 0";
        ASSERT_EQ("1", v3[0]) << "0 1 1 0";
        ASSERT_EQ(0.f, v4[0]) << "0 1 1 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1 0";
        ASSERT_EQ(1, v1.size()) << "1 1 1 0";
        ASSERT_EQ(1, v2.size()) << "1 1 1 0";
        ASSERT_EQ(1, v3.size()) << "1 1 1 0";
        ASSERT_EQ(1, v4.size()) << "1 1 1 0";
        ASSERT_EQ(true, v1[0]) << "1 1 1 0";
        ASSERT_EQ(1, v2[0]) << "1 1 1 0";
        ASSERT_EQ("1", v3[0]) << "1 1 1 0";
        ASSERT_EQ(0.f, v4[0]) << "1 1 1 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0 1";
        ASSERT_EQ(1, v1.size()) << "0 0 0 1";
        ASSERT_EQ(1, v2.size()) << "0 0 0 1";
        ASSERT_EQ(1, v3.size()) << "0 0 0 1";
        ASSERT_EQ(1, v4.size()) << "0 0 0 1";
        ASSERT_EQ(false, v1[0]) << "0 0 0 1";
        ASSERT_EQ(0, v2[0]) << "0 0 0 1";
        ASSERT_EQ("0", v3[0]) << "0 0 0 1";
        ASSERT_EQ(1.f, v4[0]) << "0 0 0 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0 1";
        ASSERT_EQ(1, v1.size()) << "1 0 0 1";
        ASSERT_EQ(1, v2.size()) << "1 0 0 1";
        ASSERT_EQ(1, v3.size()) << "1 0 0 1";
        ASSERT_EQ(1, v4.size()) << "1 0 0 1";
        ASSERT_EQ(true, v1[0]) << "1 0 0 1";
        ASSERT_EQ(0, v2[0]) << "1 0 0 1";
        ASSERT_EQ("0", v3[0]) << "1 0 0 1";
        ASSERT_EQ(1.f, v4[0]) << "1 0 0 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0 1";
        ASSERT_EQ(1, v1.size()) << "0 1 0 1";
        ASSERT_EQ(1, v2.size()) << "0 1 0 1";
        ASSERT_EQ(1, v3.size()) << "0 1 0 1";
        ASSERT_EQ(1, v4.size()) << "0 1 0 1";
        ASSERT_EQ(false, v1[0]) << "0 1 0 1";
        ASSERT_EQ(1, v2[0]) << "0 1 0 1";
        ASSERT_EQ("0", v3[0]) << "0 1 0 1";
        ASSERT_EQ(1.f, v4[0]) << "0 1 0 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0 1";
        ASSERT_EQ(1, v1.size()) << "1 1 0 1";
        ASSERT_EQ(1, v2.size()) << "1 1 0 1";
        ASSERT_EQ(1, v3.size()) << "1 1 0 1";
        ASSERT_EQ(1, v4.size()) << "1 1 0 1";
        ASSERT_EQ(true, v1[0]) << "1 1 0 1";
        ASSERT_EQ(1, v2[0]) << "1 1 0 1";
        ASSERT_EQ("0", v3[0]) << "1 1 0 1";
        ASSERT_EQ(1.f, v4[0]) << "1 1 0 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1 1";
        ASSERT_EQ(1, v1.size()) << "0 0 1 1";
        ASSERT_EQ(1, v2.size()) << "0 0 1 1";
        ASSERT_EQ(1, v3.size()) << "0 0 1 1";
        ASSERT_EQ(1, v4.size()) << "0 0 1 1";
        ASSERT_EQ(false, v1[0]) << "0 0 1 1";
        ASSERT_EQ(0, v2[0]) << "0 0 1 1";
        ASSERT_EQ("1", v3[0]) << "0 0 1 1";
        ASSERT_EQ(1.f, v4[0]) << "0 0 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1 1";
        ASSERT_EQ(1, v1.size()) << "1 0 1 1";
        ASSERT_EQ(1, v2.size()) << "1 0 1 1";
        ASSERT_EQ(1, v3.size()) << "1 0 1 1";
        ASSERT_EQ(1, v4.size()) << "1 0 1 1";
        ASSERT_EQ(true, v1[0]) << "1 0 1 1";
        ASSERT_EQ(0, v2[0]) << "1 0 1 1";
        ASSERT_EQ("1", v3[0]) << "1 0 1 1";
        ASSERT_EQ(1.f, v4[0]) << "1 0 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1 1";
        ASSERT_EQ(1, v1.size()) << "0 1 1 1";
        ASSERT_EQ(1, v2.size()) << "0 1 1 1";
        ASSERT_EQ(1, v3.size()) << "0 1 1 1";
        ASSERT_EQ(1, v4.size()) << "0 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 1 1 1";
        ASSERT_EQ(1, v2[0]) << "0 1 1 1";
        ASSERT_EQ("1", v3[0]) << "0 1 1 1";
        ASSERT_EQ(1.f, v4[0]) << "0 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
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
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1 1";
        ASSERT_EQ(1, v1.size()) << "1 1 1 1";
        ASSERT_EQ(1, v2.size()) << "1 1 1 1";
        ASSERT_EQ(1, v3.size()) << "1 1 1 1";
        ASSERT_EQ(1, v4.size()) << "1 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 1 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 1 1";
        ASSERT_EQ("1", v3[0]) << "1 1 1 1";
        ASSERT_EQ(1.f, v4[0]) << "1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[0]) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[0]) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 0 0 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "0 0 0 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[0]) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[0]) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[0]) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 0 0 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "1 0 0 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ(1, v2[0]) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[0]) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[0]) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 1 0 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "0 1 0 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ(1, v2[0]) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[0]) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[0]) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 1 0 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "1 1 0 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ("1", v3[0]) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[0]) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 0 1 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "0 0 1 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[0]) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ("1", v3[0]) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[0]) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 0 1 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "1 0 1 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ(1, v2[0]) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ("1", v3[0]) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[0]) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 1 1 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "0 1 1 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ(1, v2[0]) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ("1", v3[0]) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[0]) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 1 1 0 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "1 1 1 0 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[0]) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ(1.f, v4[0]) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 0 0 1 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "0 0 0 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[0]) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[0]) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ(1.f, v4[0]) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 0 0 1 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "1 0 0 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ(1, v2[0]) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[0]) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ(1.f, v4[0]) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 1 0 1 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "0 1 0 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ(1, v2[0]) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[0]) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ(1.f, v4[0]) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 1 0 1 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "1 1 0 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[0]) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ("1", v3[0]) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ(1.f, v4[0]) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 0 1 1 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "0 0 1 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[0]) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ("1", v3[0]) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ(1.f, v4[0]) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 0 1 1 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "1 0 1 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[0]) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ(1, v2[0]) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ("1", v3[0]) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ(1.f, v4[0]) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "0 1 1 1 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "0 1 1 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v1.size()) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v2.size()) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v3.size()) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ(2, v4.size()) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ(true, v1[0]) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ(1, v2[0]) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ("1", v3[0]) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ(1.f, v4[0]) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ(false, v1[1]) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ(0, v2[1]) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ("0", v3[1]) << "1 1 1 1 | 0 0 0 0";
        ASSERT_EQ(0.f, v4[1]) << "1 1 1 1 | 0 0 0 0";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ(0, v2[0]) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ("0", v3[0]) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ(0.f, v4[0]) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 0 0 0 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "0 0 0 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ(0, v2[0]) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ("0", v3[0]) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ(0.f, v4[0]) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 0 0 0 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "1 0 0 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[0]) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ("0", v3[0]) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ(0.f, v4[0]) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 1 0 0 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "0 1 0 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ("0", v3[0]) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ(0.f, v4[0]) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 1 0 0 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "1 1 0 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ(0, v2[0]) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[0]) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ(0.f, v4[0]) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 0 1 0 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "0 0 1 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ(0, v2[0]) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[0]) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ(0.f, v4[0]) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 0 1 0 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "1 0 1 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[0]) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[0]) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ(0.f, v4[0]) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 1 1 0 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "0 1 1 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[0]) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ(0.f, v4[0]) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 1 1 0 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "1 1 1 0 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ(0, v2[0]) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ("0", v3[0]) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[0]) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 0 0 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "0 0 0 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ(0, v2[0]) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ("0", v3[0]) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[0]) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 0 0 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "1 0 0 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[0]) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ("0", v3[0]) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[0]) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 1 0 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "0 1 0 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ("0", v3[0]) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[0]) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 1 0 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "1 1 0 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ(0, v2[0]) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[0]) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[0]) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 0 1 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "0 0 1 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ(0, v2[0]) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[0]) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[0]) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 0 1 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "1 0 1 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ(false, v1[0]) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[0]) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[0]) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[0]) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "0 1 1 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "0 1 1 1 | 1 1 1 1";
    }
    v1.clear();
    v2.clear();
    v3.clear();
    v4.clear();
    {
        char* argv[] = {
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

        _p->setCommandLine(11, argv);
        const bool res = _p->execute();
        ASSERT_EQ(true, res) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v1.size()) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v2.size()) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v3.size()) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ(2, v4.size()) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[0]) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[0]) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[0]) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[0]) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ(true, v1[1]) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ(1, v2[1]) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ("1", v3[1]) << "1 1 1 1 | 1 1 1 1";
        ASSERT_EQ(1.f, v4[1]) << "1 1 1 1 | 1 1 1 1";
    }
}

#ifdef __unix__
#pragma GCC diagnostic pop
#endif // __unix__
