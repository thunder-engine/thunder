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
/*!
    Returns the scheme of the URI. If an empty string is returned, this means the scheme is undefined and the URI is then relative.
*/
string Uri::scheme() const {
    PROFILE_FUNCTION();
    return p_ptr->mResult[2].str();
}
/*!
    Returns the host of the URI if it is defined; otherwise an empty string is returned.
*/
string Uri::host() const {
    PROFILE_FUNCTION();
    return p_ptr->mResult[4];
}
/*!
    Returns the path of the URI.
*/
string Uri::path() const {
    PROFILE_FUNCTION();
    return p_ptr->mResult[5];
}
/*!
    Returns the query string of the URI if there's a query string, or an empty result if not.
*/
string Uri::query() const {
    PROFILE_FUNCTION();
    return p_ptr->mResult[7];
}
/*!
    Returns the fragment of the URI.
*/
string Uri::fragment() const {
    PROFILE_FUNCTION();
    return p_ptr->mResult[9];
}
/*!
    Returns a directory of URI path.
*/
string Uri::dir() const {
    PROFILE_FUNCTION();
    string str = path();
    size_t found = str.rfind('/');
    if(found != string::npos) {
        str.replace(found, str.length(), "");
    }
    return str;
}
/*!
    Returns a file name in the URI path.
*/
string Uri::name() const {
    PROFILE_FUNCTION();
    string str = path();
    size_t found = str.rfind('/');
    if(found != string::npos) {
        str.replace(0, found + 1, "");
    }
    return str;
}
/*!
    Returns a base name of file in the URI path.
*/
string Uri::baseName() const {
    PROFILE_FUNCTION();
    string str = name();
    size_t found = str.find('.');
    if(found != string::npos) {
        str.replace(found, str.length(), "");
    }
    return str;
}
/*!
    Returns a file suffix in the URI path.
*/
string Uri::suffix() const {
    PROFILE_FUNCTION();
    string str = name();
    size_t found = str.find('.');
    if(found != string::npos) {
        str.replace(0, found + 1, "");
    }
    return str;
}
