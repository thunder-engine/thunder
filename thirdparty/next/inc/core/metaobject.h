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
    typedef Object             *(*Constructor)              ();

public:
    explicit MetaObject         (const char *, const MetaObject *, const Constructor, const MetaMethod::Table *, const MetaProperty::Table *, const MetaEnum::Table *);

    const char                 *name                        () const;
    const MetaObject           *super                       () const;

    Object                     *createInstance              () const;

    int                         indexOfMethod               (const char *) const;
    int                         indexOfSignal               (const char *) const;
    int                         indexOfSlot                 (const char *) const;

    MetaMethod                  method                      (int) const;
    int                         methodCount                 () const;
    int                         methodOffset                () const;

    int                         indexOfProperty             (const char *) const;

    MetaProperty                property                    (int) const;
    int                         propertyCount               () const;
    int                         propertyOffset              () const;

    int                         indexOfEnumerator           (const char *) const;

    MetaEnum                    enumerator                  (int) const;
    int                         enumeratorCount             () const;
    int                         enumeratorOffset            () const;

    bool                        canCastTo                   (const char *) const;

private:
    Constructor                 m_Constructor;
    const char                 *m_pName;
    const MetaObject           *m_pSuper;
    const MetaMethod::Table    *m_pMethods;
    const MetaProperty::Table  *m_pProperties;
    const MetaEnum::Table      *m_pEnums;
    int                         m_MethodCount;
    int                         m_PropCount;
    int                         m_EnumCount;

};

#endif // METAOBJECT_H
