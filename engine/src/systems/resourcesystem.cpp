#include "systems/resourcesystem.h"

#include <bson.h>
#include <json.h>

#include "engine.h"

#include "resources/resource.h"

class ResourceSystemPrivate {
public:
    ResourceSystem::DictionaryMap  m_IndexMap;
    unordered_map<string, Object*> m_ResourceCache;
    unordered_map<Object*, string> m_ReferenceCache;

    Object::ObjectList m_DeleteList;
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
        Resource *resource = dynamic_cast<Resource *>(it->second);
        if(resource) {
            switch(resource->state()) {
                case Resource::Loading: {
                    string uuid = reference(resource);
                    if(!uuid.empty()) {
                        File *file = Engine::file();
                        _FILE *fp = file->_fopen(uuid.c_str(), "r");
                        if(fp) {
                            ByteArray data;
                            data.resize(file->_fsize(fp));
                            file->_fread(&data[0], data.size(), 1, fp);
                            file->_fclose(fp);

                            Variant var = Bson::load(data);
                            if(!var.isValid()) {
                                var = Json::load(string(data.begin(), data.end()));
                            }

                            VariantList objects = var.toList();
                            VariantList fields = objects.front().toList();
                            resource->loadUserData(fields.back().toMap());
                        }
                    }
                } break;
                case Resource::ToBeDeleted: {
                    p_ptr->m_DeleteList.push_back(resource);
                }
                case Resource::Suspend: {
                    resource->setState(Resource::ToBeDeleted);
                }
                default: break;
            }
        }
        ++it;
    }

    for(auto it : p_ptr->m_DeleteList) {
        deleteFromCahe(it);
        delete it;
    }
    p_ptr->m_DeleteList.clear();
}

bool ResourceSystem::isThreadSafe() const {
    return true;
}

void ResourceSystem::setResource(Object *object, const string &uuid) {
    PROFILE_FUNCTION();

    p_ptr->m_ResourceCache[uuid] = object;
    p_ptr->m_ReferenceCache[object] = uuid;
}

bool ResourceSystem::isResourceExist(const string &path) {
    PROFILE_FUNCTION();

    auto it = p_ptr->m_IndexMap.find(path);
    return (it != p_ptr->m_IndexMap.end());
}

Object *ResourceSystem::loadResource(const string &path) {
    PROFILE_FUNCTION();

    if(!path.empty()) {
        string uuid = path;
        {
            auto it = p_ptr->m_IndexMap.find(path);
            if(it != p_ptr->m_IndexMap.end()) {
                uuid = it->second.second;
            }
        }
        {
            auto it = p_ptr->m_ResourceCache.find(uuid);
            if(it != p_ptr->m_ResourceCache.end() && it->second) {
                return it->second;
            } else {
                File *file = Engine::file();
                _FILE *fp = file->_fopen(uuid.c_str(), "r");
                if(fp) {
                    ByteArray data;
                    data.resize(file->_fsize(fp));
                    file->_fread(&data[0], data.size(), 1, fp);
                    file->_fclose(fp);

                    Variant var = Bson::load(data);
                    if(!var.isValid()) {
                        var = Json::load(string(data.begin(), data.end()));
                    }
                    if(var.isValid()) {
                        Object *res = Engine::toObject(var);
                        if(res) {
                            Resource *resource = dynamic_cast<Resource *>(res);
                            if(resource) {
                                resource->setState(Resource::ToBeUpdated);
                            }
                            setResource(res, uuid);
                            return res;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

void ResourceSystem::unloadResource(const string &path) {
    PROFILE_FUNCTION();

    if(!path.empty()) {
        string uuid = path;
        {
            auto it = p_ptr->m_IndexMap.find(path);
            if(it != p_ptr->m_IndexMap.end()) {
                uuid = it->second.second;
            }
        }
        {
            auto it = p_ptr->m_ResourceCache.find(uuid);
            if(it != p_ptr->m_ResourceCache.end() && it->second) {
                unloadResource(it->second);
            }
        }
    }
}

void ResourceSystem::unloadResource(Object *object) {
    PROFILE_FUNCTION();
    Resource *resource = dynamic_cast<Resource *>(object);
    if(resource) {
        resource->setState(Resource::Suspend);
    }
}

void ResourceSystem::reloadResource(Object *object) {
    PROFILE_FUNCTION();
    Resource *resource = dynamic_cast<Resource *>(object);
    if(resource) {
        resource->setState(Resource::Loading);
    }
}

string ResourceSystem::reference(Object *object) {
    PROFILE_FUNCTION();
    auto it = p_ptr->m_ReferenceCache.find(object);
    if(it != p_ptr->m_ReferenceCache.end()) {
        return it->second;
    }
    return string();
}

ResourceSystem::DictionaryMap &ResourceSystem::indices() const {
    return p_ptr->m_IndexMap;
}

void ResourceSystem::deleteFromCahe(Object *object) {
    PROFILE_FUNCTION();
    auto ref = p_ptr->m_ReferenceCache.find(object);
    if(ref != p_ptr->m_ReferenceCache.end()) {
        auto res = p_ptr->m_ResourceCache.find(ref->second);
        if(res != p_ptr->m_ResourceCache.end()) {
            p_ptr->m_ResourceCache.erase(res);
        }
        p_ptr->m_ReferenceCache.erase(ref);
    }
}
