#include "systems/resourcesystem.h"

#include <bson.h>
#include <json.h>
#include <log.h>

#include <algorithm>

#include "file.h"

#include "resources/resource.h"

#include "resources/text.h"
#include "resources/texture.h"
#include "resources/rendertarget.h"
#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/font.h"
#include "resources/animationclip.h"
#include "resources/animationstatemachine.h"
#include "resources/translator.h"
#include "resources/visualeffect.h"
#include "resources/pipeline.h"
#include "resources/pose.h"
#include "resources/prefab.h"
#include "resources/map.h"
#include "resources/computebuffer.h"
#include "resources/computeshader.h"

#include "resources/meshgroup.h"

#include "resources/controlscheme.h"

#include "resources/tileset.h"
#include "resources/tilemap.h"

ResourceSystem::ResourceSystem() :
        m_clean(false) {
    setName("ResourceSystem");

    // The order is critical for the import
    Resource::registerClassFactory(this);

    Text::registerClassFactory(this);
    Texture::registerClassFactory(this);
    Material::registerClassFactory(this);
    Mesh::registerClassFactory(this);
    MeshGroup::registerClassFactory(this);
    Sprite::registerClassFactory(this);
    Font::registerClassFactory(this);
    AnimationClip::registerClassFactory(this);
    RenderTarget::registerClassFactory(this);

    Translator::registerClassFactory(this);
    Pose::registerSuper(this);

    Prefab::registerClassFactory(this);
    Map::registerClassFactory(this);

    TileSet::registerClassFactory(this);
    TileMap::registerClassFactory(this);

    VisualEffect::registerClassFactory(this);

    AnimationStateMachine::registerSuper(this);

    Pipeline::registerClassFactory(this);

    ComputeBuffer::registerClassFactory(this);
    ComputeShader::registerClassFactory(this);

    ControlScheme::registerClassFactory(this);
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
    return Pool;
}

void ResourceSystem::setResource(Resource *object, const TString &uuid) {
    PROFILE_FUNCTION();

    m_resourceCache[uuid] = object;
    m_referenceCache[object] = uuid;
}

Resource *ResourceSystem::loadResource(const TString &path) {
    PROFILE_FUNCTION();

    if(!path.isEmpty() && !m_clean) {
        TString uuid = path;

        Resource *object = resource(uuid);
        if(object) {
            return object;
        }

        File fp(uuid);
        if(fp.open(File::ReadOnly)) {
            ByteArray data(fp.readAll());

            Variant var = Bson::load(data);
            if(!var.isValid()) {
                var = Json::load(TString(data));
            }
            if(var.isValid()) {
                Resource *resource = static_cast<Resource *>(Engine::toObject(var, nullptr, uuid));

                resource->switchState(Resource::ToBeUpdated);

                return resource;
            }
        }
    }

    return nullptr;
}

Resource *ResourceSystem::loadResourceAsync(const TString &path) {
    if(!path.isEmpty() && !m_clean) {
        ResourceInfo info;

        auto indexIt = m_indexMap.find(path);
        if(indexIt != m_indexMap.end()) {
            info = indexIt->second;
        } else {
            for(auto &it : m_indexMap) {
                if(it.second.uuid == path) {
                    info = it.second;
                    break;
                }
            }
        }

        auto resourceIt = m_resourceCache.find(info.uuid);
        if(resourceIt != m_resourceCache.end() && resourceIt->second) {
            return resourceIt->second;
        }

        Resource *resource = nullptr;
        if(!info.type.isEmpty()) {
            resource = static_cast<Resource *>(Engine::objectCreate(info.type, info.uuid, nullptr, info.id));
            resource->setState(Resource::Loading);
        } else {

        }

        return resource;
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

bool ResourceSystem::isResourceExist(const TString &path) const {
    PROFILE_FUNCTION();

    auto it = m_indexMap.find(path);
    return (it != m_indexMap.end());
}

TString ResourceSystem::reference(Resource *resource) const {
    PROFILE_FUNCTION();
    auto it = m_referenceCache.find(resource);
    if(it != m_referenceCache.end()) {
        return it->second;
    }
    return TString();
}

ResourceSystem::Dictionary &ResourceSystem::indices() {
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

void ResourceSystem::makeClean() {
    m_clean = true;
}

void ResourceSystem::processState(Resource *resource) {
    if(resource) {
        switch(resource->state()) {
            case Resource::Loading: {
                TString uuid = reference(resource);
                if(!uuid.isEmpty()) {
                    File fp(uuid);
                    if(fp.open(File::ReadOnly)) {
                        ByteArray data(fp.readAll());
                        fp.close();

                        Variant var = Bson::load(data);
                        if(!var.isValid()) {
                            var = Json::load(TString(data));
                        }

                        if(var.isValid()) {
                            Engine::toObject(var, nullptr, uuid);
                        }

                        resource->switchState(Resource::ToBeUpdated);
                    } else {
                        aError() << "Unable to load resource:" << uuid;
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

Resource *ResourceSystem::resource(TString &path) const {
    {
        auto it = m_indexMap.find(path);
        if(it != m_indexMap.end()) {
            path = it->second.uuid;
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

Object *ResourceSystem::instantiateObject(const MetaObject *meta, const TString &name, Object *parent) {
    Object *result = System::instantiateObject(meta, name, parent);

    Resource *resource = dynamic_cast<Resource *>(result);
    if(resource) {
        if(!name.isEmpty()) {
            setResource(resource, name);
        }

        resource->switchState(Resource::ToBeUpdated);
    }

    return result;
}
