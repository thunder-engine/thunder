#ifndef RESOURCESYSTEM_H
#define RESOURCESYSTEM_H

#include "system.h"
#include "resource.h"

class ENGINE_EXPORT ResourceSystem : public System {
public:
    typedef std::unordered_map<std::string, std::pair<std::string, std::string>> DictionaryMap;

public:
    ResourceSystem();

    void setResource(Resource *object, const std::string &uuid);

    bool isResourceExist(const std::string &path);

    Resource *loadResource(const std::string &path);

    void unloadResource(Resource *resource, bool force = false);

    void reloadResource(Resource *resource, bool force = false);

    void releaseAll();

    std::string reference(Resource *resource);

    Resource *resource(std::string &path) const;

    DictionaryMap &indices() const;

    void deleteFromCahe(Resource *resource);

private:
    bool init() override;

    void update(World *) override;

    int threadPolicy() const override;

    Object *instantiateObject(const MetaObject *meta, const std::string &name, Object *parent) override;

    void processState(Resource *resource);

private:
    mutable ResourceSystem::DictionaryMap  m_indexMap;
    std::unordered_map<std::string, Resource *> m_resourceCache;
    std::unordered_map<Resource *, std::string> m_referenceCache;

    ObjectList m_deleteList;

};

#endif // RESOURCESYSTEM_H
