#ifndef AOBJECTSYSTEM_H
#define AOBJECTSYSTEM_H

#include <unordered_map>
#include <set>
#include <string>
#include <memory>

#include "aobject.h"

class AMetaObject;
class AObjectSystemPrivate;

class NEXT_LIBRARY_EXPORT AObjectSystem : public AObject {
public:
    typedef unordered_map<string, AObject *>            ObjectMap;
    typedef unordered_map<string, const AMetaObject *>  FactoryMap;
    typedef unordered_map<string, string>               GroupMap;

public:
    AObjectSystem                       (const string &name = "system");
    ~AObjectSystem                      ();

    virtual int32_t                     exec                    ();

    GroupMap                            factories               () const;

    uint32_t                            nextID                  ();

public:
    static AObjectSystem               *instance                ();

    static AObject                     *objectCreate            (const string &uri, const string &name = string(), AObject *parent = 0);

    template<typename T>
    static T                           *objectCreate            (const string &name = string(), AObject *parent = 0) {
        return dynamic_cast<T *>(objectCreate(T::metaClass()->name(), name, parent));
    }

    template<typename T>
    static void                         factoryAdd              (const string &group, const AMetaObject *meta) {
        string name = T::metaClass()->name();
        string uri  = string("thor://") + group + "/" + name;
        AObjectSystem *inst = AObjectSystem::instance();
        inst->factoryAdd(name, uri, meta);

        name += " *";
        if(AMetaType::type(name.c_str()) == 0) {
            registerMetaType<T *>(name.c_str());
        }
    }

    template<typename T>
    static void                         factoryRemove           (const string &group) {
        const char *name    = T::metaClass()->name();
        AObjectSystem::instance()->factoryRemove(name, string("thor://") + group + "/" + name);
    }

    static AVariant                     toVariant               (const AObject *object);
    static AObject                     *toObject                (const AVariant &variant);

private:
    friend class ObjectSystemTest;

    void                                factoryAdd              (const string &name, const string &uri, const AMetaObject *meta);

    void                                factoryRemove           (const string &name, const string &uri);

    void                                factoryClear            ();

    AObjectSystemPrivate               *p_ptr;
};

#endif // AOBJECTSYSTEM_H
