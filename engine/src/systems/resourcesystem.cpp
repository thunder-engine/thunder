#include "systems/resourcesystem.h"

#include <bson.h>
#include <json.h>

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
                        resource->setState(Resource::ToBeUpdated);
                        setResource(resource, uuid);
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
        resource->setState(Resource::Suspend);
        if(force) {
            processState(resource);
        }
    }
}

void ResourceSystem::reloadResource(Resource *resource, bool force) {
    PROFILE_FUNCTION();
    if(resource) {
        resource->setState(Resource::Loading);
        if(force) {
            processState(resource);
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

                        VariantList objects = var.toList();
                        VariantList fields = objects.front().toList();
                        auto it = std::next(fields.begin(), 4);
                        VariantMap &properties = *(reinterpret_cast<VariantMap *>((*it).data()));
                        for(const auto &prop : properties) {
                            Variant v  = prop.second;
                            if(v.type() < MetaType::USERTYPE) {
                                resource->setProperty(prop.first.c_str(), v);
                            }
                        }
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
