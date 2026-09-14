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

#include <algorithm>

/*!
    \class Url
    \brief Url class provides an interface for working with Url's.
    \since Next 1.0
    \inmodule Core
*/

Url::Url() {

}

Url::Url(const TString &url) :
    m_url(url.toStdString()) {

    PROFILE_FUNCTION();

    std::replace(m_url.begin(), m_url.end(), '\\', '/');

    static const std::regex reg("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?");
    std::regex_match(m_url, m_result, reg);
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
TString Url::filePath() const {
    PROFILE_FUNCTION();
    return TString(m_result[5]);
}
/*!
    Returns the absolute file path of the URI.
*/
TString Url::absoluteFilePath() const {
    PROFILE_FUNCTION();
    TString preffix(scheme());
    if(!preffix.isEmpty()) {
        preffix += ":";
    }
    return preffix + host() + filePath();
}
/*!
    Returns a relative file path of URI relative to the given \a base directory.
    If paths have no common prefix, returns the full absolute path including file name.
    Handles parent directory transitions (../) when paths share a common prefix.
*/
TString Url::relativeFilePath(const TString &base) const {
    PROFILE_FUNCTION();

    TString dirPath = relativeDir(base);
    TString fileName = name();

    if(dirPath.isEmpty() || dirPath == ".") {
        return fileName;
    }

    if(dirPath.back() == '/') {
        return dirPath + fileName;
    }

    return dirPath + "/" + fileName;
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
    TString str = filePath();
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
    Returns a relative directory of URI path relative to the given \a base directory.
    If paths have no common prefix, returns the full absolute path.
    Handles parent directory transitions (../) when paths share a common prefix.
*/
TString Url::relativeDir(const TString &base) const {
    PROFILE_FUNCTION();

    TString fullPath = filePath();
    if(!host().isEmpty() && host().back() == ':') {
        fullPath = host() + fullPath;
    }
    TString basePath = base;

    basePath.replace('\\', '/');
    fullPath.replace('\\', '/');

    bool pathHasTrailingSlash = fullPath.size() > 1 && fullPath.back() == '/';
    if(!pathHasTrailingSlash) {
        int lastSlash = fullPath.lastIndexOf('/');
        if(lastSlash != -1) {
            fullPath = fullPath.left(lastSlash);
        } else {
            fullPath.clear();
        }
    }

    bool baseIsRoot = basePath == "/";
    if(basePath.size() > 1 && basePath.back() == '/') {
        basePath = basePath.left(basePath.size() - 1);
    }
    if(fullPath.size() > 1 && fullPath.back() == '/') {
        fullPath = fullPath.left(fullPath.size() - 1);
    }

    if(basePath.isEmpty() || baseIsRoot) {
        if(baseIsRoot && !fullPath.isEmpty() && fullPath.front() == '/') {
            fullPath = fullPath.mid(1, fullPath.size() - 1);
        }
        if(!fullPath.isEmpty() && fullPath.back() != '/') {
            fullPath += "/";
        }
        return fullPath;
    }

    StringList baseParts = basePath.split('/');
    StringList fullParts = fullPath.split('/');

    std::vector<TString> baseVec;
    for(const TString &part : baseParts) {
        if(!part.isEmpty()) {
            baseVec.push_back(part);
        }
    }

    std::vector<TString> fullVec;
    for(const TString &part : fullParts) {
        if(!part.isEmpty()) {
            fullVec.push_back(part);
        }
    }

    size_t common = 0;
    size_t minSize = std::min(baseVec.size(), fullVec.size());
    while(common < minSize && baseVec[common] == fullVec[common]) {
        common++;
    }

    if(common == 0) {
        if(!fullPath.isEmpty() && fullPath.back() != '/') {
            fullPath += "/";
        }
        return fullPath;
    }

    TString result;

    for(size_t i = common; i < baseVec.size(); i++) {
        result += "../";
    }

    for(size_t i = common; i < fullVec.size(); i++) {
        result += fullVec[i];
        if(i < fullVec.size() - 1) {
            result += "/";
        }
    }

    if(result.isEmpty()) {
        result = ".";
    } else if(result.back() != '/') {
        result += "/";
    }

    return result;
}
/*!
    Returns a file name in the URI path.
*/
TString Url::name() const {
    PROFILE_FUNCTION();
    TString str = filePath();
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
    char c = filePath().front();
    return c == '/' || c == '\\';
}
