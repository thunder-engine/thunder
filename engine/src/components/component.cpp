#include "components/component.h"

#include "components/actor.h"
#include "components/scene.h"
#include "components/transform.h"

#include "system.h"

#include <algorithm>

namespace {
    const char *gResource("Resource");
};

/*!
    \module Components

    \title Thunder Engine Software Developer Kit

    \brief Contains base component classes.
*/

/*!
    \class Component
    \brief Base class for everything attached to Actor.
    \inmodule Components

    The Component class is a base class for each aspect of the actor, and how it interacts with the world.
    \note This class must be a superclass only and shouldn't be created manually.
*/

Component::Component() :
        m_enable(true) {

}

Component::~Component() {
    Scene *scene = Component::scene();
    if(scene) {
        for(auto it : m_tags) {
            scene->removeFromGroupByHash(this, it);
        }
    }
}

/*!
    Returns an Actor which the Component is attached to.
*/
Actor *Component::actor() const {
    return (static_cast<Actor *>(parent()));
}
/*!
    Returns a Scene which the Component is attached to.
*/
Scene *Component::scene() const {
    Actor *actor = Component::actor();
    if(actor) {
        return actor->scene();
    }
    return nullptr;
}
/*!
    Returns a World which the Component is attached to.
*/
World *Component::world() const {
    Scene *scene = Component::scene();
    if(scene) {
        return scene->world();
    }
    return nullptr;
}
/*!
    Returns true if the component is enabled; otherwise returns false.
*/
bool Component::isEnabled() const {
    return m_enable;
}
/*!
    Sets current state of component to \a enabled or disabled.
    \note The disabled component will be created but not affect the Actor. For example, MeshRender component will not draw a mesh.
*/
void Component::setEnabled(bool enabled) {
    m_enable = enabled;
}
/*!
    Returns false in case of one of Actors in top hierarchy or this component was disabled; otherwise returns true.
*/
bool Component::isEnabledInHierarchy() const {
    Actor *actor = Component::actor();
    if(actor) {
        return actor->isEnabledInHierarchy() && m_enable;
    }
    return m_enable;
}
/*!
    Returns a transform attached to this Actor.
*/
Transform *Component::transform() const {
    Actor *actor = Component::actor();
    if(actor) {
        return actor->transform();
    }
    return nullptr;
}
/*!
    Returns a component with \a type attached to the same Actor.
    If no such component with this \a type returns nullptr.
*/
Component *Component::component(const TString &type) {
    return actor()->component(type);
}
/*!
    Clones the actor represented by \a prefab asset.
    This Actor will be a sibling of caller Actor and has local \a position and \a rotation.
*/
Actor *Component::instantiate(Prefab *prefab, Vector3 position, Quaternion rotation) {
    Actor *result = nullptr;
    if(prefab) {
        result = static_cast<Actor *>(prefab->actor()->clone(actor()));

        Transform *t = result->transform();
        if(t) {
            t->setPosition(position);
            t->setQuaternion(rotation);
        }
    }

    return result;
}
/*!
    Returns a translated version of \a source text; otherwise returns source text if no appropriate translated string is available.
*/
TString Component::tr(const TString &source) {
    return Engine::translate(source);
}
/*!
    Adds \a tag for current component.
    Automatically adds component to specific Scene group.
*/
void Component::addTag(const TString &tag) {
    addTagByHash(Mathf::hashString(tag));
}
/*!
    Adds a tag for current component by tag \a hash.
    Automatically adds component to specific Scene group.
*/
void Component::addTagByHash(uint32_t hash) {
    if(!hasTagByHash(hash)) {
        m_tags.push_back(hash);

        Scene *scene = Component::scene();
        if(scene) {
            scene->addToGroupByHash(this, hash);
        }
    }
}
/*!
    Removes \a tag for current component.
    Automatically removes component from specific Scene group.
*/
void Component::removeTag(const TString &tag) {
    removeTagByHash(Mathf::hashString(tag));
}
/*!
    Removes a tag for current component by tag \a hash.
    Automatically removes component from specific Scene group.
*/
void Component::removeTagByHash(uint32_t hash) {
    auto it = std::find(m_tags.begin(), m_tags.end(), hash);
    if(it != m_tags.end()) {
        m_tags.erase(it);

        Scene *scene = Component::scene();
        if(scene) {
            scene->removeFromGroupByHash(this, hash);
        }
    }
}
/*!
    Returns true if component has a \a tag; otherwise returns false.
*/
bool Component::hasTag(const TString &tag) {
    return hasTagByHash(Mathf::hashString(tag));
}
/*!
    Returns true if component has a tag provided by its \a hash; otherwise returns false.
*/
bool Component::hasTagByHash(uint32_t hash) {
    return std::find(m_tags.begin(), m_tags.end(), hash) != m_tags.end();
}
/*!
    \internal
*/
void Component::composeComponent() {

}
/*!
    Implement drawGizmos if you want to draw gizmos that are always drawn.
*/
void Component::drawGizmos() {

}
/*!
    Implement drawGizmosSelected to draw a gizmo if the object is selected.
*/
void Component::drawGizmosSelected() {

}

inline void trimmType(TString &type, bool &isArray) {
    if(type.back() == '*') {
        type.removeLast();
        while(type.back() == ' ') {
            type.removeLast();
        }
    } else if(type.back() == ']') {
        type.removeLast();
        while(type.back() == ' ') {
            type.removeLast();
        }
        if(type.back() == '[') {
            type.removeLast();
            isArray = true;
        }
    }
}

Object *loadObjectHelper(const Variant &value) {
    Object *object = nullptr;
    switch(value.type()) {
        case MetaType::STRING: {
            object = Engine::loadResource(value.toString());
        } break;
        case MetaType::INTEGER: {
            uint32_t uuid = value.toInt();
            if(uuid != 0) {
                object = Engine::findObject(uuid);
            }
        } break;
        default: break;
    }

    return object;
}
/*!
    \internal
*/
void Component::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    std::list<std::pair<TString, TString>> properties;
    const MetaObject *meta = metaObject();
    for(int index = 0; index < meta->propertyCount(); index++) {
        MetaProperty property = meta->property(index);
        properties.push_back(std::make_pair(property.name(), property.type().name()));
    }
    for(auto it : dynamicPropertyNames()) {
        properties.push_back(std::make_pair(it, ""));
    }

    for(auto &it : properties) {
        auto field = data.find(it.first);
        if(field != data.end()) {
            bool isArray = false;
            TString typeName = it.second;
            if(typeName.isEmpty()) {
                Variant value = property(it.first.data());
                const char *name = MetaType::name(value.userType());
                if(name) {
                    typeName = name;
                }
            }

            trimmType(typeName, isArray);
            auto factory = System::metaFactory(typeName);
            if(factory) {
                uint32_t type = MetaType::type(typeName.data()) + 1;
                if(isArray) {
                    VariantList list;
                    for(auto &it : field->second.toList()) {
                        Object *object = loadObjectHelper(it);
                        if(object) {
                            list.push_back(Variant(type, &object));
                        }
                    }
                    setProperty(it.first.data(), list);
                } else {
                    Object *object = loadObjectHelper(field->second);
                    if(object) {
                        setProperty(it.first.data(), Variant(type, &object));
                    }
                }
            } else {
                setProperty(it.first.data(), field->second);
            }
        }
    }
}

Variant saveObjectHelper(Object *object, const MetaObject *meta) {
    if(meta->canCastTo(gResource)) {
        return Engine::reference(object);
    }
    uint32_t uuid = 0;
    if(object) {
        uuid = object->uuid();
    }
    return uuid;
}
/*!
    \internal
*/
VariantMap Component::saveUserData() const {
    PROFILE_FUNCTION();
    VariantMap result;

    std::list<std::pair<TString, TString>> properties;

    const MetaObject *meta = metaObject();
    for(int index = 0; index < meta->propertyCount(); index++) {
        MetaProperty property = meta->property(index);
        properties.push_back(std::make_pair(property.name(), property.type().name()));
    }
    for(auto &it : dynamicPropertyNames()) {
        properties.push_back(std::make_pair(it, ""));
    }

    for(auto &it : properties) {
        Variant value = property(it.first.data());

        bool isArray = false;
        TString typeName = it.second;
        if(typeName.isEmpty()) {
            const char *name = MetaType::name(value.userType());
            if(name) {
                typeName = name;
            }
        }
        trimmType(typeName, isArray);
        auto factory = System::metaFactory(typeName);
        if(factory) {
            if(isArray) {
                VariantList list;
                for(auto &it : value.toList()) {
                    Object *object = (it.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(it.data()));
                    list.push_back(saveObjectHelper(object, factory->first));
                }
                result[it.first] = list;
            } else {
                Object *object = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
                result[it.first] = saveObjectHelper(object, factory->first);
            }
        } else if(isArray) {
            result[it.first] = value;
        }
    }
    return result;
}
/*!
    \internal
*/
void Component::onReferenceDestroyed() {

}
/*!
    This method will be called automatically when \a current scene will be \a changed to a new one.
*/
void Component::changeScene(Scene *current, Scene *changed) {
    for(auto it : m_tags) {
        if(current) {
            current->removeFromGroupByHash(this, it);
        }
        if(changed) {
            changed->addToGroupByHash(this, it);
        }
    }
}

void Component::setParent(Object *parent, int32_t position, bool force) {
    Scene *currentScene = Component::scene();

    Object::setParent(parent, position, force);

    Scene *changedScene = Component::scene();

    if(currentScene != changedScene) {
        changeScene(currentScene, changedScene);
    }
}
