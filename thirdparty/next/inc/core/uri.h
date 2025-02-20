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

#ifndef URI_H
#define URI_H

#include <string>
#include <regex>

#include <global.h>

class NEXT_LIBRARY_EXPORT Uri {
public:
    Uri(const std::string &uri);

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
    std::string m_uri;

    std::smatch m_result;

};

#endif // URI_H
