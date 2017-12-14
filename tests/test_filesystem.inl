/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#include <ghoul/filesystem/filesystem>
#include <iostream>
#include <fstream>

#ifdef WIN32
#include <windows.h>
#endif

class FileSystemTest : public testing::Test {
protected:
    void SetUp() override {
        ghoul::filesystem::FileSystem::deinitialize();
        ghoul::filesystem::FileSystem::initialize();
    }
};

TEST_F(FileSystemTest, HasTestDirectory) {
    EXPECT_EQ(FileSys.directoryExists("${TEMPORARY}"), true);
}

TEST_F(FileSystemTest, CreateRemoveDirectory) {
    using ghoul::filesystem::FileSystem;

    const std::string tmp = absPath("${TEMPORARY}/tmp");
    const std::string tmpRecursive1 = absPath("${TEMPORARY}/tmp/tmp2");
    const std::string tmpRecursive2 = absPath("${TEMPORARY}/tmp/tmp2/tmp3");

    EXPECT_NO_THROW(FileSys.createDirectory(tmp));
    EXPECT_THROW(FileSys.createDirectory(tmpRecursive2), FileSystem::FileSystemException);
    EXPECT_NO_THROW(FileSys.createDirectory(tmpRecursive2, FileSystem::Recursive::Yes));

    EXPECT_THROW(FileSys.deleteDirectory(tmp), FileSystem::FileSystemException);
    EXPECT_NO_THROW(FileSys.deleteDirectory(tmpRecursive2));
    EXPECT_THROW(FileSys.deleteDirectory(tmp), FileSystem::FileSystemException);
    EXPECT_NO_THROW(FileSys.deleteDirectory(tmp, FileSystem::Recursive::Yes));
}

TEST_F(FileSystemTest, Path) {
    using ghoul::filesystem::File;

    std::string path = "${TEMPORARY}/tmpfil.txt";
    std::string abspath = absPath(path);

    File* f1 = new File(path);
    File* f2 = new File(path, File::RawPath::Yes);
    File* f3 = new File(abspath, File::RawPath::Yes);

    EXPECT_NE(f1->path(), f2->path());
    EXPECT_NE(f2->path(), f3->path());
    EXPECT_EQ(f1->path(), f3->path());

    delete f3;
    delete f2;
    delete f1;
}

TEST_F(FileSystemTest, OnChangeCallback) {
    using ghoul::filesystem::File;
    using ghoul::filesystem::FileSystem;

    const char* cpath = "${TEMPORARY}/tmpfil.txt";
    std::string path = absPath(cpath);
    std::ofstream f;
    f.open(path);
    f << "tmp";
    f.close();
    bool b1 = false;
    bool b2 = false;

    auto c1 = [&b1](const File&) {
        b1 = true;
    };
    auto c2 = [&b2](const File&) {
        b2 = true;
    };

    File* f1 = new File(path, File::RawPath::No, c1);
    File* f2 = new File(path, File::RawPath::No, c1);
    File* f3 = new File(path, File::RawPath::No, c2);

    // Check that the file exists
    EXPECT_EQ(FileSys.fileExists(absPath(cpath)), true);
    EXPECT_EQ(FileSys.fileExists(path), true);
    EXPECT_EQ(FileSys.fileExists(*f1), true);

    f2->setCallback(nullptr);

    // Check that b still is false so no callback has been fired
    EXPECT_EQ(b1, false);
    EXPECT_EQ(b2, false);

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
    EXPECT_EQ(b1, true);
    EXPECT_EQ(b2, true);

    delete f3;
    delete f2;
    delete f1;

    // Check that we can delete the file
    EXPECT_EQ(FileSys.deleteFile(path), true);
}

TEST_F(FileSystemTest, TokenDefaultState) {
    ASSERT_EQ(FileSys.tokens().size(), 1);
    ASSERT_EQ(FileSys.tokens()[0], "${TEMPORARY}");
}

TEST_F(FileSystemTest, OverrideNonExistingPathToken) {
    EXPECT_NO_THROW(
        FileSys.registerPathToken(
            "${AddExistingPathToken}",
            absPath("${TEMPORARY}"),
            ghoul::filesystem::FileSystem::Override::Yes
        )
    );
}

TEST_F(FileSystemTest, OverrideExistingPathToken) {
    FileSys.registerPathToken("${A}", "a");

    {
        std::string p = "${A}";
        FileSys.expandPathTokens(p);
        EXPECT_EQ(p, "a") << "Before";
    }

    EXPECT_NO_THROW(
        FileSys.registerPathToken(
            "${A}",
            "b",
            ghoul::filesystem::FileSystem::Override::Yes
        )
    );

    {
        std::string p = "${A}";
        FileSys.expandPathTokens(p);
        EXPECT_EQ(p, "b") << "After";
    }

    //EXPECT_THROW(
    //    FileSys.registerPathToken(
    //        "${A}",
    //        "b",
    //        ghoul::filesystem::FileSystem::Override::No
    //    ),

    //);
}

TEST_F(FileSystemTest, ExpandingTokensNonExistingToken) {
    std::string p = "${NOTFOUND}";
    EXPECT_THROW(
        FileSys.expandPathTokens(p),
        ghoul::filesystem::FileSystem::ResolveTokenException
    );
}

TEST_F(FileSystemTest, ExpandingTokens) {
    FileSys.registerPathToken(
        "${A}",
        "${B}/bar"
    );

    FileSys.registerPathToken(
        "${B}",
        "${C}/foo"
    );

    FileSys.registerPathToken(
        "${C}",
        "${D}/fob"
    );

    FileSys.registerPathToken(
        "${D}",
        "foobar"
    );

    {
        std::string p = "${D}";
        FileSys.expandPathTokens(p);
        EXPECT_EQ(p, "foobar") << "Single";
    }

    {
        std::string p = "${C}/fob";
        FileSys.expandPathTokens(p);
        EXPECT_EQ(p, "foobar/fob/fob") << "Recursive1";
    }

    {
        std::string p = "${B}/final";
        FileSys.expandPathTokens(p);
        EXPECT_EQ(p, "foobar/fob/foo/final") << "Recursive2";
    }

    {
        std::string p = "${A}/final";
        FileSys.expandPathTokens(p);
        EXPECT_EQ(p, "foobar/fob/foo/bar/final") << "Recursive3";
    }

    {
        std::string p = "initial/${A}";
        FileSys.expandPathTokens(p);
        EXPECT_EQ(p, "initial/foobar/fob/foo/bar") << "Noninitial";
    }

    {
        std::string p = "initial/${B}/barbar";
        FileSys.expandPathTokens(p);
        EXPECT_EQ(p, "initial/foobar/fob/foo/barbar") << "Middle";
    }

    {
        std::string p = "initial/${C}/middle/${B}/barbar";
        FileSys.expandPathTokens(p);
        EXPECT_EQ(p, "initial/foobar/fob/middle/foobar/fob/foo/barbar") << "Multiple1";
    }

    {
        std::string p = "${D}/a/${D}/b/${D}/c/${D}";
        FileSys.expandPathTokens(p);
        EXPECT_EQ(p, "foobar/a/foobar/b/foobar/c/foobar") << "Multiple2";
    }
}

TEST_F(FileSystemTest, ExpandingTokensIgnoredRegistered) {
    FileSys.registerPathToken(
        "${B}",
        "${C}/foo"
    );

    FileSys.registerPathToken(
        "${C}",
        "${D}/fob"
    );

    FileSys.registerPathToken(
        "${D}",
        "foobar"
    );

    {
        std::string p = "${D}";
        FileSys.expandPathTokens(p, { "${D}" });
        EXPECT_EQ(p, "${D}");
    }

    {
        std::string p = "${C}";
        FileSys.expandPathTokens(p, { "${D}" });
        EXPECT_EQ(p, "${D}/fob");
    }

    {
        std::string p = "${B}";
        FileSys.expandPathTokens(p, { "${D}" });
        EXPECT_EQ(p, "${D}/fob/foo");
    }

    {
        std::string p = "1/${D}/2";
        FileSys.expandPathTokens(p, { "${D}" });
        EXPECT_EQ(p, "1/${D}/2");
    }

    {
        std::string p = "1/${C}/2";
        FileSys.expandPathTokens(p, { "${D}" });
        EXPECT_EQ(p, "1/${D}/fob/2");
    }

    {
        std::string p = "1/${B}/2";
        FileSys.expandPathTokens(p, { "${D}" });
        EXPECT_EQ(p, "1/${D}/fob/foo/2");
    }
}

TEST_F(FileSystemTest, ExpandingTokensIgnoredUnregistered) {
    FileSys.registerPathToken(
        "${B}",
        "${C}/foo"
    );

    FileSys.registerPathToken(
        "${C}",
        "${D}/fob"
    );

    FileSys.registerPathToken(
        "${D}",
        "foobar"
    );

    {
        std::string p = "${X}";
        EXPECT_NO_THROW(FileSys.expandPathTokens(p, { "${X}" }));
        EXPECT_EQ(p, "${X}") << "Single";
    }

    {
        std::string p = "${D}/${X}/${C}";
        EXPECT_NO_THROW(FileSys.expandPathTokens(p, { "${X}" }));
        EXPECT_EQ(p, "foobar/${X}/foobar/fob") << "Single";
    }

    {
        std::string p = "${X}${Y}";
        EXPECT_NO_THROW(FileSys.expandPathTokens(p, { "${X}", "${Y}" }));
        EXPECT_EQ(p, "${X}${Y}") << "Multiple";
    }

    {
        std::string p = "${D}/${X}/${C}/${Y}/${B}";
        EXPECT_NO_THROW(FileSys.expandPathTokens(p, { "${X}", "${Y}" }));
        EXPECT_EQ(p, "foobar/${X}/foobar/fob/${Y}/foobar/fob/foo") << "Multiple";
    }
}