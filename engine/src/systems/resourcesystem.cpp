#include "systems/resourcesystem.h"

#include <bson.h>
#include <json.h>
#include <log.h>

#include "file.h"

#include "resources/resource.h"

ResourceSystem::ResourceSystem() {
    setName("ResourceSystem");

}

bool ResourceSystem::init() {
    return true;
}

void ResourceSystem::update(World *) {
    PROFILE_FUNCTION();

    for(auto it = m_referenceCache.begin(); it != m_referenceCache.end();) {
        processState(it->first);
        ++it;
    }

    for(auto it : m_deleteList) {
        delete it;
    }
    m_deleteList.clear();
}

int ResourceSystem::threadPolicy() const {
    return Main;
}

void ResourceSystem::setResource(Resource *object, const string &uuid) {
    PROFILE_FUNCTION();

    m_resourceCache[uuid] = object;
    m_referenceCache[object] = uuid;
}

bool ResourceSystem::isResourceExist(const string &path) {
    PROFILE_FUNCTION();

    auto it = m_indexMap.find(path);
    return (it != m_indexMap.end());
}

Resource *ResourceSystem::loadResource(const string &path) {
    PROFILE_FUNCTION();

    if(!path.empty()) {
        string uuid = path;
        Resource *object = resource(uuid);
        if(object) {
            return object;
        }

        File *file = Engine::file();
        _FILE *fp = file->fopen(uuid.c_str(), "r");
        if(fp) {
            ByteArray data;
            size_t size = file->fsize(fp);
            data.resize(size);
            file->fread(&data[0], data.size(), 1, fp);
            file->fclose(fp);

            Variant var = Bson::load(data);
            if(!var.isValid()) {
                var = Json::load(string(data.begin(), data.end()));
            }
            if(var.isValid()) {
                return static_cast<Resource *>(Engine::toObject(var, nullptr, uuid));
            }
        }

    }
    return nullptr;
}

void ResourceSystem::unloadResource(Resource *resource, bool force) {
    PROFILE_FUNCTION();
    if(resource) {
        resource->switchState(ResourceState::Unloading);
        if(force) {
            processState(resource);
        }
    }
}

void ResourceSystem::reloadResource(Resource *resource, bool force) {
    PROFILE_FUNCTION();
    if(resource) {
        if(force) {
            resource->switchState(ResourceState::Loading);
            processState(resource);
        } else {
            resource->switchState(ResourceState::ToBeUpdated);
        }
    }
}

void ResourceSystem::releaseAll() {
    for(auto it = m_referenceCache.begin(); it != m_referenceCache.end();) {
        if(it->first->isUnloadable()) {
            unloadResource(it->first, true);
            it->first->switchState(ResourceState::ToBeUpdated);
        }
        ++it;
    }
}

string ResourceSystem::reference(Resource *resource) {
    PROFILE_FUNCTION();
    auto it = m_referenceCache.find(resource);
    if(it != m_referenceCache.end()) {
        return it->second;
    }
    return string();
}

ResourceSystem::DictionaryMap &ResourceSystem::indices() const {
    return m_indexMap;
}

void ResourceSystem::deleteFromCahe(Resource *resource) {
    PROFILE_FUNCTION();
    auto ref = m_referenceCache.find(resource);
    if(ref != m_referenceCache.end()) {
        auto res = m_resourceCache.find(ref->second);
        if(res != m_resourceCache.end()) {
            m_resourceCache.erase(res);
        }
        m_referenceCache.erase(ref);
    }
}

void ResourceSystem::processState(Resource *resource) {
    if(resource) {
        switch(resource->state()) {
            case ResourceState::Loading: {
                string uuid = reference(resource);
                if(!uuid.empty()) {
                    File *file = Engine::file();
                    _FILE *fp = file->fopen(uuid.c_str(), "r");
                    if(fp) {
                        ByteArray data;
                        data.resize(file->fsize(fp));
                        file->fread(&data[0], data.size(), 1, fp);
                        file->fclose(fp);

                        Variant var = Bson::load(data);
                        if(!var.isValid()) {
                            var = Json::load(string(data.begin(), data.end()));
                        }

                        ObjectList deleteObjects;
                        enumObjects(resource, deleteObjects);

                        VariantList objects = var.toList();
                        auto delIt = deleteObjects.begin();
                        bool first = true;
                        for(auto &obj : objects) {
                            VariantList fields = obj.toList();
                            auto it = std::next(fields.begin(), 1);
                            uint32_t id = it->toInt();

                            Object *object = resource;
                            if(!first) {
                                object = Engine::findObject(id, resource);
                            } else {
                                first = false;
                            }

                            if(object) {
                                it = std::next(fields.begin(), 4);
                                VariantMap &properties = *(reinterpret_cast<VariantMap *>((*it).data()));
                                for(const auto &prop : properties) {
                                    Variant v = prop.second;
                                    if(v.type() < MetaType::USERTYPE) {
                                        object->setProperty(prop.first.c_str(), v);
                                    }
                                }

                                object->loadUserData(fields.back().toMap());

                                delIt = deleteObjects.erase(delIt);
                            } else {
                                VariantList list;
                                list.push_back(obj);
                                Engine::toObject(list, resource, uuid);
                            }
                        }

                        deleteObjects.reverse();
                        for(auto toDel : deleteObjects) {
                            delete toDel;
                        }

                        resource->switchState(ResourceState::ToBeUpdated);
                    } else {
                        Log(Log::ERR) << "Unable to load resource: " << uuid.c_str();
                        resource->setState(ResourceState::Invalid);
                    }
                }
            } break;
            case ResourceState::Suspend: { /// \todo Don't delete reseource imidiately Cache pattern implementation required
                resource->switchState(ResourceState::Unloading);
            } break;
            case ResourceState::ToBeDeleted: {
                m_deleteList.insert(resource);
            } break;
            default: break;
        }
    }
}

Resource *ResourceSystem::resource(string &path) const {
    {
        auto it = m_indexMap.find(path);
        if(it != m_indexMap.end()) {
            path = it->second.second;
        }
    }
    {
        auto it = m_resourceCache.find(path);
        if(it != m_resourceCache.end() && it->second) {
            return it->second;
        }
    }
    return nullptr;
}

Object *ResourceSystem::instantiateObject(const MetaObject *meta, const string &name, Object *parent) {
    Object *result = System::instantiateObject(meta, name, parent);

    Resource *resource = dynamic_cast<Resource *>(result);
    if(resource) {
        setResource(resource, name);
        resource->switchState(ResourceState::ToBeUpdated);
    }

    return result;
}
