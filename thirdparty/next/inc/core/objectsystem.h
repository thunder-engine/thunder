#ifndef OBJECTSYSTEM_H
#define OBJECTSYSTEM_H

#include <unordered_map>
#include <set>
#include <string>
#include <memory>

#include "object.h"

class MetaObject;
class ObjectSystemPrivate;

class NEXT_LIBRARY_EXPORT ObjectSystem : public Object {
public:
    typedef unordered_map<string, const MetaObject *>   FactoryMap;
    typedef unordered_map<string, string>               GroupMap;

public:
    ObjectSystem                        (const string &name = "system");
    ~ObjectSystem                       ();

    virtual int32_t                     exec                    ();

    GroupMap                            factories               () const;

public:
    static ObjectSystem                *instance                ();

    static Object                      *objectCreate            (const string &uri, const string &name = string(), Object *parent = 0);

    template<typename T>
    static T                           *objectCreate            (const string &name = string(), Object *parent = 0) {
        return dynamic_cast<T *>(objectCreate(T::metaClass()->name(), name, parent));
    }

    template<typename T>
    static void                         factoryAdd              (const string &group, const MetaObject *meta) {
        string name = T::metaClass()->name();
        string uri  = string("thor://") + group + "/" + name;
        ObjectSystem *inst = ObjectSystem::instance();
        inst->factoryAdd(name, uri, meta);

        name += " *";
        if(MetaType::type(name.c_str()) == 0) {
            registerMetaType<T *>(name.c_str());
        }
    }

    template<typename T>
    static void                         factoryRemove           (const string &group) {
        const char *name    = T::metaClass()->name();
        ObjectSystem::instance()->factoryRemove(name, string("thor://") + group + "/" + name);
    }

    static Variant                      toVariant               (const Object *object);
    static Object                      *toObject                (const Variant &variant);

    static uint32_t                     generateUUID            (const Object *object);

private:
    friend class ObjectSystemTest;

    void                                factoryAdd              (const string &name, const string &uri, const MetaObject *meta);

    void                                factoryRemove           (const string &name, const string &uri);

    void                                factoryClear            ();

    ObjectSystemPrivate                *p_ptr;
};

#endif // OBJECTSYSTEM_H
