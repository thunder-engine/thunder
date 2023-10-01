#ifndef OBJECTSYSTEM_H
#define OBJECTSYSTEM_H

#include <unordered_map>
#include <set>
#include <string>
#include <memory>
#include <thread>

#include "object.h"

class MetaObject;

class NEXT_LIBRARY_EXPORT ObjectSystem : public Object {
public:
    typedef pair<const MetaObject *, ObjectSystem *> FactoryPair;
    typedef unordered_map<string, FactoryPair> FactoryMap;
    typedef unordered_map<string, string> GroupMap;

public:
    ObjectSystem();
    ~ObjectSystem() override;

    static GroupMap factories();

    static FactoryPair *metaFactory(const string &uri);

    void processEvents() override;

    bool compareTreads(ObjectSystem *system) const;

    virtual ObjectList getAllObjectsByType(const string &type) const;

public:
    template<typename T>
    static T *objectCreate(const string &name = string(), Object *parent = nullptr) {
        return dynamic_cast<T *>(objectCreate(T::metaClass()->name(), name, parent));
    }

    static Object *objectCreate(const string &uri, const string &name = string(), Object *parent = nullptr);

    template<typename T>
    void factoryAdd(const string &group, const MetaObject *meta) {
        string name = T::metaClass()->name();
        factoryAdd(name, string("thor://") + group + "/" + name, meta);
    }

    template<typename T>
    void factoryRemove(const string &group) {
        const char *name = T::metaClass()->name();
        factoryRemove(name, string("thor://") + group + "/" + name);
    }

    static Variant toVariant(const Object *object, bool force = false);
    static Object *toObject(const Variant &variant, Object *parent = nullptr, const string &name = string());

    static uint32_t generateUUID();

    static void replaceUUID(Object *object, uint32_t uuid);

    static Object *findRoot(Object *object);

    static Object *findObject(uint32_t uuid, Object *root);

protected:
    void factoryAdd(const string &name, const string &uri, const MetaObject *meta);

    void factoryRemove(const string &name, const string &uri);

    void deleteAllObjects();

    virtual Object *instantiateObject(const MetaObject *meta, const string &name, Object *parent);

    virtual void removeObject(Object *object);

private:
    friend class ObjectSystemTest;
    friend class Object;

    void addObject(Object *object);

    void suspendObject(Object *object);

protected:
    Object::ObjectList m_objectList;

    Object *m_suspendObject;


    thread::id m_threadId;

};

#endif // OBJECTSYSTEM_H
