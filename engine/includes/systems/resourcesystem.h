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

    string reference(Resource *resource);

    Resource *resource(string &path) const;

    DictionaryMap &indices() const;

private:
    bool init() override;

    const char *name() const override;

    void update(Scene *) override;

    int threadPolicy() const override;

    void deleteFromCahe(Resource *resource);

    void processState(Resource *resource);

private:
    ResourceSystemPrivate *p_ptr;
};

#endif // RESOURCESYSTEM_H
