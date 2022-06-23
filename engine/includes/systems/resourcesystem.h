#ifndef RESOURCESYSTEM_H
#define RESOURCESYSTEM_H

#include "system.h"

class Resource;

class ResourceSystemPrivate;

class ENGINE_EXPORT ResourceSystem : public System {
public:
    typedef unordered_map<string, pair<string, string>> DictionaryMap;

public:
    ResourceSystem();
    ~ResourceSystem() override;

    void setResource(Resource *object, const string &uuid);

    bool isResourceExist(const string &path);

    Resource *loadResource(const string &path);

    void unloadResource(Resource *resource, bool force = false);

    void reloadResource(Resource *resource, bool force = false);

    void releaseAll();

    string reference(Resource *resource);

    Resource *resource(string &path) const;

    DictionaryMap &indices() const;

    void deleteFromCahe(Resource *resource);

private:
    bool init() override;

    const char *name() const override;

    void update(Scene *) override;

    int threadPolicy() const override;

    Object *instantiateObject(const MetaObject *meta, const string &name, Object *parent) override;

    void processState(Resource *resource);

private:
    ResourceSystemPrivate *p_ptr;
};

#endif // RESOURCESYSTEM_H
