/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef OBJECTSYSTEM_H
#define OBJECTSYSTEM_H

#include <unordered_map>
#include <thread>

#include <astring.h>
#include <object.h>

class MetaObject;
class Invalid;

class NEXT_LIBRARY_EXPORT ObjectSystem : public Object {
public:
    typedef std::pair<const MetaObject *, ObjectSystem *> FactoryPair;
    typedef std::map<TString, FactoryPair> FactoryMap;
    typedef std::map<TString, TString> GroupMap;
    typedef std::unordered_map<uint32_t, Object *> ObjectMap;

public:
    ObjectSystem();
    ~ObjectSystem() override;

    static GroupMap factories();

    static FactoryPair *metaFactory(const TString &url);

    static void blockObjectCache(bool block);

    void processEvents() override;

    bool compareTreads(ObjectSystem *system) const;

    virtual ObjectList getAllObjectsByType(const TString &type) const;

public:
    template<typename T>
    static T *objectCreate(const TString &name = TString(), Object *parent = nullptr) {
        return dynamic_cast<T *>(objectCreate(T::metaClass()->name(), name, parent));
    }

    static Object *objectCreate(const TString &url, const TString &name = TString(), Object *parent = nullptr, uint32_t id = 0);

    template<typename T>
    void factoryAdd(const TString &group, const MetaObject *meta) {
        TString name = T::metaClass()->name();
        factoryAdd(name, TString("thor://") + group + "/" + name, meta);
    }

    template<typename T>
    void factoryRemove(const TString &group) {
        const char *name = T::metaClass()->name();
        factoryRemove(name, TString("thor://") + group + "/" + name);
    }

    static Variant toVariant(const Object *object, bool force = false);
    static Object *toObject(const Variant &variant, Object *parent = nullptr, const TString &name = TString());

    static uint32_t generateUUID();

    static void replaceUUID(Object *object, uint32_t uuid);
    static void replaceClonedUUID(Object *object, uint32_t uuid);

    static Object *findRoot(Object *object);

    static Object *findObject(uint32_t uuid);

    static void unregisterObject(Object *object);

    static Object::ObjectList &invalidObjects();

    static void removeInvalid(Invalid *invalid);

    static void notify(Object *receiver, Event *event);

protected:
    virtual void factoryAdd(const TString &name, const TString &url, const MetaObject *meta);

    void factoryRemove(const TString &name, const TString &url);

    void deleteAllObjects();

    virtual Object *instantiateObject(const MetaObject *meta, const TString &name, Object *parent);

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
