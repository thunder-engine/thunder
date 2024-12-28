/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2023 Evgeniy Prikazchikov
*/

#include "core/uri.h"

/*!
    \class Uri
    \brief Uri class provides an interface for working with URI's.
    \since Next 1.0
    \inmodule Core
*/
Uri::Uri(const std::string &uri) :
        m_uri(uri) {

    PROFILE_FUNCTION();
    replace(m_uri.begin(), m_uri.end(), '\\', '/');
    regex_match(m_uri, m_result, std::regex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"));
}

/*!
    \fn std::string Uri::scheme() const

    Returns the scheme of the URI. If an empty string is returned, this means the scheme is undefined and the URI is then relative.
*/
std::string Uri::scheme() const {
    PROFILE_FUNCTION();
    return m_result[2].str();
}
/*!
    \fn std::string Uri::host() const

    Returns the host of the URI if it is defined; otherwise an empty string is returned.
*/
std::string Uri::host() const {
    PROFILE_FUNCTION();
    return m_result[4];
}
/*!
    \fn std::string Uri::path() const

    Returns the path of the URI.
*/
std::string Uri::path() const {
    PROFILE_FUNCTION();
    return m_result[5];
}
/*!
    \fn std::string Uri::query() const

    Returns the query string of the URI if there's a query string, or an empty result if not.
*/
std::string Uri::query() const {
    PROFILE_FUNCTION();
    return m_result[7];
}
/*!
    \fn std::string Uri::fragment() const

    Returns the fragment of the URI.
*/
std::string Uri::fragment() const {
    PROFILE_FUNCTION();
    return m_result[9];
}
/*!
    \fn std::string Uri::dir() const

    Returns a directory of URI path.
*/
std::string Uri::dir() const {
    PROFILE_FUNCTION();
    std::string str = path();
    size_t found = str.rfind('/');
    if(found != std::string::npos) {
        str.replace(found, str.length(), "");
    }
    return str;
}
/*!
    \fn std::string Uri::name() const

    Returns a file name in the URI path.
*/
std::string Uri::name() const {
    PROFILE_FUNCTION();
    std::string str = path();
    size_t found = str.rfind('/');
    if(found != std::string::npos) {
        str.replace(0, found + 1, "");
    }
    return str;
}
/*!
    \fn std::string Uri::baseName() const

    Returns a base name of file in the URI path.
*/
std::string Uri::baseName() const {
    PROFILE_FUNCTION();
    std::string str = name();
    size_t found = str.find('.');
    if(found != std::string::npos) {
        str.replace(found, str.length(), "");
    }
    return str;
}
/*!
    \fn std::string Uri::suffix() const

    Returns a file suffix in the URI path.
*/
std::string Uri::suffix() const {
    PROFILE_FUNCTION();
    std::string str = name();
    size_t found = str.find('.');
    if(found != std::string::npos) {
        str.replace(0, found + 1, "");
    }
    return str;
}
