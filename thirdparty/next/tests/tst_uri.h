#include "tst_common.h"

#include "core/uri.h"

class UriTest : public ::testing::Test {

};

TEST_F(UriTest, Parse_URI) {
    Uri uri("scheme://host/path/to/uri?query#fragment");

    ASSERT_TRUE(uri.scheme() == std::string("scheme"));
    ASSERT_TRUE(uri.host() == std::string("host"));
    ASSERT_TRUE(uri.path() == std::string("/path/to/uri"));
    ASSERT_TRUE(uri.query() == std::string("query"));
    ASSERT_TRUE(uri.fragment() == std::string("fragment"));
    ASSERT_TRUE(uri.name() == std::string("uri"));
}

TEST_F(UriTest, Parse_Path) {
    {
        Uri uri("C:\\host\\path\\to\\uri.tar.gz");

        ASSERT_TRUE(uri.path() == std::string("/host/path/to/uri.tar.gz"));
        ASSERT_TRUE(uri.dir() == std::string("/host/path/to"));
        ASSERT_TRUE(uri.name() == std::string("uri.tar.gz"));
        ASSERT_TRUE(uri.baseName() == std::string("uri"));
        ASSERT_TRUE(uri.suffix() == std::string("tar.gz"));
    }
    {
        Uri uri("/host/path/to/uri.tar.gz");

        ASSERT_TRUE(uri.path() == std::string("/host/path/to/uri.tar.gz"));
        ASSERT_TRUE(uri.dir() == std::string("/host/path/to"));
        ASSERT_TRUE(uri.name() == std::string("uri.tar.gz"));
        ASSERT_TRUE(uri.baseName() == std::string("uri"));
        ASSERT_TRUE(uri.suffix() == std::string("tar.gz"));
    }
}
