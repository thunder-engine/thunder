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

#ifndef PROCESSENVIRONMENT_H
#define PROCESSENVIRONMENT_H

#include <global.h>
#include <astring.h>

#include <map>

class NEXT_LIBRARY_EXPORT ProcessEnvironment {
public:
    ProcessEnvironment &operator=(const ProcessEnvironment &other);

    void insert(const TString &key, const TString &value);

    TString value(const TString &key) const;

    const std::map<TString, TString> &envVars() const;

    static ProcessEnvironment systemEnvironment();

private:
    std::map<TString, TString> m_envVars;

};

#endif // PROCESSENVIRONMENT_H
