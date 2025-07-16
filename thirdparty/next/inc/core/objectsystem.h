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

#ifndef OBJECTSYSTEM_H
#define OBJECTSYSTEM_H

#include <unordered_map>
#include <set>
#include <memory>
#include <thread>

#include <astring.h>
#include <object.h>

class MetaObject;

class NEXT_LIBRARY_EXPORT ObjectSystem : public Object {
public:
    typedef std::pair<const MetaObject *, ObjectSystem *> FactoryPair;
    typedef std::map<String, FactoryPair> FactoryMap;
    typedef std::map<String, String> GroupMap;
    typedef std::unordered_map<uint32_t, Object *> ObjectMap;

public:
    ObjectSystem();
    ~ObjectSystem() override;

    static GroupMap factories();

    static FactoryPair *metaFactory(const String &url);

    static void blockObjectCache(bool block);

    void processEvents() override;

    bool compareTreads(ObjectSystem *system) const;

    virtual ObjectList getAllObjectsByType(const String &type) const;

public:
    template<typename T>
    static T *objectCreate(const String &name = String(), Object *parent = nullptr) {
        return dynamic_cast<T *>(objectCreate(T::metaClass()->name(), name, parent));
    }

    static Object *objectCreate(const String &url, const String &name = String(), Object *parent = nullptr);

    template<typename T>
    void factoryAdd(const String &group, const MetaObject *meta) {
        String name = T::metaClass()->name();
        factoryAdd(name, String("thor://") + group + "/" + name, meta);
    }

    template<typename T>
    void factoryRemove(const String &group) {
        const char *name = T::metaClass()->name();
        factoryRemove(name, String("thor://") + group + "/" + name);
    }

    static Variant toVariant(const Object *object, bool force = false);
    static Object *toObject(const Variant &variant, Object *parent = nullptr, const String &name = String());

    static uint32_t generateUUID();

    static void replaceUUID(Object *object, uint32_t uuid);
    static void replaceClonedUUID(Object *object, uint32_t uuid);

    static Object *findRoot(Object *object);

    static Object *findObject(uint32_t uuid);

    static void unregisterObject(Object *object);

protected:
    void factoryAdd(const String &name, const String &url, const MetaObject *meta);

    void factoryRemove(const String &name, const String &url);

    void deleteAllObjects();

    virtual Object *instantiateObject(const MetaObject *meta, const String &name, Object *parent);

    virtual void addObject(Object *object);

    virtual void removeObject(Object *object);

private:
    friend class ObjectSystemTest;
    friend class Object;

protected:
    Object::ObjectList m_objectList;
    Object::ObjectList m_objectToRemove;

    std::thread::id m_threadId;

    static bool s_blockCache;

};

#endif // OBJECTSYSTEM_H
