#include "tst_common.h"

#include "core/uri.h"

class UriTest : public ::testing::Test {

};

TEST_F(UriTest, Parse_URI) {
    Uri uri("scheme://host/path/to/uri?query#fragment");

    ASSERT_TRUE(uri.scheme() == string("scheme"));
    ASSERT_TRUE(uri.host() == string("host"));
    ASSERT_TRUE(uri.path() == string("/path/to/uri"));
    ASSERT_TRUE(uri.query() == string("query"));
    ASSERT_TRUE(uri.fragment() == string("fragment"));
    ASSERT_TRUE(uri.name() == string("uri"));
}

TEST_F(UriTest, Parse_Path) {
    {
        Uri uri("C:\\host\\path\\to\\uri.tar.gz");

        ASSERT_TRUE(uri.path() == string("/host/path/to/uri.tar.gz"));
        ASSERT_TRUE(uri.dir() == string("/host/path/to"));
        ASSERT_TRUE(uri.name() == string("uri.tar.gz"));
        ASSERT_TRUE(uri.baseName() == string("uri"));
        ASSERT_TRUE(uri.suffix() == string("tar.gz"));
    }
    {
        Uri uri("/host/path/to/uri.tar.gz");

        ASSERT_TRUE(uri.path() == string("/host/path/to/uri.tar.gz"));
        ASSERT_TRUE(uri.dir() == string("/host/path/to"));
        ASSERT_TRUE(uri.name() == string("uri.tar.gz"));
        ASSERT_TRUE(uri.baseName() == string("uri"));
        ASSERT_TRUE(uri.suffix() == string("tar.gz"));
    }
}
