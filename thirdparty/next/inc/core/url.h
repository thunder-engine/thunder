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
        A_METHOD(TString, Url::suffix),
        A_METHOD(TString, Url::completeSuffix)
    )

public:
    Url();
    Url(const TString &url);

    virtual ~Url();

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
    TString completeSuffix() const;

    bool isAbsolute() const;

private:
    TString m_url;

    std::smatch m_result;

};

#endif // URL_H
