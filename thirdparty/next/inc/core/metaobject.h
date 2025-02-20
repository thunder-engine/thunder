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

#ifndef METAOBJECT_H
#define METAOBJECT_H

#include <string>

#include "metatype.h"
#include "metaproperty.h"
#include "metamethod.h"
#include "metaenum.h"

#include "macros.h"

class Object;

class NEXT_LIBRARY_EXPORT MetaObject {
public:
    typedef Object *(*Constructor)();

public:
    explicit MetaObject(const char *, const MetaObject *, const Constructor, const MetaMethod::Table *, const MetaProperty::Table *, const MetaEnum::Table *);

    const char *name() const;
    const MetaObject *super() const;

    Object *createInstance() const;

    int indexOfMethod(const char *) const;
    int indexOfSignal(const char *) const;
    int indexOfSlot(const char *) const;

    MetaMethod method(int) const;
    int methodCount() const;
    int methodOffset() const;

    int indexOfProperty(const char *) const;

    MetaProperty property(int) const;
    int propertyCount() const;
    int propertyOffset() const;

    int indexOfEnumerator(const char *) const;

    MetaEnum enumerator(int) const;
    int enumeratorCount() const;
    int enumeratorOffset() const;

    bool canCastTo(const char *) const;

private:
    Constructor m_constructor;
    const char *m_name;
    const MetaObject *m_super;
    const MetaMethod::Table *m_methods;
    const MetaProperty::Table *m_properties;
    const MetaEnum::Table *m_enums;
    int m_methodCount;
    int m_propCount;
    int m_enumCount;

};

#endif // METAOBJECT_H
