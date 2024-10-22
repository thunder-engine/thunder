#include "components/component.h"

#include "components/actor.h"
#include "components/scene.h"
#include "components/transform.h"

#include "system.h"

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
        m_enable(true),
        m_started(false) {

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
    return actor()->scene();
}
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
    Returns true if the component is flagged as started; otherwise returns false.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
bool Component::isStarted() const {
    return m_started;
}
/*!
    Marks component as \a started.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
void Component::setStarted(bool started) {
    m_started = started;
}
/*!
    Returns a transform attached to this Actor.
*/
Transform *Component::transform() const {
    Actor *act = actor();
    if(act) {
        return act->transform();
    }
    return nullptr;
}
/*!
    Returns a component with \a type attached to the same Actor.
    If no such component with this \a type returns nullptr.
*/
Component *Component::component(const std::string type) {
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
    Returns a translated version of \a source text; otherwise returns source text if no appropriate translated std::string is available.
*/
std::string Component::tr(const std::string source) {
    return Engine::translate(source);
}
/*!
    This method will be triggered in case of Actor will change its own parent.
    \internal
*/
void Component::actorParentChanged() {

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

inline void trimmType(std::string &type, bool &isArray) {
    if(type.back() == '*') {
        type.pop_back();
        while(type.back() == ' ') {
            type.pop_back();
        }
    } else if(type.back() == ']') {
        type.pop_back();
        while(type.back() == ' ') {
            type.pop_back();
        }
        if(type.back() == '[') {
            type.pop_back();
            isArray = true;
        }
    }
}

Object *loadObjectHelper(const Variant &value, const MetaObject *meta, Object *root) {
    Object *object = nullptr;
    if(meta->canCastTo(gResource)) {
        object = Engine::loadResource<Object>(value.toString());
    } else {
        uint32_t uuid = value.toInt();
        if(uuid) {
            object = Engine::findObject(uuid, root);
        }
    }

    return object;
}
/*!
    \internal
*/
void Component::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    const MetaObject *meta = metaObject();
    for(int index = 0; index < meta->propertyCount(); index++) {
        MetaProperty property = meta->property(index);
        auto field = data.find(property.name());
        if(field != data.end()) {
            bool isArray = false;
            std::string typeName = property.type().name();
            trimmType(typeName, isArray);
            auto factory = System::metaFactory(typeName);
            if(factory) {
                uint32_t type = MetaType::type(typeName.c_str()) + 1;
                Object *root = Engine::findRoot(this);
                if(isArray) {
                    VariantList list;
                    for(auto it : field->second.toList()) {
                        Object *object = loadObjectHelper(it, factory->first, root);
                        list.push_back(Variant(type, &object));
                    }
                    property.write(this, list);
                } else {
                    Object *object = loadObjectHelper(field->second, factory->first, root);
                    if(object) {
                        property.write(this, Variant(type, &object));
                    }
                }
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

    const MetaObject *meta = metaObject();
    for(int index = 0; index < meta->propertyCount(); index++) {
        MetaProperty property = meta->property(index);
        bool isArray = false;
        std::string typeName = property.type().name();
        trimmType(typeName, isArray);
        auto factory = System::metaFactory(typeName);
        if(factory) {
            Variant value = property.read(this);
            if(isArray) {
                VariantList list;
                for(auto it : value.toList()) {
                    Object *object = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(it.data()));
                    list.push_back(saveObjectHelper(object, factory->first));
                }
                result[property.name()] = list;
            } else {
                Object *object = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
                result[property.name()] = saveObjectHelper(object, factory->first);
            }
        }
    }
    return result;
}
/*!
    \internal
*/
bool Component::isSerializable() const {
    return (clonedFrom() == 0);
}
/*!
    \internal
*/
void Component::onReferenceDestroyed() {

}
