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

#include <astring.h>
#include <regex>

#include <global.h>
#include <metaobject.h>

class NEXT_LIBRARY_EXPORT Url {
    A_GENERIC(Url)

    A_METHODS(
        A_METHOD(TString, Url::scheme),
        A_METHOD(TString, Url::host),
        A_METHOD(TString, Url::path),
        A_METHOD(TString, Url::query),
        A_METHOD(TString, Url::fragment),
        A_METHOD(TString, Url::dir),
        A_METHOD(TString, Url::name),
        A_METHOD(TString, Url::baseName),
        A_METHOD(TString, Url::suffix)
    )

public:
    Url();
    Url(const TString &url);

    bool operator== (const Url &right) const;

    TString scheme() const;
    TString host() const;
    TString path() const;
    TString query() const;
    TString fragment() const;
    TString dir() const;
    TString absoluteDir() const;
    TString name() const;
    TString baseName() const;
    TString suffix() const;

private:
    TString m_url;

    std::smatch m_result;

};

#endif // URL_H
