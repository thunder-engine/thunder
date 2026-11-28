/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef RESOURCESYSTEM_H
#define RESOURCESYSTEM_H

#include "system.h"
#include "resource.h"

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
    typedef std::unordered_map<TString, TString> Aliases;

    typedef void (*BundleUpdatedCallback)(const TString &path, bool unload, void *object);

public:
    ResourceSystem();
    ~ResourceSystem();

    bool loadBundle(const TString &path);
    bool unloadBundle(const TString &path);

    Resource *loadResource(const TString &path, uint32_t lod = 0);

    Resource *loadResourceAsync(const TString &path, uint32_t lod = 0);

    void unloadResource(Resource *resource, bool force = false);

    void reloadResource(Resource *resource, bool force = false);

    void releaseAll();

    bool isResourceExist(const TString &path) const;

    TString reference(const TString &path) const;
    TString reference(Resource *resource) const;

    Resource *resource(TString &path) const;
    void setResource(Resource *object, const TString &uuid);

    Dictionary &indices();
    Aliases &aliases();

    void deleteFromCahe(Resource *resource);

    void setCleanImport(bool flag);

    int indexOf(const TString &type) const;

    void subscribe(BundleUpdatedCallback callback, void *object);
    void unsubscribe(void *object);

    static void loadResourceData(Resource *resource, const VariantMap &data);
    static VariantMap saveResourceData(Resource *resource);

    static void loadResourceProperties(Resource *resource, const VariantMap &data);
    static VariantMap saveResourceProperties(Resource *resource);

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
    ResourceSystem::Aliases m_aliases;

    std::unordered_map<TString, Resource *> m_resourceCache;
    std::unordered_map<Resource *, TString> m_referenceCache;

    StringList m_types;

    ObjectList m_deleteList;

    bool m_clean;

};

#endif // RESOURCESYSTEM_H
