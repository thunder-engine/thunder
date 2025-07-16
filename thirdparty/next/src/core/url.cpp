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

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#include "core/url.h"

/*!
    \class Url
    \brief Url class provides an interface for working with Url's.
    \since Next 1.0
    \inmodule Core
*/

Url::Url() {

}

Url::Url(const String &url) :
        m_url(url) {

    PROFILE_FUNCTION();

    m_url.replace('\\', '/');

    static const std::regex reg("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?");
    std::regex_match(m_url.toStdString(), m_result, reg);
}
/*!
    Compares current Url with \a right hand Url; Returns true if Urls are equal.
*/
bool Url::operator== (const Url &right) const {
    return m_url == right.m_url;
}
/*!
    \fn std::string Uri::scheme() const

    Returns the scheme of the URI. If an empty string is returned, this means the scheme is undefined and the URI is then relative.
*/
String Url::scheme() const {
    PROFILE_FUNCTION();
    return m_result[2].str();
}
/*!
    \fn std::string Uri::host() const

    Returns the host of the URI if it is defined; otherwise an empty string is returned.
*/
String Url::host() const {
    PROFILE_FUNCTION();
    return String(m_result[4]);
}
/*!
    \fn std::string Uri::path() const

    Returns the path of the URI.
*/
String Url::path() const {
    PROFILE_FUNCTION();
    return String(m_result[5]);
}
/*!
    \fn std::string Uri::query() const

    Returns the query string of the URI if there's a query string, or an empty result if not.
*/
String Url::query() const {
    PROFILE_FUNCTION();
    return String(m_result[7]);
}
/*!
    \fn std::string Uri::fragment() const

    Returns the fragment of the URI.
*/
String Url::fragment() const {
    PROFILE_FUNCTION();
    return String(m_result[9]);
}
/*!
    \fn std::string Uri::dir() const

    Returns a directory of URI path.
*/
String Url::dir() const {
    PROFILE_FUNCTION();
    String str = path();
    size_t found = str.lastIndexOf('/');
    if(found != -1) {
        return str.left(found);
    }
    return str;
}
/*!
    \fn std::string Uri::name() const

    Returns a file name in the URI path.
*/
String Url::name() const {
    PROFILE_FUNCTION();
    String str = path();
    size_t found = str.lastIndexOf('/');
    if(found != -1) {
        return str.right(found + 1);
    }
    return str;
}
/*!
    \fn std::string Uri::baseName() const

    Returns a base name of file in the URI path.
*/
String Url::baseName() const {
    PROFILE_FUNCTION();
    String str = name();
    size_t found = str.indexOf('.');
    if(found != -1) {
        return str.left(found);
    }
    return str;
}
/*!
    \fn std::string Uri::suffix() const

    Returns a file suffix in the URI path.
*/
String Url::suffix() const {
    PROFILE_FUNCTION();
    String str = name();
    size_t found = str.indexOf('.');
    if(found != -1) {
        return str.right(found + 1);
    }
    return str;
}
