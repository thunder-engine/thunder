#ifndef RESOURCESYSTEM_H
#define RESOURCESYSTEM_H

#include "system.h"
#include "resource.h"

#include <set>

class ENGINE_EXPORT ResourceSystem : public System {
public:
    struct ResourceInfo {
        TString bundle;

        TString type;

        TString uuid;

        TString md5;

        uint32_t id = 0;
    };

    typedef std::unordered_map<TString, ResourceInfo> Dictionary;

    typedef void (*BundleUpdatedCallback)(const TString &path, bool unload, void *object);

public:
    ResourceSystem();
    ~ResourceSystem();

    bool loadBundle(const TString &path);
    bool unloadBundle(const TString &path);

    Resource *loadResource(const TString &path);

    Resource *loadResourceAsync(const TString &path);

    void unloadResource(Resource *resource, bool force = false);

    void reloadResource(Resource *resource, bool force = false);

    void releaseAll();

    bool isResourceExist(const TString &path) const;

    TString reference(const TString &path) const;
    TString reference(Resource *resource) const;

    Resource *resource(TString &path) const;
    void setResource(Resource *object, const TString &uuid);

    Dictionary &indices();

    void deleteFromCahe(Resource *resource);

    void setCleanImport(bool flag);

    int indexOf(const TString &type) const;

    void subscribe(BundleUpdatedCallback callback, void *object);
    void unsubscribe(void *object);

private:
    void update(World *) override;

    int threadPolicy() const override;

    void factoryAdd(const TString &name, const TString &url, const MetaObject *meta) override;

    Object *instantiateObject(const MetaObject *meta, const TString &name, Object *parent) override;

    void processState(Resource *resource);

    void removeObject(Object *object) override;

private:
    std::list<std::pair<ResourceSystem::BundleUpdatedCallback, void *>> m_observers;

    ResourceSystem::Dictionary m_indexMap;

    std::unordered_map<TString, Resource *> m_resourceCache;
    std::unordered_map<Resource *, TString> m_referenceCache;

    StringList m_types;

    ObjectList m_deleteList;

    bool m_clean;

};

#endif // RESOURCESYSTEM_H
