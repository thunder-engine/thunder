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

#include "core/metaobject.h"

#include "core/object.h"

#include <cstring>
/*!
    \class MetaObject
    \brief The MetaObject provides an interface to retrieve information about Object at runtime.
    \since Next 1.0
    \inmodule Core

    This class is a part of Object-Introspection-Mechanism. MetaObject provides information about one particular class inherited from base Object class.
    Developers are able to retrieve information about properties, methods and inheritance chains.

    This class is actively used in Signal-Slot mechanism.
*/
/*!
    \typedef MetaObject::Constructor

    Callback which contain address to method to construct new Object with represented type.
*/
/*!
    Constructs MetaObject object for Object with type \a name, inherited from \a super class and provided \a constructor, \a methods, \a props and \a enums.
*/
MetaObject::MetaObject(const char *name, const MetaObject *super, const Constructor constructor,
                       const MetaMethod::Table *methods, const MetaProperty::Table *props, const MetaEnum::Table *enums) :
        m_constructor(constructor),
        m_name(name),
        m_super(super),
        m_methods(methods),
        m_properties(props),
        m_enums(enums),
        m_methodCount(0),
        m_propCount(0),
        m_enumCount(0) {
    PROFILE_FUNCTION();
    while(methods && methods[m_methodCount].name) {
        m_methodCount++;
    }
    while(props && props[m_propCount].type) {
        m_propCount++;
    }
    while(enums && enums[m_enumCount].name) {
        m_enumCount++;
    }
}
/*!
    Returns the name of the object type.
*/
const char *MetaObject::name() const {
    PROFILE_FUNCTION();
    return m_name;
}
/*!
    Returns an introspection object for parent class.
*/
const MetaObject *MetaObject::super() const {
    PROFILE_FUNCTION();
    return m_super;
}
/*!
    Constructs and return a new instance of associated class.
*/
Object *MetaObject::createInstance() const {
    PROFILE_FUNCTION();
    return (*m_constructor)();
}
/*!
    Returns index of class method by provided \a signature; otherwise returns -1.
    \note This method looks through class hierarchy.
*/
int MetaObject::indexOfMethod(const char *signature) const {
    PROFILE_FUNCTION();
    const MetaObject *s = this;

    int hash = Mathf::hashString(signature);

    while(s) {
        for(int i = 0; i < s->m_methodCount; ++i) {
            if(MetaMethod(s->m_methods + i).hash() == hash) {
                return i + s->methodOffset();
            }
        }
        s = s->m_super;
    }
    return -1;
}
/*!
    Returns index of class signal by provided \a signature; otherwise returns -1.
    \note This method looks through class hierarchy.
*/
int MetaObject::indexOfSignal(const char *signature) const {
    PROFILE_FUNCTION();
    const MetaObject *s = this;

    int hash = Mathf::hashString(signature);

    while(s) {
        for(int i = 0; i < s->m_methodCount; ++i) {
            MetaMethod m(s->m_methods + i);
            if(m.type() == MetaMethod::Signal && m.hash() == hash) {
                return i + s->methodOffset();
            }
        }
        s = s->m_super;
    }
    return -1;
}
/*!
    Returns index of class slot by provided \a signature; otherwise returns -1.
    \note This method looks through class hierarchy.
*/
int MetaObject::indexOfSlot(const char *signature) const {
    PROFILE_FUNCTION();
    const MetaObject *s = this;

    int hash = Mathf::hashString(signature);

    while(s) {
        for(int i = 0; i < s->m_methodCount; ++i) {
            MetaMethod m(s->m_methods + i);
            if(m.type() == MetaMethod::Slot && m.hash() == hash) {
                return i + s->methodOffset();
            }
        }
        s = s->m_super;
    }
    return -1;
}
/*!
    Returns MetaMethod object by provided \a index of method.
    \note This method looks through class hierarchy.
*/
MetaMethod MetaObject::method(int index) const {
    PROFILE_FUNCTION();
    int i = index - methodOffset();
    if(i < 0 && m_super) {
        return m_super->method(index);
    }

    if(i >= 0 && i < m_methodCount) {
        return MetaMethod(m_methods + i);
    }
    return MetaMethod(nullptr);
}
/*!
    Returns the sum of methods for the current class and parent classes. It's includes signals and slots.
*/
int MetaObject::methodCount() const {
    PROFILE_FUNCTION();
    int count = m_methodCount;
    const MetaObject *s = m_super;
    while(s) {
        count += s->m_methodCount;
        s = s->m_super;
    }
    return count;
}
/*!
    Returns the first index of method for current class. The offset is the sum of all methods in parent classes.
*/
int MetaObject::methodOffset() const {
    PROFILE_FUNCTION();
    int offset = 0;
    const MetaObject *s = m_super;
    while(s) {
        offset += s->m_methodCount;
        s = s->m_super;
    }
    return offset;
}
/*!
    Returns index of class property by provided \a name; otherwise returns -1.
    \note This method looks through class hierarchy.
*/
int MetaObject::indexOfProperty(const char *name) const {
    PROFILE_FUNCTION();
    const MetaObject *s = this;

    while(s) {
        for(int i = 0; i < s->m_propCount; ++i) {
            if(strcmp(s->m_properties[i].name, name) == 0) {
                return i + s->propertyOffset();
            }
        }
        s = s->m_super;
    }
    return -1;
}
/*!
    Returns MetaProperty object by provided \a index of property.
    \note This method looks through class hierarchy.
*/
MetaProperty MetaObject::property(int index) const {
    PROFILE_FUNCTION();
    int i = index - propertyOffset();
    if(i < 0 && m_super) {
        return m_super->property(index);
    }
    if(i >= 0 && i < m_propCount) {
        return MetaProperty(m_properties + i);
    }
    return MetaProperty(nullptr);
}
/*!
    Returns the sum of properties for the current class and parent classes.
*/
int MetaObject::propertyCount() const {
    PROFILE_FUNCTION();
    int count = m_propCount;
    const MetaObject *s = m_super;
    while(s) {
        count += s->m_propCount;
        s = s->m_super;
    }
    return count;
}
/*!
    Returns the first index of property for current class. The offset is the sum of all properties in parent classes.
*/
int MetaObject::propertyOffset() const {
    PROFILE_FUNCTION();
    int offset = 0;
    const MetaObject *s = m_super;
    while(s) {
        offset += s->m_propCount;
        s = s->m_super;
    }
    return offset;
}
/*!
    Returns index of class enumerator by provided \a name; otherwise returns -1.
    \note This method looks through class hierarchy.
*/
int MetaObject::indexOfEnumerator(const char *name) const {
    PROFILE_FUNCTION();
    const MetaObject *s = this;

    while(s) {
        for(int i = 0; i < s->m_enumCount; ++i) {
            if(strcmp(s->m_enums[i].name, name) == 0) {
                return i + s->enumeratorOffset();
            }
        }
        s = s->m_super;
    }
    return -1;
}
/*!
    Returns MetaEnum object by provided \a index of enumerator.
    \note This method looks through class hierarchy.
*/
MetaEnum MetaObject::enumerator(int index) const {
    PROFILE_FUNCTION();
    int i = index - enumeratorOffset();
    if(i < 0 && m_super) {
        return m_super->enumerator(index);
    }
    if(i >= 0 && i < m_enumCount) {
        return MetaEnum(m_enums + i);
    }
    return MetaEnum(nullptr);
}
/*!
    Returns the sum of enumerators for the current class and parent classes.
*/
int MetaObject::enumeratorCount() const {
    PROFILE_FUNCTION();
    int count = m_enumCount;
    const MetaObject *s = m_super;
    while(s) {
        count += s->m_enumCount;
        s = s->m_super;
    }
    return count;
}
/*!
    Returns the first index of enumerator for current class. The offset is the sum of all enumerator in parent classes.
*/
int MetaObject::enumeratorOffset() const {
    PROFILE_FUNCTION();
    int offset = 0;
    const MetaObject *s = m_super;
    while(s) {
        offset += s->m_enumCount;
        s = s->m_super;
    }
    return offset;
}

/*!
    Checks the abillity to cast the current object to \a type.
    \note This method tries to go through inheritance to find a common parent class.

    Returns true if object can be cast to \a type; otherwise returns false.
*/
bool MetaObject::canCastTo(const char *type) const {
    PROFILE_FUNCTION();
    const MetaObject *s = this;

    while(s) {
        if(strcmp(s->m_name, type) == 0) {
            return true;
        }
        s = s->m_super;
    }
    return false;
}
