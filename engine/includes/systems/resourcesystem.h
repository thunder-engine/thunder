#ifndef RESOURCESYSTEM_H
#define RESOURCESYSTEM_H

#include "system.h"

class Resource;

class ENGINE_EXPORT ResourceSystem : public System {
public:
    typedef unordered_map<string, pair<string, string>> DictionaryMap;

public:
    ResourceSystem();

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

    void update(World *) override;

    int threadPolicy() const override;

    Object *instantiateObject(const MetaObject *meta, const string &name, Object *parent) override;

    void processState(Resource *resource);

private:
    mutable ResourceSystem::DictionaryMap  m_indexMap;
    unordered_map<string, Resource *> m_resourceCache;
    unordered_map<Resource *, string> m_referenceCache;

    set<Resource *> m_deleteList;

};

#endif // RESOURCESYSTEM_H
