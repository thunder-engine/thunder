/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#include "tst_common.h"

#include "core/url.h"

class UrlTest : public ::testing::Test {

};

TEST_F(UrlTest, Parse_URL) {
    Url url("scheme://host/path/to/uri?query#fragment");

    ASSERT_TRUE(url.scheme() == String("scheme"));
    ASSERT_TRUE(url.host() == String("host"));
    ASSERT_TRUE(url.path() == String("/path/to/uri"));
    ASSERT_TRUE(url.query() == String("query"));
    ASSERT_TRUE(url.fragment() == String("fragment"));
    ASSERT_TRUE(url.name() == String("uri"));
}

TEST_F(UrlTest, Parse_WinPath) {
    Url url("C:\\host\\path\\to\\uri.tar.gz");

    ASSERT_TRUE(url.path() == String("/host/path/to/uri.tar.gz"));
    ASSERT_TRUE(url.dir() == String("/host/path/to"));
    ASSERT_TRUE(url.name() == String("uri.tar.gz"));
    ASSERT_TRUE(url.baseName() == String("uri"));
    ASSERT_TRUE(url.suffix() == String("tar.gz"));
}

TEST_F(UrlTest, Parse_UnixPath) {
    Url url("/host/path/to/uri.tar.gz");

    ASSERT_TRUE(url.path() == String("/host/path/to/uri.tar.gz"));
    ASSERT_TRUE(url.dir() == String("/host/path/to"));
    ASSERT_TRUE(url.name() == String("uri.tar.gz"));
    ASSERT_TRUE(url.baseName() == String("uri"));
    ASSERT_TRUE(url.suffix() == String("tar.gz"));
}
