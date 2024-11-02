#include "systems/resourcesystem.h"

#include <bson.h>
#include <json.h>
#include <log.h>

#include <algorithm>

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

    while(!m_deleteList.empty()) {
        Object *res = m_deleteList.back();
        m_deleteList.pop_back();

        delete res;
    }
}

int ResourceSystem::threadPolicy() const {
    return Main;
}

void ResourceSystem::setResource(Resource *object, const std::string &uuid) {
    PROFILE_FUNCTION();

    m_resourceCache[uuid] = object;
    m_referenceCache[object] = uuid;
}

bool ResourceSystem::isResourceExist(const std::string &path) {
    PROFILE_FUNCTION();

    auto it = m_indexMap.find(path);
    return (it != m_indexMap.end());
}

Resource *ResourceSystem::loadResource(const std::string &path) {
    PROFILE_FUNCTION();

    if(!path.empty()) {
        std::string uuid = path;
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
                var = Json::load(std::string(data.begin(), data.end()));
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
        resource->switchState(Resource::Unloading);
        if(force) {
            processState(resource);
        }
    }
}

void ResourceSystem::reloadResource(Resource *resource, bool force) {
    PROFILE_FUNCTION();
    if(resource) {
        if(force) {
            resource->switchState(Resource::Loading);
            processState(resource);
        } else {
            resource->switchState(Resource::ToBeUpdated);
        }
    }
}

void ResourceSystem::releaseAll() {
    for(auto it = m_referenceCache.begin(); it != m_referenceCache.end();) {
        if(it->first->isUnloadable()) {
            unloadResource(it->first, true);
            it->first->switchState(Resource::ToBeUpdated);
        }
        ++it;
    }
}

std::string ResourceSystem::reference(Resource *resource) {
    PROFILE_FUNCTION();
    auto it = m_referenceCache.find(resource);
    if(it != m_referenceCache.end()) {
        return it->second;
    }
    return std::string();
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

    for(auto it : m_deleteList) {
        if(it == resource) {
            m_deleteList.remove(it);
            break;
        }
    }
}

void ResourceSystem::processState(Resource *resource) {
    if(resource) {
        switch(resource->state()) {
            case Resource::Loading: {
                std::string uuid = reference(resource);
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
                            var = Json::load(std::string(data.begin(), data.end()));
                        }

                        ObjectList deleteObjects;
                        enumObjects(resource, deleteObjects);

                        VariantList objects = var.toList();
                        auto delIt = deleteObjects.begin();
                        bool first = true;
                        for(auto &obj : objects) {
                            VariantList fields = obj.toList();
                            auto it = std::next(fields.begin(), 1);
                            Object *object = resource;
                            if(!first) {
                                object = Engine::findObject(it->toInt(), resource);
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

                        resource->switchState(Resource::ToBeUpdated);
                    } else {
                        Log(Log::ERR) << "Unable to load resource: " << uuid.c_str();
                        resource->setState(Resource::Invalid);
                    }
                }
            } break;
            case Resource::Suspend: { /// \todo Don't delete resource imidiately Cache pattern implementation required
                //resource->switchState(Resource::Unloading);
            } break;
            case Resource::ToBeDeleted: {
                auto it = std::find(m_deleteList.begin(), m_deleteList.end(), resource);
                if(it == m_deleteList.end()) {
                    m_deleteList.push_back(resource);
                }
            } break;
            default: break;
        }
    }
}

Resource *ResourceSystem::resource(std::string &path) const {
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

Object *ResourceSystem::instantiateObject(const MetaObject *meta, const std::string &name, Object *parent) {
    Object *result = System::instantiateObject(meta, name, parent);

    Resource *resource = dynamic_cast<Resource *>(result);
    if(resource) {
        if(!name.empty()) {
            setResource(resource, name);
        }
        resource->switchState(Resource::ToBeUpdated);
    }

    return result;
}
