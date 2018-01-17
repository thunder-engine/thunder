#include "core/auri.h"

#include <regex>

class AUriPrivate {
public:
    string                  mUri;

    smatch                  mResult;
};

AUri::AUri(const string &uri) :
        p_ptr(new AUriPrivate) {
    PROFILE_FUNCTION()
    p_ptr->mUri = uri;
    regex_match(p_ptr->mUri, p_ptr->mResult, regex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"));
}

string AUri::scheme() const {
    PROFILE_FUNCTION()
    return p_ptr->mResult[2].str();
}

string AUri::host() const {
    PROFILE_FUNCTION()
    return p_ptr->mResult[4];
}

string AUri::path() const {
    PROFILE_FUNCTION()
    return p_ptr->mResult[5];
}

string AUri::query() const {
    PROFILE_FUNCTION()
    return p_ptr->mResult[7];
}

string AUri::fragment() const {
    PROFILE_FUNCTION()
    return p_ptr->mResult[9];
}

string AUri::name() const {
    PROFILE_FUNCTION()
    string str = path();
    size_t found = str.rfind('/');
    if(found != string::npos) {
        str.replace(0, found + 1, "");
    }
    return str;
}
