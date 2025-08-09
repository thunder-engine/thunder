#ifndef RESOURCESYSTEM_H
#define RESOURCESYSTEM_H

#include "system.h"
#include "resource.h"

class ENGINE_EXPORT ResourceSystem : public System {
public:
    typedef std::unordered_map<TString, std::pair<TString, TString>> DictionaryMap;

public:
    ResourceSystem();

    void setResource(Resource *object, const TString &uuid);

    Resource *loadResource(const TString &path);

    void unloadResource(Resource *resource, bool force = false);

    void reloadResource(Resource *resource, bool force = false);

    void releaseAll();

    bool isResourceExist(const TString &path) const;

    TString reference(Resource *resource) const;

    Resource *resource(TString &path) const;

    DictionaryMap &indices() const;

    void deleteFromCahe(Resource *resource);

private:
    bool init() override;

    void update(World *) override;

    int threadPolicy() const override;

    Object *instantiateObject(const MetaObject *meta, const TString &name, Object *parent) override;

    void processState(Resource *resource);

private:
    mutable ResourceSystem::DictionaryMap  m_indexMap;
    std::unordered_map<TString, Resource *> m_resourceCache;
    std::unordered_map<Resource *, TString> m_referenceCache;

    ObjectList m_deleteList;

};

#endif // RESOURCESYSTEM_H
