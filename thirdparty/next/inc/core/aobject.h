/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2016 Evgeniy Prikazchikov
*/

#ifndef AOBJECT_H
#define AOBJECT_H

#include <cstdint>
#include <string>
#include <map>
#include <queue>
#include <list>

#include "acommon.h"

#include "avariant.h"

#include "ametaobject.h"
#include "aevent.h"

#define A_REGISTER(Class, Super, Group) \
    A_OBJECT(Class, Super) \
public: \
    static void                     registerClassFactory    () { \
        AObjectSystem::factoryAdd<Class>(#Group, Class::metaClass()); \
    } \
    static void                     unregisterClassFactory  () { \
        AObjectSystem::factoryRemove<Class>(#Group); \
    }


#define A_OVERRIDE(Class, Super, Group) \
    A_OBJECT(Class, Super) \
public: \
    static void                     registerClassFactory    () { \
        AObjectSystem::factoryAdd<Super>(#Group, Class::metaClass()); \
    } \
    static void                     unregisterClassFactory  () { \
        AObjectSystem::factoryRemove<Super>(#Group); \
        AObjectSystem::factoryAdd<Super>(#Group, Super::metaClass()); \
    } \
    virtual string                  typeName                () const { \
        return Super::metaClass()->name(); \
    }

class AObjectPrivate;

class NEXT_LIBRARY_EXPORT AObject {
public:
    struct Link {
        AObject                    *sender;

        int32_t                     signal;

        AObject                    *receiver;

        int32_t                     method;

        string                      reference;
    };

    typedef list<AObject *>         ObjectList;

    typedef list<Link>              LinkList;

private:
    AObjectPrivate                 *p_ptr;

public:
    AObject                         ();

    virtual ~AObject                ();

    static AObject                 *construct                   ();

    static const AMetaObject       *metaClass                   ();
    virtual const AMetaObject      *metaObject                  () const;

    virtual AObject                *clone                       ();

    AObject                        *parent                      () const;

    string                          name                        () const;

    uint32_t                        uuid                        () const;

    static void                     connect                     (AObject *sender, const char *signal, AObject *receiver, const char *method);
    static void                     disconnect                  (AObject *sender, const char *signal, AObject *receiver, const char *method);

    void                            deleteLater                 ();

    void                            setName                     (const string &value);

    bool                            isEnable                    () const;


    AObject                        *find                        (const string &path);

    template<typename T>
    T                               findChild                   (bool recursive = true) {
        for(auto it : getChildren()) {
            AObject *object = it;
            T result    = dynamic_cast<T>(object);
            if(result) {
                return result;
            } else if(recursive) {
                T child = object->findChild<T>(recursive);
                if(child) {
                    return child;
                }
            }
        }
        return nullptr;
    }

    template<typename T>
    list<T>                         findChildren                (bool recursive = true) {
        list<T> result;
        for(auto it : getChildren()) {
            AObject *component = it;
            T object    = dynamic_cast<T>(component);
            if(object) {
                result.push_back(object);
            }

            if(recursive) {
                list<T> childs   = component->findChildren<T>(recursive);
                result.insert(result.end(), childs.begin(), childs.end());
            }
        }
        return result;
    }

// Virtual members
public:
    virtual const ObjectList       &getChildren                 () const;
    virtual const LinkList         &getReceivers                () const;

    virtual void                    setParent                   (AObject *parent);
    virtual string                  typeName                    () const;
    virtual AVariant                property                    (const char *name) const;
    virtual void                    setProperty                 (const char *name, const AVariant &value);

    virtual void                    setEnable                   (bool state);

    virtual bool                    event                       (AEvent *);

    virtual void                    loadUserData                (const AVariantMap &);

    virtual AVariantMap             saveUserData                () const;

protected:
    void                            addChild                    (AObject *value);
    void                            removeChild                 (AObject *value);

    void                            emitSignal                  (const char *signal, const AVariant &args = AVariant());
    bool                            postEvent                   (AEvent *e);
    void                            processEvents               ();

    virtual void                    onCreated                   ();

    AObject                        *sender                      () const;

private:
    friend class ObjectTest;
    friend class AThreadPoolPrivate;
    friend class AObjectSystem;

private:
    void                            setUUID                     (uint32_t id);

    bool                            operator==                  (const AObject &) const { return false; }
    bool                            operator!=                  (const AObject &) const { return false; }

    AObject                        &operator=                   (AObject &);

    AObject                         (const AObject &);
};

#endif // AOBJECT_H
