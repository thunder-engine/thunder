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

#ifndef Object_H
#define Object_H

#include <cstdint>
#include <string>
#include <map>
#include <queue>
#include <list>

#include "common.h"

#include "variant.h"

#include "metaobject.h"
#include "event.h"

#define A_REGISTER(Class, Super, Group) \
    A_OBJECT(Class, Super) \
public: \
    static void                     registerClassFactory    () { \
        ObjectSystem::factoryAdd<Class>(#Group, Class::metaClass()); \
    } \
    static void                     unregisterClassFactory  () { \
        ObjectSystem::factoryRemove<Class>(#Group); \
    }


#define A_OVERRIDE(Class, Super, Group) \
    A_OBJECT(Class, Super) \
public: \
    static void                     registerClassFactory    () { \
        ObjectSystem::factoryAdd<Super>(#Group, Class::metaClass()); \
    } \
    static void                     unregisterClassFactory  () { \
        ObjectSystem::factoryRemove<Super>(#Group); \
        ObjectSystem::factoryAdd<Super>(#Group, Super::metaClass()); \
    } \
    virtual string                  typeName                () const { \
        return Super::metaClass()->name(); \
    }

class ObjectPrivate;

class NEXT_LIBRARY_EXPORT Object {
public:
    struct Link {
        Object                     *sender;

        int32_t                     signal;

        Object                     *receiver;

        int32_t                     method;
    };

    typedef list<Object *>          ObjectList;

    typedef list<Link>              LinkList;

private:
    ObjectPrivate                  *p_ptr;

public:
    Object                          ();

    virtual ~Object                 ();

    static Object                  *construct                   ();

    static const MetaObject        *metaClass                   ();
    virtual const MetaObject       *metaObject                  () const;

    virtual Object                 *clone                       ();

    Object                         *parent                      () const;

    string                          name                        () const;

    uint32_t                        uuid                        () const;

    static void                     connect                     (Object *sender, const char *signal, Object *receiver, const char *method);
    static void                     disconnect                  (Object *sender, const char *signal, Object *receiver, const char *method);

    void                            deleteLater                 ();

    void                            setName                     (const string &name);

    Object                         *find                        (const string &path);

    template<typename T>
    T                               findChild                   (bool recursive = true) {
        for(auto it : getChildren()) {
            Object *object = it;
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
            Object *component = it;
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

    virtual void                    setParent                   (Object *parent);
    virtual string                  typeName                    () const;
    virtual Variant                 property                    (const char *name) const;
    virtual void                    setProperty                 (const char *name, const Variant &value);

    virtual bool                    event                       (Event *event);

    virtual void                    loadUserData                (const VariantMap &data);

    virtual VariantMap              saveUserData                () const;

protected:
    void                            emitSignal                  (const char *signal, const Variant &args = Variant());
    void                            postEvent                   (Event *event);

    Object                         *sender                      () const;

private:
    friend class ObjectTest;
    friend class ThreadPoolPrivate;
    friend class ObjectSystem;

private:
    void                            processEvents               ();

    void                            addChild                    (Object *value);
    void                            removeChild                 (Object *value);

    void                            setUUID                     (uint32_t id);

    bool                            operator==                  (const Object &) const { return false; }
    bool                            operator!=                  (const Object &) const { return false; }

    Object                         &operator=                   (Object &);

    Object                          (const Object &);
};

#endif // Object_H
