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

    ASSERT_TRUE(url.scheme() == std::string("scheme"));
    ASSERT_TRUE(url.host() == std::string("host"));
    ASSERT_TRUE(url.path() == std::string("/path/to/uri"));
    ASSERT_TRUE(url.query() == std::string("query"));
    ASSERT_TRUE(url.fragment() == std::string("fragment"));
    ASSERT_TRUE(url.name() == std::string("uri"));
}

TEST_F(UrlTest, Parse_Path) {
    {
        Url url("C:\\host\\path\\to\\uri.tar.gz");

        ASSERT_TRUE(url.path() == std::string("/host/path/to/uri.tar.gz"));
        ASSERT_TRUE(url.dir() == std::string("/host/path/to"));
        ASSERT_TRUE(url.name() == std::string("uri.tar.gz"));
        ASSERT_TRUE(url.baseName() == std::string("uri"));
        ASSERT_TRUE(url.suffix() == std::string("tar.gz"));
    }
    {
        Url url("/host/path/to/uri.tar.gz");

        ASSERT_TRUE(url.path() == std::string("/host/path/to/uri.tar.gz"));
        ASSERT_TRUE(url.dir() == std::string("/host/path/to"));
        ASSERT_TRUE(url.name() == std::string("uri.tar.gz"));
        ASSERT_TRUE(url.baseName() == std::string("uri"));
        ASSERT_TRUE(url.suffix() == std::string("tar.gz"));
    }
}
