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

#include <global.h>

#include "variant.h"

#include "metaobject.h"
#include "event.h"

#ifndef Q_QDOC
#define A_REGISTER(Class, Super, Group) \
    A_OBJECT(Class, Super) \
public: \
    static void registerClassFactory (ObjectSystem *system) { \
        REGISTER_META_TYPE(Class); \
        system->factoryAdd<Class>(#Group, Class::metaClass()); \
    } \
    static void unregisterClassFactory (ObjectSystem *system) { \
        UNREGISTER_META_TYPE(Class); \
        system->factoryRemove<Class>(#Group); \
    }
#else
#define A_REGISTER(Class, Super, Group)
#endif

#ifndef Q_QDOC
#define A_OVERRIDE(Class, Super, Group) \
    A_OBJECT_OVERRIDE(Class, Super) \
public: \
    static void registerClassFactory (ObjectSystem *system) { \
        system->factoryAdd<Super>(#Group, Class::metaClass()); \
    } \
    static void unregisterClassFactory (ObjectSystem *system) { \
        system->factoryRemove<Super>(#Group); \
        system->factoryAdd<Super>(#Group, Super::metaClass()); \
    } \
    string typeName () const override { \
        return Super::metaClass()->name(); \
    }
#else
#define A_OVERRIDE(Class, Super, Group)
#endif

class ObjectPrivate;
class ObjectSystem;

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

    virtual Object                 *clone                       (Object *parent = nullptr);

    Object                         *parent                      () const;

    string                          name                        () const;

    uint32_t                        uuid                        () const;

    static bool                     connect                     (Object *sender, const char *signal, Object *receiver, const char *method);
    static void                     disconnect                  (Object *sender, const char *signal, Object *receiver, const char *method);

    void                            deleteLater                 ();

    void                            setName                     (const string &name);

    Object                         *find                        (const string &path);

    template<typename T>
    T                               findChild                   (bool recursive = true) {
        for(auto it : getChildren()) {
            Object *object = it;
            T result = dynamic_cast<T>(object);
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
            T object = dynamic_cast<T>(component);
            if(object) {
                result.push_back(object);
            }

            if(recursive) {
                list<T> childs = component->findChildren<T>(recursive);
                result.insert(result.end(), childs.begin(), childs.end());
            }
        }
        return result;
    }

// Virtual members
public:
    virtual const ObjectList       &getChildren                 () const;
    virtual const LinkList         &getReceivers                () const;

    virtual void                    setParent                   (Object *parent, bool force = false);
    virtual string                  typeName                    () const;
    virtual Variant                 property                    (const char *name) const;
    virtual void                    setProperty                 (const char *name, const Variant &value);

    virtual bool                    event                       (Event *event);

    virtual bool                    isSerializable              () const;

    uint32_t                        clonedFrom                  () const;
    virtual void                    clearCloneRef               ();

    virtual bool                    operator==                  (const Object &) const final { return false; }
    virtual bool                    operator!=                  (const Object &) const final { return false; }

protected:
    virtual void                    loadData                    (const VariantList &data);
    virtual void                    loadObjectData              (const VariantMap &data);
    virtual void                    loadUserData                (const VariantMap &data);

    virtual VariantList             saveData                    () const;
    virtual VariantMap              saveUserData                () const;

    void                            emitSignal                  (const char *signal, const Variant &args = Variant());
    void                            postEvent                   (Event *event);

    VariantList                     serializeData               (const MetaObject *meta) const;

    virtual void                    addChild                    (Object *child);
    void                            removeChild                 (Object *child);

    Object                         *sender                      () const;

    ObjectSystem                   *system                      () const;

private:
    friend class ObjectTest;
    friend class ThreadPoolPrivate;
    friend class ObjectSystem;

private:
    virtual void                    processEvents               ();

    void                            setUUID                     (uint32_t id);

    void                            setSystem                   (ObjectSystem *system);
};

#endif // Object_H
