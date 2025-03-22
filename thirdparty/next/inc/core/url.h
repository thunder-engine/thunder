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

#ifndef URL_H
#define URL_H

#include <string>
#include <regex>

#include <global.h>
#include <metaobject.h>

class NEXT_LIBRARY_EXPORT Url {
    A_GENERIC(Url)

    A_METHODS(
        A_METHOD(string, Url::scheme),
        A_METHOD(string, Url::host),
        A_METHOD(string, Url::path),
        A_METHOD(string, Url::query),
        A_METHOD(string, Url::fragment),
        A_METHOD(string, Url::dir),
        A_METHOD(string, Url::name),
        A_METHOD(string, Url::baseName),
        A_METHOD(string, Url::suffix)
    )

public:
    Url();
    Url(const std::string &url);

    bool operator== (const Url &right) const;

    std::string scheme() const;
    std::string host() const;
    std::string path() const;
    std::string query() const;
    std::string fragment() const;
    std::string dir() const;
    std::string name() const;
    std::string baseName() const;
    std::string suffix() const;

private:
    std::string m_url;

    std::smatch m_result;

};

#endif // URL_H
