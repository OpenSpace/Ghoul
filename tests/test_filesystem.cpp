/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2022                                                               *
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

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/misc/exception.h>
#include <iostream>
#include <filesystem>
#include <fstream>

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

TEST_CASE("FileSystem: OnChangeCallback", "[filesystem]") {
    using ghoul::filesystem::File;
    using ghoul::filesystem::FileSystem;

    const char* cpath = "${TEMPORARY}/tmpfil.txt";
    std::filesystem::path path = absPath(cpath);
    std::ofstream f(path);
    f.open(path);
    f << "tmp";
    f.close();
    bool b1 = false;
    bool b2 = false;

    auto c1 = [&b1]() { b1 = true; };
    auto c2 = [&b2]() { b2 = true; };

    File* f1 = new File(path);
    f1->setCallback(c1);
    File* f2 = new File(path);
    f2->setCallback(c1);
    File* f3 = new File(path);
    f3->setCallback(c2);

    // Check that the file exists
    REQUIRE(std::filesystem::is_regular_file(absPath(cpath)));
    REQUIRE(std::filesystem::is_regular_file(path));
    REQUIRE(std::filesystem::is_regular_file(f1->path()));

    f2->setCallback(nullptr);

    // Check that b still is false so no callback has been fired
    REQUIRE_FALSE(b1);
    REQUIRE_FALSE(b2);

    // overwrite the file
    f.open(path);
    f << "tmp";
    f.close();
    FileSys.triggerFilesystemEvents();

    // Sleep the main thread to make sure the filesystem have time to respond
    const int seconds = 4;
#ifdef WIN32
    int count = 0;
    while ((b1 == false || b2 == false) && count < 100 * seconds) {
        Sleep(10);
        ++count;
    }
#else
    int count = 0;
    while ((b1 == false || b2 == false) && count < 10000 * seconds) {
        usleep(100);
        FileSys.triggerFilesystemEvents();
        ++count;
    }
#endif
    REQUIRE(b1);
    REQUIRE(b2);

    delete f3;
    delete f2;
    delete f1;

    // Check that we can delete the file
    REQUIRE(std::filesystem::remove(path));
}

TEST_CASE("FileSystem: TokenDefaultState", "[filesystem]") {
    REQUIRE(FileSys.tokens().size() == 3);
    REQUIRE(FileSys.tokens()[0] == "${TEMPORARY}");
    REQUIRE(FileSys.tokens()[1] == "${UNIT_SCRIPT}");
    REQUIRE(FileSys.tokens()[2] == "${UNIT_TEST}");
}

TEST_CASE("FileSystem: Override Non Existing Path Token", "[filesystem]") {
    // @TODO (abock, 2019-12-29) This test needs to be rewritten to not mess with the
    // global state of other tests
    return;

    REQUIRE_NOTHROW(
        FileSys.registerPathToken(
            "${AddExistingPathToken}",
            absPath("${TEMPORARY}"),
            ghoul::filesystem::FileSystem::Override::Yes
        )
    );
}

TEST_CASE("FileSystem: OverrideExistingPathToken", "[filesystem]") {
    // @TODO (abock, 2019-12-29) This test needs to be rewritten to not mess with the
    // global state of other tests

    //FileSys.registerPathToken("${A}", "a");

    //{
    //    std::string p = "${A}";
    //    FileSys.expandPathTokens(p);
    //    REQUIRE(p == "a");
    //}

    //REQUIRE_NOTHROW(
    //    FileSys.registerPathToken(
    //        "${A}",
    //        "b",
    //        ghoul::filesystem::FileSystem::Override::Yes
    //    )
    //);

    //{
    //    std::string p = "${A}";
    //    FileSys.expandPathTokens(p);
    //    REQUIRE(p == "b");
    //}
}

TEST_CASE("FileSystem: ExpandingTokensNonExistingToken", "[filesystem]") {
    std::string p = "${NOTFOUND}";
    REQUIRE_THROWS_AS(FileSys.expandPathTokens(p), ghoul::RuntimeError);
}

TEST_CASE("FileSystem: ExpandingTokens", "[filesystem]") {
    // @TODO (abock, 2019-12-29) This test needs to be rewritten to not mess with the
    // global state of other tests

    //FileSys.registerPathToken("${A}", "${B}/bar");
    //FileSys.registerPathToken("${B}", "${C}/foo");
    //FileSys.registerPathToken("${C}", "${D}/fob");
    //FileSys.registerPathToken("${D}", "foobar");

    //{
    //    std::string p = "${D}";
    //    FileSys.expandPathTokens(p);
    //    REQUIRE(p == "foobar");
    //}

    //{
    //    std::string p = "${C}/fob";
    //    FileSys.expandPathTokens(p);
    //    REQUIRE(p == "foobar/fob/fob");
    //}

    //{
    //    std::string p = "${B}/final";
    //    FileSys.expandPathTokens(p);
    //    REQUIRE(p == "foobar/fob/foo/final");
    //}

    //{
    //    std::string p = "${A}/final";
    //    FileSys.expandPathTokens(p);
    //    REQUIRE(p == "foobar/fob/foo/bar/final");
    //}

    //{
    //    std::string p = "initial/${A}";
    //    FileSys.expandPathTokens(p);
    //    REQUIRE(p == "initial/foobar/fob/foo/bar");
    //}

    //{
    //    std::string p = "initial/${B}/barbar";
    //    FileSys.expandPathTokens(p);
    //    REQUIRE(p == "initial/foobar/fob/foo/barbar");
    //}

    //{
    //    std::string p = "initial/${C}/middle/${B}/barbar";
    //    FileSys.expandPathTokens(p);
    //    REQUIRE(p == "initial/foobar/fob/middle/foobar/fob/foo/barbar");
    //}

    //{
    //    std::string p = "${D}/a/${D}/b/${D}/c/${D}";
    //    FileSys.expandPathTokens(p);
    //    REQUIRE(p == "foobar/a/foobar/b/foobar/c/foobar");
    //}
}

TEST_CASE("FileSystem: ExpandingTokensIgnoredRegistered", "[filesystem]") {
    // @TODO (abock, 2019-12-29) This test needs to be rewritten to not mess with the
    // global state of other tests

    //FileSys.registerPathToken("${B}", "${C}/foo");
    //FileSys.registerPathToken("${C}", "${D}/fob");
    //FileSys.registerPathToken("${D}", "foobar");

    //{
    //    std::string p = "${D}";
    //    FileSys.expandPathTokens(p, { "${D}" });
    //    REQUIRE(p == "${D}");
    //}

    //{
    //    std::string p = "${C}";
    //    FileSys.expandPathTokens(p, { "${D}" });
    //    REQUIRE(p == "${D}/fob");
    //}

    //{
    //    std::string p = "${B}";
    //    FileSys.expandPathTokens(p, { "${D}" });
    //    REQUIRE(p == "${D}/fob/foo");
    //}

    //{
    //    std::string p = "1/${D}/2";
    //    FileSys.expandPathTokens(p, { "${D}" });
    //    REQUIRE(p == "1/${D}/2");
    //}

    //{
    //    std::string p = "1/${C}/2";
    //    FileSys.expandPathTokens(p, { "${D}" });
    //    REQUIRE(p == "1/${D}/fob/2");
    //}

    //{
    //    std::string p = "1/${B}/2";
    //    FileSys.expandPathTokens(p, { "${D}" });
    //    REQUIRE(p == "1/${D}/fob/foo/2");
    //}
}

TEST_CASE("FileSystem: Expanding Tokens Ignored Unregistered", "[filesystem]") {
    // @TODO (abock, 2019-12-29) This test needs to be rewritten to not mess with the
    // global state of other tests

    //FileSys.registerPathToken("${B}", "${C}/foo");
    //FileSys.registerPathToken("${C}", "${D}/fob");
    //FileSys.registerPathToken("${D}", "foobar");

    //{
    //    std::string p = "${X}";
    //    REQUIRE_NOTHROW(FileSys.expandPathTokens(p, { "${X}" }));
    //    REQUIRE(p == "${X}");
    //}

    //{
    //    std::string p = "${D}/${X}/${C}";
    //    REQUIRE_NOTHROW(FileSys.expandPathTokens(p, { "${X}" }));
    //    REQUIRE(p == "foobar/${X}/foobar/fob");
    //}

    //{
    //    std::string p = "${X}${Y}";
    //    REQUIRE_NOTHROW(FileSys.expandPathTokens(p, { "${X}", "${Y}" }));
    //    REQUIRE(p == "${X}${Y}");
    //}

    //{
    //    std::string p = "${D}/${X}/${C}/${Y}/${B}";
    //    REQUIRE_NOTHROW(FileSys.expandPathTokens(p, { "${X}", "${Y}" }));
    //    REQUIRE(p == "foobar/${X}/foobar/fob/${Y}/foobar/fob/foo");
    //}
}
