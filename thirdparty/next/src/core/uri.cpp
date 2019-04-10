#include "core/uri.h"

#include <regex>

class UriPrivate {
public:
    string                  mUri;

    smatch                  mResult;
};
/*!
    \class Uri
    \brief Uri class provides an interface for working with URI's.
    \since Next 1.0
    \inmodule Core
*/
Uri::Uri(const string &uri) :
        p_ptr(new UriPrivate) {
    PROFILE_FUNCTION();
    p_ptr->mUri = uri;
    replace(p_ptr->mUri.begin(), p_ptr->mUri.end(), '\\', '/');
    regex_match(p_ptr->mUri, p_ptr->mResult, regex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"));
}

Uri::~Uri() {
    delete p_ptr;
}

string Uri::scheme() const {
    PROFILE_FUNCTION();
    return p_ptr->mResult[2].str();
}

string Uri::host() const {
    PROFILE_FUNCTION();
    return p_ptr->mResult[4];
}

string Uri::path() const {
    PROFILE_FUNCTION();
    return p_ptr->mResult[5];
}

string Uri::query() const {
    PROFILE_FUNCTION();
    return p_ptr->mResult[7];
}

string Uri::fragment() const {
    PROFILE_FUNCTION();
    return p_ptr->mResult[9];
}

string Uri::dir() const {
    PROFILE_FUNCTION();
    string str = path();
    size_t found = str.rfind('/');
    if(found != string::npos) {
        str.replace(found, str.length(), "");
    }
    return str;
}

string Uri::name() const {
    PROFILE_FUNCTION();
    string str = path();
    size_t found = str.rfind('/');
    if(found != string::npos) {
        str.replace(0, found + 1, "");
    }
    return str;
}

string Uri::baseName() const {
    PROFILE_FUNCTION();
    string str = name();
    size_t found = str.find('.');
    if(found != string::npos) {
        str.replace(found, str.length(), "");
    }
    return str;
}

string Uri::suffix() const {
    PROFILE_FUNCTION();
    string str = name();
    size_t found = str.find('.');
    if(found != string::npos) {
        str.replace(0, found + 1, "");
    }
    return str;
}
