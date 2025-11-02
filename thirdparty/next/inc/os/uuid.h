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

#ifndef UUID_H
#define UUID_H

#include <astring.h>
#include <array>

class NEXT_LIBRARY_EXPORT Uuid {
public:
    Uuid();

    explicit Uuid(const TString &uuid);

    static Uuid createUuid();

    bool isNull() const;

    TString toString() const;

    ByteArray toByteArray() const;

    bool operator== (const Uuid &other) const;

    bool operator!= (const Uuid &other) const;

    bool operator< (const Uuid &other) const;

private:
    std::array<uint8_t, 16> data;

};

#endif // UUID_H
