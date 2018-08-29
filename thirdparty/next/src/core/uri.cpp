#include "core/uri.h"

#include <regex>

class UriPrivate {
public:
    string                  mUri;

    smatch                  mResult;
};
/*!
    \class Uri
    \brief Uri class provides an interface for working with URI adresses.
    \since Next 1.0
    \inmodule Core
*/
Uri::Uri(const string &uri) :
        p_ptr(new UriPrivate) {
    PROFILE_FUNCTION()
    p_ptr->mUri = uri;
    replace(p_ptr->mUri.begin(), p_ptr->mUri.end(), '\\', '/');
    regex_match(p_ptr->mUri, p_ptr->mResult, regex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"));
}

Uri::~Uri() {
    delete p_ptr;
}

/*!
    Returns the URI scheme for current address.

    \code
        Uri address("scheme://host/path/to/uri?query#fragment");
        std::cout << address.scheme() << std::endl; // Prints: scheme
    \endcode
*/
string Uri::scheme() const {
    PROFILE_FUNCTION()
    return p_ptr->mResult[2].str();
}
/*!
    Returns the URI host for current address.

    \code
        Uri address"scheme://host/path/to/uri?query#fragment");
        std::cout << address.host() << std::endl; // Prints: host
    \endcode
*/
string Uri::host() const {
    PROFILE_FUNCTION()
    return p_ptr->mResult[4];
}
/*!
    Returns the URI path for current address.

    \code
        Uri address("scheme://host/path/to/uri?query#fragment");
        std::cout << address.path() << std::endl; // Prints: /path/to/uri
    \endcode
*/
string Uri::path() const {
    PROFILE_FUNCTION()
    return p_ptr->mResult[5];
}
/*!
    Returns the URI query for current address.

    \code
        Uri address("scheme://host/path/to/uri?query#fragment");
        std::cout << address.query() << std::endl; // Prints: query
    \endcode
*/
string Uri::query() const {
    PROFILE_FUNCTION()
    return p_ptr->mResult[7];
}
/*!
    Returns the URI fragment for current address.

    \code
        Uri address("scheme://host/path/to/uri?query#fragment");
        std::cout << address.fragment() << std::endl; // Prints: fragment
    \endcode
*/
string Uri::fragment() const {
    PROFILE_FUNCTION()
    return p_ptr->mResult[9];
}
/*!
    Returns the URI dir path for current address.

    \code
        Uri address("/host/path/to/uri.tar.gz");
        std::cout << address.fragment() << std::endl; // Prints: /host/path/to
    \endcode
*/
string Uri::dir() const {
    PROFILE_FUNCTION()
    string str = path();
    size_t found = str.rfind('/');
    if(found != string::npos) {
        str.replace(found, str.length(), "");
    }
    return str;
}
/*!
    Returns the URI file name for current address.

    \code
        Uri address("/host/path/to/uri.tar.gz");
        std::cout << address.fragment() << std::endl; // Prints: uri.tar.gz
    \endcode
*/
string Uri::name() const {
    PROFILE_FUNCTION()
    string str = path();
    size_t found = str.rfind('/');
    if(found != string::npos) {
        str.replace(0, found + 1, "");
    }
    return str;
}
/*!
    Returns the URI file base name for current address.

    \code
        Uri address("/host/path/to/uri.tar.gz");
        std::cout << address.fragment() << std::endl; // Prints: uri
    \endcode
*/
string Uri::baseName() const {
    PROFILE_FUNCTION()
    string str = name();
    size_t found = str.find('.');
    if(found != string::npos) {
        str.replace(found, str.length(), "");
    }
    return str;
}
/*!
    Returns the URI file suffix for current address.

    \code
        Uri address("/host/path/to/uri.tar.gz");
        std::cout << address.fragment() << std::endl; // Prints: tar.gz
    \endcode
*/
string Uri::suffix() const {
    PROFILE_FUNCTION()
    string str = name();
    size_t found = str.find('.');
    if(found != string::npos) {
        str.replace(0, found + 1, "");
    }
    return str;
}
