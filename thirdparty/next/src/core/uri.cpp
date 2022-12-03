#include "core/uri.h"

/*!
    \class Uri
    \brief Uri class provides an interface for working with URI's.
    \since Next 1.0
    \inmodule Core
*/
Uri::Uri(const string &uri) :
        m_uri(uri) {

    PROFILE_FUNCTION();
    replace(m_uri.begin(), m_uri.end(), '\\', '/');
    regex_match(m_uri, m_result, regex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"));
}

/*!
    Returns the scheme of the URI. If an empty string is returned, this means the scheme is undefined and the URI is then relative.
*/
string Uri::scheme() const {
    PROFILE_FUNCTION();
    return m_result[2].str();
}
/*!
    Returns the host of the URI if it is defined; otherwise an empty string is returned.
*/
string Uri::host() const {
    PROFILE_FUNCTION();
    return m_result[4];
}
/*!
    Returns the path of the URI.
*/
string Uri::path() const {
    PROFILE_FUNCTION();
    return m_result[5];
}
/*!
    Returns the query string of the URI if there's a query string, or an empty result if not.
*/
string Uri::query() const {
    PROFILE_FUNCTION();
    return m_result[7];
}
/*!
    Returns the fragment of the URI.
*/
string Uri::fragment() const {
    PROFILE_FUNCTION();
    return m_result[9];
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
