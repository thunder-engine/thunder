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

#ifndef URI_H
#define URI_H

#include <string>
#include <regex>

#include <global.h>

using namespace std;

class NEXT_LIBRARY_EXPORT Uri {
public:
    Uri(const string &uri);

    string scheme() const;
    string host() const;
    string path() const;
    string query() const;
    string fragment() const;
    string dir() const;
    string name() const;
    string baseName() const;
    string suffix() const;

private:
    string m_uri;

    smatch m_result;

};

#endif // URI_H
