#include "tst_common.h"

#include <webrequest.h>

class NetworkTest : public ::testing::Test {

};

TEST_F(NetworkTest, Basic_Http_Check) {
    WebRequest *request = WebRequest::get("http://thunderengine.org/");
    if(request) {
        request->send();
        while(!request->isDone()) {

        }
        ASSERT_TRUE(request->errorCode() == 200);
    }
}

TEST_F(NetworkTest, Basic_Https_Check) {
    WebRequest *request = WebRequest::get("https://thunderengine.org/");
    if(request) {
        request->send();
        while(!request->isDone()) {

        }
        ASSERT_TRUE(request->errorCode() == 200);
    }
}
