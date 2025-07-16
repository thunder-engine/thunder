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

ResourceSystem::ResourceSystem() {
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

void ResourceSystem::setResource(Resource *object, const String &uuid) {
    PROFILE_FUNCTION();

    m_resourceCache[uuid] = object;
    m_referenceCache[object] = uuid;
}

bool ResourceSystem::isResourceExist(const String &path) {
    PROFILE_FUNCTION();

    auto it = m_indexMap.find(path);
    return (it != m_indexMap.end());
}

Resource *ResourceSystem::loadResource(const String &path) {
    PROFILE_FUNCTION();

    if(!path.isEmpty()) {
        String uuid = path;
        Resource *object = resource(uuid);
        if(object) {
            return object;
        }

        File *file = Engine::file();
        _FILE *fp = file->fopen(uuid.data(), "r");
        if(fp) {
            ByteArray data;
            size_t size = file->fsize(fp);
            data.resize(size);
            file->fread(&data[0], data.size(), 1, fp);
            file->fclose(fp);

            Variant var = Bson::load(data);
            if(!var.isValid()) {
                var = Json::load(String(data));
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

String ResourceSystem::reference(Resource *resource) {
    PROFILE_FUNCTION();
    auto it = m_referenceCache.find(resource);
    if(it != m_referenceCache.end()) {
        return it->second;
    }
    return String();
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
                String uuid = reference(resource);
                if(!uuid.isEmpty()) {
                    File *file = Engine::file();
                    _FILE *fp = file->fopen(uuid.data(), "r");
                    if(fp) {
                        ByteArray data;
                        data.resize(file->fsize(fp));
                        file->fread(&data[0], data.size(), 1, fp);
                        file->fclose(fp);

                        Variant var = Bson::load(data);
                        if(!var.isValid()) {
                            var = Json::load(String(data));
                        }

                        ObjectList deleteObjects;
                        enumObjects(resource, deleteObjects);

                        const VariantList &objects = *(reinterpret_cast<VariantList *>(var.data()));
                        auto delIt = deleteObjects.begin();
                        bool first = true;
                        for(auto &obj : objects) {
                            const VariantList &fields = *(reinterpret_cast<VariantList *>(obj.data()));
                            auto it = std::next(fields.begin(), 1);
                            Object *object = resource;
                            if(!first) {
                                object = Engine::findObject(it->toInt());
                            } else {
                                first = false;
                            }

                            if(object) {
                                it = std::next(fields.begin(), 4);
                                const VariantMap &properties = *(reinterpret_cast<VariantMap *>((*it).data()));
                                for(const auto &prop : properties) {
                                    Variant v = prop.second;
                                    if(v.type() < MetaType::USERTYPE) {
                                        object->setProperty(prop.first.data(), v);
                                    }
                                }

                                object->loadUserData(fields.back().toMap());

                                if(delIt != deleteObjects.end()) {
                                    delIt = deleteObjects.erase(delIt);
                                }
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

Resource *ResourceSystem::resource(String &path) const {
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

Object *ResourceSystem::instantiateObject(const MetaObject *meta, const String &name, Object *parent) {
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
