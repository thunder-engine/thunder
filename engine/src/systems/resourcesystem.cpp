#include "systems/resourcesystem.h"

#include <bson.h>
#include <json.h>

#include "engine.h"

#include "analytics/profiler.h"

#include "resources/resource.h"

static StringMap                        s_IndexMap;
static unordered_map<string, Object*>   s_ResourceCache;
static unordered_map<Object*, string>   s_ReferenceCache;

bool ResourceSystem::init() {
    return true;
}

const char *ResourceSystem::name() const {
    return "ResourceSystem";
}

void ResourceSystem::update(Scene *) {
    for(auto it : s_ResourceCache) {
        Resource *resource = dynamic_cast<Resource *>(it.second);
        if(resource && resource->state() == Resource::ToBeDeleted) {

        }
    }
}

bool ResourceSystem::isThreadSafe() const {
    return true;
}

void ResourceSystem::setResource(Object *object, const string &uuid) {
    PROFILER_MARKER;

    s_ResourceCache[uuid] = object;
    s_ReferenceCache[object] = uuid;
}

Object *ResourceSystem::loadResource(const string &path) {
    PROFILER_MARKER;

    if(!path.empty()) {
        string uuid = path;
        {
            auto it = s_IndexMap.find(path);
            if(it != s_IndexMap.end()) {
                uuid    = it->second;
            }
        }
        {
            auto it = s_ResourceCache.find(uuid);
            if(it != s_ResourceCache.end() && it->second) {
                return it->second;
            } else {
                IFile *file = Engine::file();
                _FILE *fp   = file->_fopen(uuid.c_str(), "r");
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
    PROFILER_MARKER;

    if(!path.empty()) {
        string uuid = path;
        {
            auto it = s_IndexMap.find(path);
            if(it != s_IndexMap.end()) {
                uuid = it->second;
            }
        }
        {
            auto it = s_ResourceCache.find(uuid);
            if(it != s_ResourceCache.end() && it->second) {
                Resource *resource = dynamic_cast<Resource *>(it->second);
                if(resource) {
                    resource->setState(Resource::Suspend);
                } else {
                    delete it->second;
                    s_ResourceCache.erase(it);
                }
            }
        }
    }
}

string ResourceSystem::reference(Object *object) {
    PROFILER_MARKER;

    auto it = s_ReferenceCache.find(object);
    if(it != s_ReferenceCache.end()) {
        return it->second;
    }
    return string();
}

StringMap &ResourceSystem::indices() const {
    return s_IndexMap;
}
