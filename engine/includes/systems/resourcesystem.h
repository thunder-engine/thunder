#ifndef RESOURCESYSTEM_H
#define RESOURCESYSTEM_H

#include "system.h"
#include "resource.h"

class ENGINE_EXPORT ResourceSystem : public System {
public:
    typedef std::unordered_map<String, std::pair<String, String>> DictionaryMap;

public:
    ResourceSystem();

    void setResource(Resource *object, const String &uuid);

    bool isResourceExist(const String &path);

    Resource *loadResource(const String &path);

    void unloadResource(Resource *resource, bool force = false);

    void reloadResource(Resource *resource, bool force = false);

    void releaseAll();

    String reference(Resource *resource);

    Resource *resource(String &path) const;

    DictionaryMap &indices() const;

    void deleteFromCahe(Resource *resource);

private:
    bool init() override;

    void update(World *) override;

    int threadPolicy() const override;

    Object *instantiateObject(const MetaObject *meta, const String &name, Object *parent) override;

    void processState(Resource *resource);

private:
    mutable ResourceSystem::DictionaryMap  m_indexMap;
    std::unordered_map<String, Resource *> m_resourceCache;
    std::unordered_map<Resource *, String> m_referenceCache;

    ObjectList m_deleteList;

};

#endif // RESOURCESYSTEM_H
