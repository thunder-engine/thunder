#include "systems/resourcesystem.h"

#include <bson.h>
#include <json.h>
#include <log.h>

#include "engine.h"

#include "resources/resource.h"

class ResourceSystemPrivate {
public:
    ResourceSystem::DictionaryMap  m_IndexMap;
    unordered_map<string, Resource*> m_ResourceCache;
    unordered_map<Resource*, string> m_ReferenceCache;

    list<Resource *> m_DeleteList;
};

ResourceSystem::ResourceSystem() :
    p_ptr(new ResourceSystemPrivate) {

}

ResourceSystem::~ResourceSystem() {
    delete p_ptr;
}

bool ResourceSystem::init() {
    return true;
}

const char *ResourceSystem::name() const {
    return "ResourceSystem";
}

void ResourceSystem::update(Scene *) {
    PROFILE_FUNCTION();

    for(auto it = p_ptr->m_ResourceCache.begin(); it != p_ptr->m_ResourceCache.end();) {
        processState(it->second);
        ++it;
    }

    for(auto it : p_ptr->m_DeleteList) {
        deleteFromCahe(it);
        delete it;
    }
    p_ptr->m_DeleteList.clear();
}

int ResourceSystem::threadPolicy() const {
    return Pool;
}

void ResourceSystem::setResource(Resource *object, const string &uuid) {
    PROFILE_FUNCTION();

    p_ptr->m_ResourceCache[uuid] = object;
    p_ptr->m_ReferenceCache[object] = uuid;
}

bool ResourceSystem::isResourceExist(const string &path) {
    PROFILE_FUNCTION();

    auto it = p_ptr->m_IndexMap.find(path);
    return (it != p_ptr->m_IndexMap.end());
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
            data.resize(file->fsize(fp));
            file->fread(&data[0], data.size(), 1, fp);
            file->fclose(fp);

            Variant var = Bson::load(data);
            if(!var.isValid()) {
                var = Json::load(string(data.begin(), data.end()));
            }
            if(var.isValid()) {
                Object *res = Engine::toObject(var);
                if(res) {
                    Resource *resource = dynamic_cast<Resource *>(res);
                    if(resource) {
                        setResource(resource, uuid);
                        resource->switchState(Resource::ToBeUpdated);
                        return resource;
                    }
                }
            }
        }

    }
    return nullptr;
}

void ResourceSystem::unloadResource(Resource *resource, bool force) {
    PROFILE_FUNCTION();
    if(resource) {
        resource->switchState(Resource::Suspend);
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

string ResourceSystem::reference(Resource *resource) {
    PROFILE_FUNCTION();
    auto it = p_ptr->m_ReferenceCache.find(resource);
    if(it != p_ptr->m_ReferenceCache.end()) {
        return it->second;
    }
    return string();
}

ResourceSystem::DictionaryMap &ResourceSystem::indices() const {
    return p_ptr->m_IndexMap;
}

void ResourceSystem::deleteFromCahe(Resource *resource) {
    PROFILE_FUNCTION();
    auto ref = p_ptr->m_ReferenceCache.find(resource);
    if(ref != p_ptr->m_ReferenceCache.end()) {
        auto res = p_ptr->m_ResourceCache.find(ref->second);
        if(res != p_ptr->m_ResourceCache.end()) {
            p_ptr->m_ResourceCache.erase(res);
        }
        p_ptr->m_ReferenceCache.erase(ref);
    }
}

typedef list<Object *> List;
static void enumObjects(Object *object, List &list) {
    PROFILE_FUNCTION();
    list.push_back(object);
    for(const auto &it : object->getChildren()) {
        enumObjects(it, list);
    }
}

void ResourceSystem::processState(Resource *resource) {
    if(resource) {
        switch(resource->state()) {
            case Resource::Loading: {
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

                        List deleteObjects;
                        enumObjects(resource, deleteObjects);

                        VariantList objects = var.toList();
                        auto del = deleteObjects.begin();
                        for(auto &obj : objects) {
                            VariantList fields = obj.toList();
                            auto it = std::next(fields.begin(), 1);
                            uint32_t uuid = it->toInt();

                            Object *object = Engine::findObject(uuid, resource);
                            if(object) {
                                it = std::next(fields.begin(), 4);
                                VariantMap &properties = *(reinterpret_cast<VariantMap *>((*it).data()));
                                for(const auto &prop : properties) {
                                    Variant v  = prop.second;
                                    if(v.type() < MetaType::USERTYPE) {
                                        object->setProperty(prop.first.c_str(), v);
                                    }
                                }

                                del = deleteObjects.erase(del);

                                Resource *res = dynamic_cast<Resource *>(object);
                                if(res) {
                                    res->loadUserData(fields.back().toMap());
                                }
                            } else {
                                if(fields.begin()->toString() != "Prefab") {
                                    VariantList list;
                                    list.push_back(obj);
                                    Engine::toObject(list, resource);
                                }
                            }
                        }

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
            case Resource::Suspend: {
                resource->switchState(Resource::Unloading);
            } break;
            case Resource::ToBeDeleted: {
                p_ptr->m_DeleteList.push_back(resource);
            } break;
            default: break;
        }
    }
}

Resource *ResourceSystem::resource(string &path) const {
    {
        auto it = p_ptr->m_IndexMap.find(path);
        if(it != p_ptr->m_IndexMap.end()) {
            path = it->second.second;
        }
    }
    {
        auto it = p_ptr->m_ResourceCache.find(path);
        if(it != p_ptr->m_ResourceCache.end() && it->second) {
            return it->second;
        }
    }
    return nullptr;
}
