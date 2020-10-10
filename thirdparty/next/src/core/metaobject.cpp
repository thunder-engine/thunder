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
        m_Constructor(constructor),
        m_pName(name),
        m_pSuper(super),
        m_pMethods(methods),
        m_pProperties(props),
        m_pEnums(enums),
        m_MethodCount(0),
        m_PropCount(0),
        m_EnumCount(0) {
    PROFILE_FUNCTION();
    while(methods && methods[m_MethodCount].name) {
        m_MethodCount++;
    }
    while(props && props[m_PropCount].type) {
        m_PropCount++;
    }
    while(enums && enums[m_EnumCount].name) {
        m_EnumCount++;
    }
}
/*!
    Returns the name of the object type.
*/
const char *MetaObject::name() const {
    PROFILE_FUNCTION();
    return m_pName;
}
/*!
    Returns an introspection object for parent class.
*/
const MetaObject *MetaObject::super() const {
    PROFILE_FUNCTION();
    return m_pSuper;
}
/*!
    Constructs and return a new instance of associated class.
*/
Object *MetaObject::createInstance() const {
    PROFILE_FUNCTION();
    return (*m_Constructor)();
}
/*!
    Returns index of class method by provided \a signature; otherwise returns -1.
    \note This method looks through class hierarchy.
*/
int MetaObject::indexOfMethod(const char *signature) const {
    PROFILE_FUNCTION();
    const MetaObject *s = this;

    while(s) {
        for(int i = 0; i < s->m_MethodCount; ++i) {
            if(MetaMethod(s->m_pMethods + i).signature() == signature) {
                return i + s->methodOffset();
            }
        }
        s = s->m_pSuper;
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

    while(s) {
        for(int i = 0; i < s->m_MethodCount; ++i) {
            MetaMethod m(s->m_pMethods + i);
            if(m.type() == MetaMethod::Signal && m.signature() == signature) {
                return i + s->methodOffset();
            }
        }
        s = s->m_pSuper;
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

    while(s) {
        for(int i = 0; i < s->m_MethodCount; ++i) {
            MetaMethod m(s->m_pMethods + i);
            if(m.type() == MetaMethod::Slot && m.signature() == signature) {
                return i + s->methodOffset();
            }
        }
        s = s->m_pSuper;
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
    if(i < 0 && m_pSuper) {
        return m_pSuper->method(index);
    }

    if(i >= 0 && i < m_MethodCount) {
        return MetaMethod(m_pMethods + i);
    }
    return MetaMethod(nullptr);
}
/*!
    Returns the sum of methods for the current class and parent classes. It's includes signals and slots.
*/
int MetaObject::methodCount() const {
    PROFILE_FUNCTION();
    int count = m_MethodCount;
    const MetaObject *s = m_pSuper;
    while(s) {
        count += s->m_MethodCount;
        s = s->m_pSuper;
    }
    return count;
}
/*!
    Returns the first index of method for current class. The offset is the sum of all methods in parent classes.
*/
int MetaObject::methodOffset() const {
    PROFILE_FUNCTION();
    int offset = 0;
    const MetaObject *s = m_pSuper;
    while(s) {
        offset += s->m_MethodCount;
        s = s->m_pSuper;
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
        for(int i = 0; i < s->m_PropCount; ++i) {
            if(strcmp(s->m_pProperties[i].name, name) == 0) {
                return i + s->propertyOffset();
            }
        }
        s = s->m_pSuper;
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
    if(i < 0 && m_pSuper) {
        return m_pSuper->property(index);
    }
    if(i >= 0 && i < m_PropCount) {
        return MetaProperty(m_pProperties + i);
    }
    return MetaProperty(nullptr);
}
/*!
    Returns the sum of properties for the current class and parent classes.
*/
int MetaObject::propertyCount() const {
    PROFILE_FUNCTION();
    int count = m_PropCount;
    const MetaObject *s = m_pSuper;
    while(s) {
        count  += s->m_PropCount;
        s = s->m_pSuper;
    }
    return count;
}
/*!
    Returns the first index of property for current class. The offset is the sum of all properties in parent classes.
*/
int MetaObject::propertyOffset() const {
    PROFILE_FUNCTION();
    int offset = 0;
    const MetaObject *s = m_pSuper;
    while(s) {
        offset += s->m_PropCount;
        s = s->m_pSuper;
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
        for(int i = 0; i < s->m_EnumCount; ++i) {
            if(strcmp(s->m_pEnums[i].name, name) == 0) {
                return i + s->enumeratorOffset();
            }
        }
        s = s->m_pSuper;
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
    if(i < 0 && m_pSuper) {
        return m_pSuper->enumerator(index);
    }
    if(i >= 0 && i < m_EnumCount) {
        return MetaEnum(m_pEnums + i);
    }
    return MetaEnum(nullptr);
}
/*!
    Returns the sum of enumerators for the current class and parent classes.
*/
int MetaObject::enumeratorCount() const {
    PROFILE_FUNCTION();
    int count = m_EnumCount;
    const MetaObject *s = m_pSuper;
    while(s) {
        count  += s->m_EnumCount;
        s = s->m_pSuper;
    }
    return count;
}
/*!
    Returns the first index of enumerator for current class. The offset is the sum of all enumerator in parent classes.
*/
int MetaObject::enumeratorOffset() const {
    PROFILE_FUNCTION();
    int offset = 0;
    const MetaObject *s = m_pSuper;
    while(s) {
        offset += s->m_EnumCount;
        s = s->m_pSuper;
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
        if(strcmp(s->m_pName, type) == 0) {
            return true;
        }
        s = s->m_pSuper;
    }
    return false;
}
