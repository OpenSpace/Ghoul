/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#include <ghoul/misc/csvreader.h>

class CSVReaderTest : public testing::Test {
protected:
    void SetUp() override {

        // This is not necessary to do every SetUp phase, but I don't know of a better
        // way to do it once per fixture ---abock
        _test0 = constants::TestDirectory + "/csvreader/test0.csv";
        //_test1 = constants::TestDirectory + "/csvreader/test1.csv";
    }

    std::string _test0;
    std::string _test1;
};

TEST_F(CSVReaderTest, Initial) {
    std::vector<std::vector<std::string>> t0 = ghoul::loadCSVFile(_test0);
}

TEST_F(CSVReaderTest, FullLoad) {
    std::vector<std::vector<std::string>> noHeader = ghoul::loadCSVFile(_test0);
    std::vector<std::vector<std::string>> header = ghoul::loadCSVFile(_test0, true);
    
    ASSERT_EQ(351, noHeader.size());
    ASSERT_EQ(352, header.size());

    for (size_t i = 1; i < header.size(); ++i) {
        EXPECT_EQ(19, noHeader[i - 1].size());
        EXPECT_EQ(19, header[i].size());
        ASSERT_EQ(noHeader[i - 1], header[i]);
    }

    EXPECT_EQ("flora", header[1][2]);
    EXPECT_EQ("303.82211", header[93][10]);
    EXPECT_EQ("0.19090799", header[177][13]);
    EXPECT_EQ("eduarda", header[283][2]);
}

TEST_F(CSVReaderTest, ColumnNumber) {
    std::vector<int> col = { 1, 2, 4, 6 };
    std::vector<std::vector<std::string>> noHeader = ghoul::loadCSVFile(_test0, col);
    std::vector<std::vector<std::string>> header = ghoul::loadCSVFile(_test0, col, true);

    ASSERT_EQ(351, noHeader.size());
    ASSERT_EQ(352, header.size());

    for (size_t i = 1; i < header.size(); ++i) {
        EXPECT_EQ(4, noHeader[i - 1].size());
        EXPECT_EQ(4, header[i].size());
        ASSERT_EQ(noHeader[i - 1], header[i]);
    }

    EXPECT_EQ("106", header[73][0]);
    EXPECT_EQ("279", header[227][0]);
    EXPECT_EQ("0.15", header[65][2]);
    EXPECT_EQ("53849", header[65][3]);
}

TEST_F(CSVReaderTest, ColumnName) {
    //   4, 7, 2
    std::vector<std::string> col = { "slope_g", "obs_num", "designation" };
    std::vector<std::vector<std::string>> noHeader = ghoul::loadCSVFile(_test0, col);
    std::vector<std::vector<std::string>> header = ghoul::loadCSVFile(_test0, col, true);

    ASSERT_EQ(351, noHeader.size());
    ASSERT_EQ(352, header.size());

    for (size_t i = 1; i < header.size(); ++i) {
        EXPECT_EQ(3, noHeader[i - 1].size());
        EXPECT_EQ(3, header[i].size());
        ASSERT_EQ(noHeader[i - 1], header[i]);
    }

    EXPECT_EQ("0.15", header[84][0]);
    EXPECT_EQ("2142", header[261][1]);
    EXPECT_EQ("peitho", header[84][2]);
}
