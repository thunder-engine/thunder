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

#ifndef METAENUM_H
#define METAENUM_H

#include "metatype.h"

class NEXT_LIBRARY_EXPORT MetaEnum {
public:
    struct EnumTable {
        const char *name;
        int value;
    };

    struct Table {
        const char *name;
        const EnumTable *table;
    };

public:
    explicit MetaEnum(const Table *table);

    bool isValid() const;

    const char *name() const;

    int keyCount() const;
    const char *key(int index) const;

    int value(int index) const;

    int keyToValue(const char *key) const;
    const char *valueToKey(int value) const;

    const Table *table() const;

private:
    const Table *m_table;

    int m_enumCount;

};

#endif // METAENUM_H
