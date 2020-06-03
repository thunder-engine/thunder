#ifndef OBJECTSYSTEM_H
#define OBJECTSYSTEM_H

#include <unordered_map>
#include <set>
#include <string>
#include <memory>

#include "object.h"

class MetaObject;

class NEXT_LIBRARY_EXPORT ObjectSystem : public Object {
public:
    typedef pair<const MetaObject *, ObjectSystem *>    FactoryPair;
    typedef unordered_map<string, FactoryPair>          FactoryMap;
    typedef unordered_map<string, string>               GroupMap;

public:
    ObjectSystem                        ();
    ~ObjectSystem                       () override;

    GroupMap                            factories               () const;

    static FactoryPair                 *metaFactory             (const string &uri);

    void                                processEvents           () override;

public:
    template<typename T>
    static T                           *objectCreate            (const string &name = string(), Object *parent = nullptr) {
        return dynamic_cast<T *>(objectCreate(T::metaClass()->name(), name, parent));
    }

    static Object                      *objectCreate            (const string &uri, const string &name = string(), Object *parent = nullptr);

    template<typename T>
    void                                factoryAdd              (const string &group, const MetaObject *meta) {
        string name = T::metaClass()->name();
        factoryAdd(name, string("thor://") + group + "/" + name, meta);
    }

    template<typename T>
    void                                factoryRemove           (const string &group) {
        const char *name = T::metaClass()->name();
        factoryRemove(name, string("thor://") + group + "/" + name);
    }

    static Variant                      toVariant               (const Object *object);
    static Object                      *toObject                (const Variant &variant, Object *root = nullptr);

    static uint32_t                     generateUUID            ();

    static void                         replaceUUID             (Object *object, uint32_t uuid);

    static Object                      *findRoot                (Object *object);

    static Object                      *findObject              (uint32_t uuid, Object *root);

protected:
    void                                factoryAdd              (const string &name, const string &uri, const MetaObject *meta);

    void                                factoryRemove           (const string &name, const string &uri);

private:
    friend class ObjectSystemTest;
    friend class Object;

    void                                addObject               (Object *object);

    void                                removeObject            (Object *object);

    void                                suspendObject           (Object *object);

protected:
    Object::ObjectList                  m_ObjectList;

    Object*                             m_SuspendObject;

};

#endif // OBJECTSYSTEM_H
