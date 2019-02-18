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
    ~ObjectSystem                       ();

    virtual void                        update                  ();

    GroupMap                            factories               () const;

    static FactoryPair                 *metaFactory             (const string &uri);

public:
    template<typename T>
    static T                           *objectCreate            (const string &name = string(), Object *parent = 0) {
        return dynamic_cast<T *>(objectCreateImpl(T::metaClass()->name(), name, parent));
    }

    static Object                      *objectCreateImpl        (const string &uri, const string &name = string(), Object *parent = 0);

    template<typename T>
    void                                factoryAdd              (const string &group, const MetaObject *meta) {
        string name = T::metaClass()->name();
        factoryAdd(name, string("thor://") + group + "/" + name, meta);
    }

    template<typename T>
    void                                factoryRemove           (const string &group) {
        const char *name    = T::metaClass()->name();
        factoryRemove(name, string("thor://") + group + "/" + name);
    }

    static Variant                      toVariant               (const Object *object);
    static Object                      *toObject                (const Variant &variant);

    static uint32_t                     generateUID             ();

protected:
    static void                         processObject           (Object *object);

private:
    friend class ObjectSystemTest;
    friend class Object;

    void                                removeObject            (Object *object);

    void                                factoryAdd              (const string &name, const string &uri, const MetaObject *meta);

    void                                factoryRemove           (const string &name, const string &uri);

private:
    Object::ObjectList                  m_List;

};

#endif // OBJECTSYSTEM_H
