/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
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
    Returns the scheme of the URI. If an empty string is returned, this means the scheme is undefined and the URI is then relative.
*/
TString Url::scheme() const {
    PROFILE_FUNCTION();
    return TString(m_result[2]);
}
/*!
    Returns the host of the URI if it is defined; otherwise an empty string is returned.
*/
TString Url::host() const {
    PROFILE_FUNCTION();
    return TString(m_result[4]);
}
/*!
    Returns the path of the URI.
*/
TString Url::path() const {
    PROFILE_FUNCTION();
    return TString(m_result[5]);
}
/*!
    Returns the query string of the URI if there's a query string, or an empty result if not.
*/
TString Url::query() const {
    PROFILE_FUNCTION();
    return TString(m_result[7]);
}
/*!
    Returns the fragment of the URI.
*/
TString Url::fragment() const {
    PROFILE_FUNCTION();
    return TString(m_result[9]);
}
/*!
    Returns a directory of URI path.
*/
TString Url::dir() const {
    PROFILE_FUNCTION();
    TString str = path();
    size_t found = str.lastIndexOf('/');
    if(found != -1) {
        return str.left(found);
    }
    return TString();
}
/*!
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
    Returns a file name suffix name of file in the URI path.
*/
TString Url::suffix() const {
    PROFILE_FUNCTION();
    TString str = name();
    size_t found = str.lastIndexOf('.');
    if(found != -1) {
        return str.right(found + 1);
    }
    return TString();
}
/*!
    Returns a file suffix in the URI path.
*/
TString Url::completeSuffix() const {
    PROFILE_FUNCTION();
    TString str = name();
    size_t found = str.indexOf('.');
    if(found != -1) {
        return str.right(found + 1);
    }
    return TString();
}
/*!
    Returns true if provided path is absolute.
*/
bool Url::isAbsolute() const {
    char c = path().front();
    return c == '/' || c == '\\';
}
