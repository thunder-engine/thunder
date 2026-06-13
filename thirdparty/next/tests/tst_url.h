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
        ASSERT_TRUE(url.path() == TString("simple"));
        ASSERT_TRUE(url.dir() == TString(""));
        ASSERT_TRUE(url.name() == TString("simple"));
    }

    TEST_F(UrlTest, Parse_URL) {
        Url url("scheme://host/path/to/uri?query#fragment");

        ASSERT_TRUE(url.scheme() == TString("scheme"));
        ASSERT_TRUE(url.host() == TString("host"));
        ASSERT_TRUE(url.path() == TString("/path/to/uri"));
        ASSERT_TRUE(url.dir() == TString("/path/to"));
        ASSERT_TRUE(url.query() == TString("query"));
        ASSERT_TRUE(url.fragment() == TString("fragment"));
        ASSERT_TRUE(url.name() == TString("uri"));
    }

    TEST_F(UrlTest, Parse_WinPath) {
        Url url("C:\\host\\path\\to\\uri.tar.gz");

        ASSERT_TRUE(url.path() == TString("/host/path/to/uri.tar.gz"));
        ASSERT_TRUE(url.dir() == TString("/host/path/to"));
        ASSERT_TRUE(url.name() == TString("uri.tar.gz"));
        ASSERT_TRUE(url.baseName() == TString("uri"));
        ASSERT_TRUE(url.suffix() == TString("gz"));
        ASSERT_TRUE(url.completeSuffix() == TString("tar.gz"));
    }

    TEST_F(UrlTest, Parse_UnixPath) {
        Url url("/host/path/to/uri.tar.gz");

        ASSERT_TRUE(url.path() == TString("/host/path/to/uri.tar.gz"));
        ASSERT_TRUE(url.dir() == TString("/host/path/to"));
        ASSERT_TRUE(url.name() == TString("uri.tar.gz"));
        ASSERT_TRUE(url.baseName() == TString("uri"));
        ASSERT_TRUE(url.suffix() == TString("gz"));
        ASSERT_TRUE(url.completeSuffix() == TString("tar.gz"));
    }
}
