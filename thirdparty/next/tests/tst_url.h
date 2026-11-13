/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "tst_common.h"

#include "core/url.h"

namespace NextSuite {
class UrlTest : public ::testing::Test {

};

TEST_F(UrlTest, Parse_Simple) {
    Url url("simple");

    ASSERT_TRUE(url.scheme() == TString(""));
    ASSERT_TRUE(url.host() == TString(""));
    ASSERT_TRUE(url.filePath() == TString("simple"));
    ASSERT_TRUE(url.dir() == TString(""));
    ASSERT_TRUE(url.name() == TString("simple"));
}

TEST_F(UrlTest, Parse_URL) {
    Url url("scheme://host/path/to/uri?query#fragment");

    ASSERT_TRUE(url.scheme() == TString("scheme"));
    ASSERT_TRUE(url.host() == TString("host"));
    ASSERT_TRUE(url.filePath() == TString("/path/to/uri"));
    ASSERT_TRUE(url.dir() == TString("/path/to"));
    ASSERT_TRUE(url.query() == TString("query"));
    ASSERT_TRUE(url.fragment() == TString("fragment"));
    ASSERT_TRUE(url.name() == TString("uri"));
}

TEST_F(UrlTest, Parse_WinPath) {
    Url url("C:\\host\\path\\to\\uri.tar.gz");

    ASSERT_TRUE(url.filePath() == TString("/host/path/to/uri.tar.gz"));
    ASSERT_TRUE(url.dir() == TString("/host/path/to"));
    ASSERT_TRUE(url.name() == TString("uri.tar.gz"));
    ASSERT_TRUE(url.baseName() == TString("uri"));
    ASSERT_TRUE(url.suffix() == TString("gz"));
    ASSERT_TRUE(url.completeSuffix() == TString("tar.gz"));
}

TEST_F(UrlTest, Parse_UnixPath) {
    Url url("/host/path/to/uri.tar.gz");

    ASSERT_TRUE(url.filePath() == TString("/host/path/to/uri.tar.gz"));
    ASSERT_TRUE(url.dir() == TString("/host/path/to"));
    ASSERT_TRUE(url.name() == TString("uri.tar.gz"));
    ASSERT_TRUE(url.baseName() == TString("uri"));
    ASSERT_TRUE(url.suffix() == TString("gz"));
    ASSERT_TRUE(url.completeSuffix() == TString("tar.gz"));
}

// Тесты для метода relativeDir
TEST_F(UrlTest, RelativeDir_SamePath) {
    Url url("file:///home/user/projects/main.cpp");
    TString base = "/home/user/projects/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("."));
}

TEST_F(UrlTest, RelativeDir_Subdirectory) {
    Url url("file:///home/user/projects/subdir/file.txt");
    TString base = "/home/user/projects/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("subdir/"));
}

TEST_F(UrlTest, RelativeDir_ParentDirectory) {
    Url url("file:///home/user/other/file.txt");
    TString base = "/home/user/projects/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("../other/"));
}

TEST_F(UrlTest, RelativeDir_MultipleParentDirectories) {
    Url url("file:///home/other/file.txt");
    TString base = "/home/user/projects/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("../../other/"));
}

TEST_F(UrlTest, RelativeDir_NoCommonPrefix) {
    Url url("file:///var/www/html/index.html");
    TString base = "/home/user/projects/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("/var/www/html/"));
}

TEST_F(UrlTest, RelativeDir_EmptyBase) {
    Url url("file:///home/user/projects/file.txt");
    TString base = "";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("/home/user/projects/"));
}

TEST_F(UrlTest, RelativeDir_WindowsSeparators) {
    Url url("file://C:/Users/John/Documents/file.txt");
    TString base = "C:\\Users\\John\\";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("Documents/"));
}

TEST_F(UrlTest, RelativeDir_DeepNesting) {
    Url url("file:///home/user/projects/src/core/engine.cpp");
    TString base = "/home/user/projects/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("src/core/"));
}

TEST_F(UrlTest, RelativeDir_BaseWithoutTrailingSlash) {
    Url url("file:///home/user/projects/subdir/file.txt");
    TString base = "/home/user/projects";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("subdir/"));
}

TEST_F(UrlTest, RelativeDir_UrlWithTrailingSlash) {
    Url url("file:///home/user/projects/subdir/");
    TString base = "/home/user/projects/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("subdir/"));
}

TEST_F(UrlTest, RelativeDir_WithHostAndScheme) {
    Url url("https://example.com/home/user/projects/file.txt");
    TString base = "/home/user/projects/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("."));
}

TEST_F(UrlTest, RelativeDir_RootBase) {
    Url url("file:///home/user/projects/file.txt");
    TString base = "/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("home/user/projects/"));
}

TEST_F(UrlTest, RelativeDir_SamePrefixDifferentRoots) {
    Url url("file:///home/user/projects/file.txt");
    TString base = "/home/user/work/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("../projects/"));
}

TEST_F(UrlTest, RelativeDir_Normalization) {
    Url url("file:///home/user/projects//subdir///file.txt");
    TString base = "/home/user/projects/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("subdir/"));
}

TEST_F(UrlTest, RelativeDir_HttpUrl) {
    Url url("http://localhost:8080/api/v1/users");
    TString base = "/api/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("v1/"));
}

TEST_F(UrlTest, RelativeDir_DifferentSchemes) {
    Url url("https://example.com/path/to/file.txt");
    TString base = "/path/to/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("."));
}

TEST_F(UrlTest, RelativeDir_UnixAbsolutePath) {
    Url url("/usr/local/bin/script.sh");
    TString base = "/usr/local/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("bin/"));
}

TEST_F(UrlTest, RelativeDir_NoCommonPrefixWithRoot) {
    Url url("file:///home/user/file.txt");
    TString base = "/var/";

    TString result = url.relativeDir(base);
    ASSERT_TRUE(result == TString("/home/user/"));
}

TEST_F(UrlTest, RelativeFilePath_WindowsPath) {
    Url url("D:/Projects/thunder-samples/2d/flappy/content/Level.map");
    TString base = "D:/Projects/thunder-samples/2d/flappy/content";

    TString result = url.relativeFilePath(base);
    ASSERT_TRUE(result == TString("Level.map"));
}

// --- cdUp ---

TEST_F(UrlTest, CdUp_SimplePath) {
    Url url("/home/user/projects/file.txt");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/home/user/projects/"));
}

TEST_F(UrlTest, CdUp_TrailingSlash) {
    Url url("/home/user/projects/");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/home/user/"));
}

TEST_F(UrlTest, CdUp_RelativePath) {
    Url url("a/b/c");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("a/b/"));
}

TEST_F(UrlTest, CdUp_RelativeSingleLevel) {
    Url url("a/b");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("a/"));
}

TEST_F(UrlTest, CdUp_RelativeNoSlash_ReturnsFalse) {
    Url url("file.txt");

    ASSERT_FALSE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("file.txt"));
}

TEST_F(UrlTest, CdUp_RootPath_ReturnsFalse) {
    Url url("/");

    ASSERT_FALSE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/"));
}

TEST_F(UrlTest, CdUp_TopLevelAbsolute) {
    Url url("/home");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/"));
}

TEST_F(UrlTest, CdUp_EmptyPath_ReturnsFalse) {
    Url url("");

    ASSERT_FALSE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString(""));
}

TEST_F(UrlTest, CdUp_PreservesSchemeAndHost) {
    Url url("https://example.com/a/b/c?x=1#frag");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.scheme()   == TString("https"));
    ASSERT_TRUE(url.host()     == TString("example.com"));
    ASSERT_TRUE(url.filePath() == TString("/a/b/"));
    ASSERT_TRUE(url.query()    == TString("x=1"));
    ASSERT_TRUE(url.fragment() == TString("frag"));
}

TEST_F(UrlTest, CdUp_FileUrl) {
    Url url("file:///home/user/projects/file.txt");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.scheme()   == TString("file"));
    ASSERT_TRUE(url.filePath() == TString("/home/user/projects/"));
}

TEST_F(UrlTest, CdUp_WindowsSeparators) {
    Url url("C:\\Users\\John\\Documents\\file.txt");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/Users/John/Documents/"));
}

TEST_F(UrlTest, CdUp_MultipleTimes) {
    Url url("/a/b/c/d/file.txt");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/a/b/c/d/"));

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/a/b/c/"));

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/a/b/"));

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/a/"));

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/"));

    ASSERT_FALSE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/"));
}

TEST_F(UrlTest, CdUp_DotPath_ReturnsFalse) {
    Url url(".");

    ASSERT_FALSE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("."));
}

TEST_F(UrlTest, CdUp_DotDotPath_ReturnsFalse) {
    Url url("..");

    ASSERT_FALSE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString(".."));
}

TEST_F(UrlTest, CdUp_ThenQueryStillAccessible) {
    Url url("/a/b/c?key=value");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.filePath() == TString("/a/b/"));
    ASSERT_TRUE(url.query()    == TString("key=value"));
}

TEST_F(UrlTest, CdUp_ThenNameIsEmpty) {
    Url url("/a/b/c");

    ASSERT_TRUE(url.cdUp());
    ASSERT_TRUE(url.name() == TString(""));
}
}
