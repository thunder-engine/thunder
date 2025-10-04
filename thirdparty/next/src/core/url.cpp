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

Url::Url(const TString &url) :
        m_url(url) {

    PROFILE_FUNCTION();

    m_url.replace('\\', '/');

    static const std::regex reg("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?");
    std::regex_match(m_url.toStdString(), m_result, reg);
}

Url::~Url() {

}
/*!
    Compares current Url with \a right hand Url; Returns true if Urls are equal.
*/
bool Url::operator== (const Url &right) const {
    return m_url == right.m_url;
}
/*!
    \fn TString Uri::scheme() const

    Returns the scheme of the URI. If an empty string is returned, this means the scheme is undefined and the URI is then relative.
*/
TString Url::scheme() const {
    PROFILE_FUNCTION();
    return TString(m_result[2]);
}
/*!
    \fn TString Uri::host() const

    Returns the host of the URI if it is defined; otherwise an empty string is returned.
*/
TString Url::host() const {
    PROFILE_FUNCTION();
    return TString(m_result[4]);
}
/*!
    \fn TString Uri::path() const

    Returns the path of the URI.
*/
TString Url::path() const {
    PROFILE_FUNCTION();
    return TString(m_result[5]);
}
/*!
    \fn TString Uri::query() const

    Returns the query string of the URI if there's a query string, or an empty result if not.
*/
TString Url::query() const {
    PROFILE_FUNCTION();
    return TString(m_result[7]);
}
/*!
    \fn TString Uri::fragment() const

    Returns the fragment of the URI.
*/
TString Url::fragment() const {
    PROFILE_FUNCTION();
    return TString(m_result[9]);
}
/*!
    \fn TString Uri::dir() const

    Returns a directory of URI path.
*/
TString Url::dir() const {
    PROFILE_FUNCTION();
    TString str = path();
    size_t found = str.lastIndexOf('/');
    if(found != -1) {
        return str.left(found);
    }
    return str;
}
/*!
    \fn TString Uri::absoluteDir() const

    Returns the absolute dir path of the URI.
*/
TString Url::absoluteDir() const {
    PROFILE_FUNCTION();
    TString preffix(scheme());
    if(!preffix.isEmpty()) {
        preffix += ":";
    }
    return preffix + host() + dir();
}
/*!
    \fn TString Uri::name() const

    Returns a file name in the URI path.
*/
TString Url::name() const {
    PROFILE_FUNCTION();
    TString str = path();
    size_t found = str.lastIndexOf('/');
    if(found != -1) {
        return str.right(found + 1);
    }
    return str;
}
/*!
    \fn TString Uri::baseName() const

    Returns a base name of file in the URI path.
*/
TString Url::baseName() const {
    PROFILE_FUNCTION();
    TString str = name();
    size_t found = str.indexOf('.');
    if(found != -1) {
        return str.left(found);
    }
    return str;
}
/*!
    \fn TString Uri::suffix() const

    Returns a file suffix in the URI path.
*/
TString Url::suffix() const {
    PROFILE_FUNCTION();
    TString str = name();
    size_t found = str.indexOf('.');
    if(found != -1) {
        return str.right(found + 1);
    }
    return str;
}
